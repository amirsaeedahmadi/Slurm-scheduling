/*****************************************************************************\
 *  job_submit_partition.c - Set default partition in job submit request
 *  specifications.
 *****************************************************************************
 *  Copyright (C) 2010 Lawrence Livermore National Security.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Morris Jette <jette1@llnl.gov>
 *  CODE-OCEC-09-009. All rights reserved.
 *
 *  This file is part of Slurm, a resource management program.
 *  For details, see <https://slurm.schedmd.com/>.
 *  Please also read the included file: DISCLAIMER.
 *
 *  Slurm is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  In addition, as a special exception, the copyright holders give permission
 *  to link the code of portions of this program with the OpenSSL library under
 *  certain conditions as described in each individual source file, and
 *  distribute linked combinations including the two. You must obey the GNU
 *  General Public License in all respects for all of the code used other than
 *  OpenSSL. If you modify file(s) with this exception, you may extend this
 *  exception to your version of the file(s), but you are not obligated to do
 *  so. If you do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source files in
 *  the program, then also delete it here.
 *
 *  Slurm is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Slurm; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
\*****************************************************************************/

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "slurm/slurm_errno.h"
#include "src/common/slurm_xlator.h"

#include "src/common/xstring.h"
#include "src/slurmctld/locks.h"
#include "src/slurmctld/slurmctld.h"

/*
 * These variables are required by the generic plugin interface.  If they
 * are not found in the plugin, the plugin loader will ignore it.
 *
 * plugin_name - a string giving a human-readable description of the
 * plugin.  There is no maximum length, but the symbol must refer to
 * a valid string.
 *
 * plugin_type - a string suggesting the type of the plugin or its
 * applicability to a particular form of data or method of data handling.
 * If the low-level plugin API is used, the contents of this string are
 * unimportant and may be anything.  Slurm uses the higher-level plugin
 * interface which requires this string to be of the form
 *
 *	<application>/<method>
 *
 * where <application> is a description of the intended application of
 * the plugin (e.g., "auth" for Slurm authentication) and <method> is a
 * description of how this plugin satisfies that application.  Slurm will
 * only load authentication plugins if the plugin_type string has a prefix
 * of "auth/".
 *
 * plugin_version - an unsigned 32-bit integer containing the Slurm version
 * (major.minor.micro combined into a single number).
 */
const char plugin_name[]       	= "Job submit partition plugin";
const char plugin_type[]       	= "job_submit/partition";
const uint32_t plugin_version   = SLURM_VERSION_NUMBER;
static int last_io = 0;

/*****************************************************************************\
 * We've provided a simple example of the type of things you can do with this
 * plugin. If you develop another plugin that may be of interest to others
 * please post it to slurm-dev@schedmd.com  Thanks!
\*****************************************************************************/

/* Test if this user can run jobs in the selected partition based upon
 * the partition's AllowGroups parameter. */
static bool _user_access(uid_t run_uid, uint32_t submit_uid,
			 part_record_t *part_ptr)
{
	int i;

	if (run_uid == 0) {
		if (part_ptr->flags & PART_FLAG_NO_ROOT)
			return false;
		return true;
	}

	if ((part_ptr->flags & PART_FLAG_ROOT_ONLY) && (submit_uid != 0))
		return false;

	if (part_ptr->allow_uids == NULL)
		return true;	/* AllowGroups=ALL */

	for (i=0; part_ptr->allow_uids[i]; i++) {
		if (part_ptr->allow_uids[i] == run_uid)
			return true;	/* User in AllowGroups */
	}
	return false;		/* User not in AllowGroups */
}

static bool _valid_memory(part_record_t *part_ptr, job_desc_msg_t *job_desc)
{
	uint64_t job_limit, part_limit;

	if (!part_ptr->max_mem_per_cpu)
		return true;

	if (job_desc->pn_min_memory == NO_VAL64)
		return true;

	if ((job_desc->pn_min_memory   & MEM_PER_CPU) &&
	    (part_ptr->max_mem_per_cpu & MEM_PER_CPU)) {
		/* Perform per CPU memory limit test */
		job_limit  = job_desc->pn_min_memory   & (~MEM_PER_CPU);
		part_limit = part_ptr->max_mem_per_cpu & (~MEM_PER_CPU);
		if (job_desc->pn_min_cpus != NO_VAL16) {
			job_limit  *= job_desc->pn_min_cpus;
			part_limit *= job_desc->pn_min_cpus;
		}
	} else if (((job_desc->pn_min_memory   & MEM_PER_CPU) == 0) &&
		   ((part_ptr->max_mem_per_cpu & MEM_PER_CPU) == 0)) {
		/* Perform per node memory limit test */
		job_limit  = job_desc->pn_min_memory;
		part_limit = part_ptr->max_mem_per_cpu;
	} else {
		/* Can not compare per node to per CPU memory limits */
		return true;
	}

	if (job_limit > part_limit) {
		debug("job_submit/partition: skipping partition %s due to "
		      "memory limit (%"PRIu64" > %"PRIu64")",
		      part_ptr->name, job_limit, part_limit);
		return false;
	}

	return true;
}

