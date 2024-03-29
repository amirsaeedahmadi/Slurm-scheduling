/*****************************************************************************\
 *  groups.h - T=Header to gather group membership information
 *             These functions utilize a cache for performance reasons
 *****************************************************************************
 *  Copyright (C) 2010 Lawrence Livermore National Security.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Morris Jette <jette1@llnl.gov> et. al.
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

#ifndef _HAVE_GROUPS_H
#define _HAVE_GROUPS_H

#include <unistd.h>
#include <sys/types.h>

/* Delete our group/uid cache */
extern void clear_group_cache(void);

/*
 * get_groups_members - identify the users in a given comma separated group list
 * IN group_names - comma separated group list
 * OUT user_cnt - pointer to an integer holding returned array size
 * NOTE: The caller must xfree non-NULL return values
 * NOTE: Call clear_group_cache() to flush cache
 */
extern uid_t *get_groups_members(char *group_names, int *user_cnt);

/* get_group_tlm - return the time of last modification for the GROUP_FILE */
extern time_t get_group_tlm(void);

#endif /* !_HAVE_GROUPS_H */
