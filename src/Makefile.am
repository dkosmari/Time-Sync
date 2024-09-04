# src/Makefile.am

SUBDIRS = net


AM_CPPFLAGS = \
	-I$(top_srcdir)/external/libwupsxx/include \
	$(DEVKITPRO_CPPFLAGS)

AM_CXXFLAGS = \
	-Wall -Wextra \
	$(DEVKITPRO_CFLAGS)

AM_LDFLAGS = $(DEVKITPRO_LDFLAGS)

LIBS = \
	$(top_builddir)/external/libwupsxx/src/libwupsxx.a \
	$(DEVKITPRO_LIBS)


WPS_FILE = time-sync.wps
noinst_PROGRAMS = time-sync.elf


time_sync_elf_SOURCES = \
	async_queue.hpp						\
	cfg.cpp cfg.hpp						\
	clock_item.cpp clock_item.hpp				\
	core.cpp core.hpp					\
	curl.cpp curl.hpp					\
	http_client.cpp http_client.hpp				\
	main.cpp						\
	notify.cpp notify.hpp					\
	ntp.cpp ntp.hpp						\
	preview_screen.cpp preview_screen.hpp			\
	synchronize_item.cpp synchronize_item.hpp		\
	thread_pool.cpp thread_pool.hpp				\
	time_utils.cpp time_utils.hpp				\
	timezone_offset_item.cpp timezone_offset_item.hpp	\
	timezone_query_item.cpp timezone_query_item.hpp		\
	utc.cpp utc.hpp						\
	utils.cpp utils.hpp					\
	verbosity_item.cpp verbosity_item.hpp


time_sync_elf_LDADD = net/libnet.a


all-local: $(WPS_FILE)


install-exec-local: $(WPS_FILE)
	curl "ftp://wiiu:/fs/vol/external01/wiiu/environments/aroma/plugins/" --upload-file $<

uninstall-local:
	curl "ftp://wiiu" --quote "DELE /fs/vol/external01/wiiu/environments/aroma/plugins/$(WPS_FILE)"


run-local: all
	WIILOAD=tcp:wiiu wiiload $(WPS_FILE)


.PHONY: company
company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> $(srcdir)/compile_flags.txt


@INC_AMINCLUDE@