extern char * my_name_adaptor(char* current_job_name, char* current_partition_name) 
{

	char * result = current_job_name;
    if (xstrstr(current_job_name, "CPU") && xstrstr(current_partition_name, "IO")) {
        // If job's name contains "CPU" and it's assigned to "IO" partition,
        // assign it to "CPU" partition
		info("AMIRSAEED: in first if");
        // strncpy(job_desc->partition, "CPU", SLURM_MAX_NAME_LEN);
		result = "CPU";
		info("AMIRSAEED: after that ");
    } else if (xstrstr(current_job_name, "IO") && xstrstr(current_partition_name, "CPU")) {
        // If job's name contains "IO" and it's assigned to "CPU" partition,
        // assign it to "IO" partition
        // strncpy(job_desc->partition, "IO1", SLURM_MAX_NAME_LEN);
		result = "IO";
    } else if (current_partition_name == NULL) {
        // If job_desc->partition is NULL, assign partition based on job's name
        if (xstrstr(current_job_name, "CPU")) {
            // strncpy(job_desc->partition, "CPU", SLURM_MAX_NAME_LEN);
			result = "CPU";
        } else if (xstrstr(current_job_name, "IO")) {
            // strncpy(job_desc->partition, "IO1", SLURM_MAX_NAME_LEN);
			result = "IO";
        }
    }

	return result;
}

/* This example code will set a job's default partition to the partition with
 * highest priority_tier is available to this user. This is only an example
 * and tremendous flexibility is available. */
extern int job_submit(job_desc_msg_t *job_desc, uint32_t submit_uid,
		      char **err_msg)
{
	info("AMIRSAEED IN JOB SUBMIT");
	info("AMIRSAEED: Before Job id = %d, Job name = %s and Partition name = %s",job_desc->job_id, job_desc->name, job_desc->partition);

	ListIterator part_iterator;
	part_record_t *part_ptr;
	part_record_t *io_one_part;
	part_record_t *io_two_part;
	part_record_t *cpu_part;
	part_record_t *top_prio_part = NULL;

	// if (job_desc->partition)	/* job already specified partition */
	// 	return SLURM_SUCCESS;

	part_iterator = list_iterator_create(part_list);
	part_ptr = list_next(part_iterator);
	cpu_part = list_next(part_iterator);
	io_one_part = list_next(part_iterator);
	io_two_part = list_next(part_iterator);
	list_iterator_destroy(part_iterator);

	part_iterator = list_iterator_create(part_list);
	while ((part_ptr = list_next(part_iterator))) {
		info("AMIRSAEED: part_ptr.name is %s", part_ptr->name);
		if (!(part_ptr->state_up & PARTITION_SUBMIT))
			continue;	/* nobody can submit jobs here */
		info("up bood");
		if (!_user_access(job_desc->user_id, submit_uid, part_ptr))
			continue;	/* AllowGroups prevents use */
		info("user access ok bood.");
		if (!top_prio_part ||
		    (top_prio_part->priority_tier < part_ptr->priority_tier)) {
			info("tooye if ham oomad");
			/* Test job specification elements here */
			if (!_valid_memory(part_ptr, job_desc))
				continue;
			info ("valid memory ham ok bood");

			/* Found higher priority partition */
			// top_prio_part = part_ptr;
			info("partition name is %s", part_ptr->name);

			top_prio_part = my_name_adaptor(job_desc->name, job_desc->partition);
			info("New partition name is: %s", top_prio_part);
			// if (xstrcmp(job_desc->partition, io_one_part->name) == 0) {
			// 	info("esme partititon ham io1 bood");
			// 	top_prio_part = part_ptr;
			// }
		}
	}
	list_iterator_destroy(part_iterator);

	// if (top_prio_part) {
	// 	info("AMIRSAEED: Setting partition of submitted job to %s",
	// 	     top_prio_part->name);
	// 	job_desc->partition = xstrdup(top_prio_part->name);
	// }
    char *strings[] = {
        io_one_part->name,
        io_two_part->name
    };

    // assign io partitions in a round robbin manner
    int index = last_io % 2;
	last_io++;

	if (xstrcmp(top_prio_part, "CPU") == 0) {
		job_desc->partition = cpu_part->name;
	} else {
		job_desc->partition = strings[index];
	}
	

	// job_desc->partition = cpu_part->name;

	// job_desc->partition = "CPU";

	info("AMIRSAEED: After  Job id = %d, Job name = %s and Partition name = %s\n",job_desc->job_id, job_desc->name, job_desc->partition);

	return SLURM_SUCCESS;
}

extern int job_modify(job_desc_msg_t *job_desc, job_record_t *job_ptr,
		      uint32_t submit_uid)
{
	return SLURM_SUCCESS;
}
