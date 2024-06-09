EXTRA_DIST = \
	bootstrap \
	README.md


AM_CPPFLAGS =			\
	$(PORTLIBS_CPPFLAGS)	\
	$(WUMS_CPPFLAGS)	\
	$(WUPS_CPPFLAGS)	\
	$(WUT_CPPFLAGS)		\
	-I$(top_srcdir)/include

AM_CXXFLAGS =			\
	$(WUT_CFLAGS)		\
	-ffunction-sections	\
	-fipa-pta

AM_LDFLAGS = \
	$(WUT_LDFLAGS) \
	$(WUPS_LDFLAGS)

LIBS =			\
	$(PORTLIBS_LIBS) \
	$(WUMS_LIBS)	\
	$(WUPS_LIBS)	\
	$(WUT_LIBS)

PLUGIN_FILE = Time_Sync.wps
bin_PROGRAMS = Time_Sync.elf

Time_Sync_elf_SOURCES =				\
	./include/async_queue.hpp		\
	./include/cfg.hpp			\
	./include/clock_item.hpp		\
	./include/config_screen.hpp		\
	./include/core.hpp			\
	./include/curl.hpp			\
	./include/http_client.hpp		\
	./include/logging.hpp			\
	./include/net/address.hpp		\
	./include/net/addrinfo.hpp		\
	./include/net/error.hpp			\
	./include/net/socket.hpp		\
	./include/nintendo_glyphs.h		\
	./include/notify.hpp			\
	./include/ntp.hpp			\
	./include/preview_screen.hpp		\
	./include/thread_pool.hpp		\
	./include/time_utils.hpp		\
	./include/timezone_offset_item.hpp	\
	./include/timezone_query_item.hpp	\
	./include/utc.hpp			\
	./include/utils.hpp			\
	./include/verbosity_item.hpp		\
	./include/wupsxx/bool_item.hpp		\
	./include/wupsxx/category.hpp		\
	./include/wupsxx/config_error.hpp	\
	./include/wupsxx/duration_items.hpp	\
	./include/wupsxx/int_item.hpp		\
	./include/wupsxx/item.hpp		\
	./include/wupsxx/numeric_item.hpp	\
	./include/wupsxx/storage.hpp		\
	./include/wupsxx/storage_error.hpp	\
	./include/wupsxx/text_item.hpp		\
	./include/wupsxx/var_watch.hpp		\
	./source/cfg.cpp			\
	./source/clock_item.cpp			\
	./source/config_screen.cpp		\
	./source/core.cpp			\
	./source/curl.cpp			\
	./source/http_client.cpp		\
	./source/logging.cpp			\
	./source/main.cpp			\
	./source/net/address.cpp		\
	./source/net/addrinfo.cpp		\
	./source/net/error.cpp			\
	./source/net/socket.cpp			\
	./source/notify.cpp			\
	./source/ntp.cpp			\
	./source/preview_screen.cpp		\
	./source/thread_pool.cpp		\
	./source/time_utils.cpp			\
	./source/timezone_offset_item.cpp	\
	./source/timezone_query_item.cpp	\
	./source/utc.cpp			\
	./source/utils.cpp			\
	./source/verbosity_item.cpp		\
	./source/wupsxx/bool_item.cpp		\
	./source/wupsxx/category.cpp		\
	./source/wupsxx/config_error.cpp	\
	./source/wupsxx/duration_items.cpp	\
	./source/wupsxx/int_item.cpp		\
	./source/wupsxx/item.cpp		\
	./source/wupsxx/numeric_item_impl.hpp	\
	./source/wupsxx/storage.cpp		\
	./source/wupsxx/storage_error.cpp	\
	./source/wupsxx/text_item.cpp


CLEANFILES = *.wps

all-local: $(PLUGIN_FILE)

%.strip.elf: %.elf
	$(STRIP) -g $< -o $@

%.wps: %.strip.elf
	$(ELF2RPL) $< $@


PHONY = upload
PLUGIN_PATH = /fs/vol/external01/wiiu/environments/aroma/plugins/
upload: $(PLUGIN_FILE)
	curl --upload-file $(PLUGIN_FILE) ftp://wiiu:$(PLUGIN_PATH)


PHONY += company

company: compile_flags.txt

compile_flags.txt: Makefile
	echo "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
