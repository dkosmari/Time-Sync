EXTRA_DIST = \
	bootstrap \
	docker-build.sh \
	Dockerfile \
	LICENSE.md \
	README.md


SUBDIRS = $(subdirs) $(subdirs_extra)


AM_CPPFLAGS = $(DEVKITPRO_CPPFLAGS)

AM_CXXFLAGS = \
	-Wall -Wextra -Werror \
	$(DEVKITPRO_CFLAGS)

AM_LDFLAGS = $(DEVKITPRO_LDFLAGS)

LIBS = $(DEVKITPRO_LIBS)


noinst_LIBRARIES = src/net/libnet.a

src_net_libnet_a_SOURCES = \
	src/net/address.cpp src/net/address.hpp		\
	src/net/addrinfo.cpp src/net/addrinfo.hpp	\
	src/net/error.cpp src/net/error.hpp		\
	src/net/socket.cpp src/net/socket.hpp


WPS_FILE = time-sync.wps
noinst_PROGRAMS = time-sync.elf

time_sync_elf_SOURCES = \
	src/async_queue.hpp						\
	src/cfg.cpp src/cfg.hpp						\
	src/clock_item.cpp src/clock_item.hpp				\
	src/core.cpp src/core.hpp					\
	src/curl.cpp src/curl.hpp					\
	src/http_client.cpp src/http_client.hpp				\
	src/main.cpp							\
	src/notify.cpp src/notify.hpp					\
	src/ntp.cpp src/ntp.hpp						\
	src/preview_screen.cpp src/preview_screen.hpp			\
	src/synchronize_item.cpp src/synchronize_item.hpp		\
	src/thread_pool.cpp src/thread_pool.hpp				\
	src/time_utils.cpp src/time_utils.hpp				\
	src/time_zone_offset_item.cpp src/time_zone_offset_item.hpp	\
	src/time_zone_query_item.cpp src/time_zone_query_item.hpp	\
	src/utc.cpp src/utc.hpp						\
	src/utils.cpp src/utils.hpp					\
	src/verbosity_item.cpp src/verbosity_item.hpp

time_sync_elf_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/external/libwupsxx/include

time_sync_elf_LDADD = \
	$(builddir)/src/net/libnet.a \
	$(builddir)/external/libwupsxx/src/libwupsxx.a




all-local: $(WPS_FILE)


install-exec-local: $(WPS_FILE)
	curl "ftp://wiiu:/fs/vol/external01/wiiu/environments/aroma/plugins/" --upload-file $<

uninstall-local:
	curl "ftp://wiiu" --quote "DELE /fs/vol/external01/wiiu/environments/aroma/plugins/$(WPS_FILE)"


.PHONY: run company


run: all
	WIILOAD=tcp:wiiu wiiload $(WPS_FILE)


company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > $(srcdir)/compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> $(srcdir)/compile_flags.txt


@INC_AMINCLUDE@
DISTCLEANFILES = $(AMINCLUDE)

