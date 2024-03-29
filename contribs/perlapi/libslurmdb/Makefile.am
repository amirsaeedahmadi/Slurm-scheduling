AUTOMAKE_OPTIONS = foreign
# copied from pidgin
#
perl_dir = perl
perlpath = /usr/bin/perl
_perlsitedir=$(exec_prefix)`perl -e 'use Config; $$T=$$Config{installsitearch}; $$P=$$Config{installprefix}; $$P1="$$P/local"; $$T =~ s/$$P1//; $$T =~ s/$$P//; print $$T;'`
perl_sources = \
	       $(perl_dir)/Makefile.PL.in \
	       $(perl_dir)/ppport.h \
	       $(perl_dir)/Slurmdb.pm \
	       $(perl_dir)/Slurmdb.xs \
	       $(perl_dir)/slurmdb-perl.h \
	       $(perl_dir)/cluster.c

test_sources = \
	       $(perl_dir)/t/00-use.t \
	       $(perl_dir)/t/01-clusters_get.t \
	       $(perl_dir)/t/02-report_cluster_account_by_user.t \
	       $(perl_dir)/t/03-report_cluster_user_by_account.t \
	       $(perl_dir)/t/04-report_job_sizes_grouped_by_account.t \
	       $(perl_dir)/t/05-report_user_top_usage.t \
	       $(perl_dir)/t/06-jobs_get.t \
	       $(perl_dir)/t/07-qos_get.t

$(perl_dir)/Makefile:	$(perl_dir)/Makefile.PL
	@if test "x${top_srcdir}" != "x${top_builddir}"; then \
		for f in ${perl_sources}; do \
			${LN_S} -f ${abs_srcdir}/$$f $$f; \
		done; \
		for f in ${test_sources}; do \
			$(mkdir_p) `dirname $$f`; \
			${LN_S} -f ${abs_srcdir}/$$f $$f; \
		done; \
	fi
	@cd $(perl_dir) && $(perlpath) Makefile.PL $(PERL_MM_PARAMS) \
		prefix=${prefix} INSTALL_BASE= PERL_MM_OPT= \
		INSTALLDIRS=site INSTALLSITELIB=${_perlsitedir} \
		INSTALLSITEARCH=${_perlsitedir} \
		INSTALLSITEMAN1DIR=$(mandir)/man1 \
		INSTALLSITEMAN3DIR=$(mandir)/man3

#
# Note on linking logic below
#
# Suse Linux compiles with gcc, but picks some other compiler to use for linking.
#   Since some CFLAGS may be incompatible with this other compiler, the build
#   may fail, as seen on BlueGene platforms.
# Other Linux implementations sems to work fine with the LD specified as below
#
all-local: $(perl_dir)/Makefile #libslurm
	@cd $(perl_dir) && \
	if [ ! -f Makefile ]; then \
		$(perlpath) Makefile.PL $(PERL_MM_PARAMS) prefix=${prefix} INSTALL_BASE= PERL_MM_OPT= INSTALLSITEMAN1DIR=$(mandir)/man1 INSTALLSITEMAN3DIR=$(mandir)/man3; \
	fi && \
	($(MAKE) CC="$(CC)" LD="$(CC) $(CFLAGS) $(LDFLAGS)" CCFLAGS="$(PERL_CFLAGS) -g -static $(CFLAGS) $(CPPFLAGS)" exec_prefix="${exec_prefix}" $(PERL_EXTRA_OPTS) || \
	 $(MAKE) CC="$(CC)" LD="$(CC) $(CFLAGS) $(LDFLAGS)" CCFLAGS="$(PERL_CFLAGS) -g -static $(CFLAGS) $(CPPFLAGS)" exec_prefix="${exec_prefix}" $(PERL_EXTRA_OPTS)) && \
	cd ..;

install-exec-local:
	@cd $(perl_dir) && \
	$(MAKE) DESTDIR=$(DESTDIR) install && \
	cd ..;

# Evil Hack (TM)
# ... which doesn't work with DESTDIR installs. FIXME?
uninstall-local:
	@cd $(perl_dir) && \
	`$(MAKE) uninstall | grep unlink | sed -e 's#/usr#${prefix}#' -e 's#unlink#rm -f#'` && \
	cd ..;

clean-generic:
	@cd $(perl_dir); \
	$(MAKE) clean; \
	if test "x${top_srcdir}" != "x${top_builddir}"; then \
		rm -fr lib t *c *h *xs typemap classmap; \
	fi; \
	cd ..;

	@if test "x${top_srcdir}" != "x${top_builddir}"; then \
		for f in ${perl_sources}; do \
			$(mkdir_p) `dirname $$f`; \
			${LN_S} -f ${abs_srcdir}/$$f $$f; \
		done; \
		for f in ${test_sources}; do \
			$(mkdir_p) `dirname $$f`; \
			${LN_S} -f ${abs_srcdir}/$$f $$f; \
		done; \
	fi

distclean-generic:
	@cd $(perl_dir); \
	$(MAKE) realclean; \
	rm -f Makefile.PL; \
	rm -f Makefile.old; \
	rm -f Makefile; \
	cd ..;

	@rm -f Makefile

	@if test "x${top_srcdir}" != "x${top_builddir}"; then \
		for f in ${perl_sources}; do \
			${LN_S} -f ${abs_srcdir}/$$f $$f; \
		done; \
		for f in ${test_sources}; do \
			$(mkdir_p) `dirname $$f`; \
			${LN_S} -f ${abs_srcdir}/$$f $$f; \
		done; \
	fi

AM_CPPFLAGS = \
	-DVERSION=\"$(VERSION)\" \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(DEBUG_CFLAGS) \
	$(PERL_CFLAGS)
