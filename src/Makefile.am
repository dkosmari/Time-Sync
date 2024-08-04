# src/Makefile.am

SUBDIRS = net wupsxx


AM_CPPFLAGS = $(DEVKITPRO_CPPFLAGS)
AM_CXXFLAGS = $(DEVKITPRO_CFLAGS)
AM_LDFLAGS = $(DEVKITPRO_LDFLAGS)
LIBS = $(DEVKITPRO_LIBS)


WPS_FILE = Time_Sync.wps
noinst_PROGRAMS = Time_Sync.elf


Time_Sync_elf_SOURCES = \
	async_queue.hpp						\
	cfg.cpp cfg.hpp						\
	clock_item.cpp clock_item.hpp				\
	config_screen.cpp config_screen.hpp			\
	core.cpp core.hpp					\
	curl.cpp curl.hpp					\
	http_client.cpp http_client.hpp				\
	logging.cpp logging.hpp					\
	main.cpp						\
	nintendo_glyphs.h					\
	notify.cpp notify.hpp					\
	ntp.cpp ntp.hpp						\
	preview_screen.cpp preview_screen.hpp			\
	thread_pool.cpp thread_pool.hpp				\
	time_utils.cpp time_utils.hpp				\
	timezone_offset_item.cpp timezone_offset_item.hpp	\
	timezone_query_item.cpp timezone_query_item.hpp		\
	utc.cpp utc.hpp						\
	utils.cpp utils.hpp					\
	verbosity_item.cpp verbosity_item.hpp


Time_Sync_elf_LDADD = net/libnet.a wupsxx/libwupsxx.a


all-local: $(WPS_FILE)


install-exec-local: $(WPS_FILE)
	curl "ftp://wiiu:/fs/vol/external01/wiiu/environments/aroma/plugins/" --upload-file $<

uninstall-local:
	curl "ftp://wiiu" --quote "DELE /fs/vol/external01/wiiu/environments/aroma/plugins/$(WPS_FILE)"


run-local: all
	WIILOAD=tcp:wiiu wiiload $(WPS_FILE)


PHONY: company
company: compile_flags.txt
compile_flags.txt: Makefile
	printf "%s\n" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt


@INC_AMINCLUDE@

