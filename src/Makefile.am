# src/Makefile.am

SUBDIRS = net wupsxx


AM_CPPFLAGS = $(ALL_CPPFLAGS)
AM_CXXFLAGS = $(ALL_CXXFLAGS)
AM_LDFLAGS = $(ALL_LDFLAGS)
LIBS = $(ALL_LIBS)


PLUGIN_FILE = Time_Sync.wps
bin_PROGRAMS = Time_Sync.elf


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


CLEANFILES = *.wps

all-local: $(PLUGIN_FILE)

%.strip.elf: %.elf
	$(STRIP) -g $< -o $@

%.wps: %.strip.elf
	$(ELF2RPL) $< $@


PHONY = upload

PLUGIN_PATH = /fs/vol/external01/wiiu/environments/aroma/plugins/

upload: $(PLUGIN_FILE)
	curl --upload-file $< ftp://wiiu:$(PLUGIN_PATH)


PHONY += company

company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s\n" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
