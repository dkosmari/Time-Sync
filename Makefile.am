EXTRA_DIST = \
	bootstrap \
	README.md


AM_CPPFLAGS =			\
	$(PORTLIBS_CPPFLAGS)	\
	$(WUMS_CPPFLAGS)	\
	$(WUPS_CPPFLAGS)	\
	$(WUT_CPPFLAGS)

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



noinst_LIBRARIES = libnet.a libwupsxx.a


libnet_a_SOURCES =					\
	src/net/address.cpp src/net/address.hpp		\
	src/net/addrinfo.cpp src/net/addrinfo.hpp	\
	src/net/error.cpp src/net/error.hpp		\
	src/net/socket.cpp src/net/socket.hpp


libwupsxx_a_SOURCES =							\
	src/wupsxx/bool_item.cpp src/wupsxx/bool_item.hpp		\
	src/wupsxx/category.cpp src/wupsxx/category.hpp			\
	src/wupsxx/config_error.cpp src/wupsxx/config_error.hpp		\
	src/wupsxx/duration_items.cpp src/wupsxx/duration_items.hpp	\
	src/wupsxx/int_item.cpp src/wupsxx/int_item.hpp			\
	src/wupsxx/item.cpp src/wupsxx/item.hpp				\
	src/wupsxx/numeric_item.hpp src/wupsxx/numeric_item_impl.hpp	\
	src/wupsxx/storage.cpp src/wupsxx/storage.hpp			\
	src/wupsxx/storage_error.cpp src/wupsxx/storage_error.hpp	\
	src/wupsxx/text_item.cpp src/wupsxx/text_item.hpp		\
	src/wupsxx/var_watch.hpp


PLUGIN_FILE = Time_Sync.wps
bin_PROGRAMS = Time_Sync.elf

Time_Sync_elf_SOURCES =							\
	src/async_queue.hpp						\
	src/cfg.cpp src/cfg.hpp						\
	src/clock_item.cpp src/clock_item.hpp				\
	src/config_screen.cpp src/config_screen.hpp			\
	src/core.cpp src/core.hpp					\
	src/curl.cpp src/curl.hpp					\
	src/http_client.cpp src/http_client.hpp				\
	src/logging.cpp src/logging.hpp					\
	src/main.cpp							\
	src/nintendo_glyphs.h						\
	src/notify.cpp src/notify.hpp					\
	src/ntp.cpp src/ntp.hpp						\
	src/preview_screen.cpp src/preview_screen.hpp			\
	src/thread_pool.cpp src/thread_pool.hpp				\
	src/time_utils.cpp src/time_utils.hpp				\
	src/timezone_offset_item.cpp src/timezone_offset_item.hpp	\
	src/timezone_query_item.cpp src/timezone_query_item.hpp		\
	src/utc.cpp src/utc.hpp						\
	src/utils.cpp src/utils.hpp					\
	src/verbosity_item.cpp src/verbosity_item.hpp


Time_Sync_elf_LDADD = libnet.a libwupsxx.a


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
	printf "%s\n" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
