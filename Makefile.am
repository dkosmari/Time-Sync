EXTRA_DIST = \
	bootstrap \
	docker-build.sh \
	Dockerfile \
	LICENSE.md \
	README.md


SUBDIRS = \
	external/libcurlwrapper \
	external/libwupsxx

DIST_SUBDIRS = external


external/libwupsxx/lib/libwupsxx.la:
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C external/libwupsxx V=$V


AM_CPPFLAGS =					\
	$(WUPS_CPPFLAGS)			\
	$(LIBCURLWRAPPER_CPPFLAGS)		\
	$(LIBNOTIFICATIONS_CPPFLAGS)		\
	-I$(srcdir)/external/libwupsxx/include

AM_CXXFLAGS = \
	-Wall -Wextra -Werror \
	-pthread

AM_LDFLAGS = $(WUPS_LDFLAGS)

LDADD =						\
	src/net/libnet.la			\
	external/libwupsxx/lib/libwupsxx.la	\
	$(LIBCURLWRAPPER_LIB)			\
	$(LIBNOTIFICATIONS_LIB)			\
	$(WUPS_LIB)


noinst_LTLIBRARIES = src/net/libnet.la

src_net_libnet_la_SOURCES =		\
	src/net/address.cpp		\
	src/net/address.hpp		\
	src/net/addrinfo.cpp		\
	src/net/addrinfo.hpp		\
	src/net/error.cpp		\
	src/net/error.hpp		\
	src/net/socket.cpp		\
	src/net/socket.hpp


ELF_FILE := time-sync.elf
WPS_FILE := $(ELF_FILE:.elf=.wps)


noinst_PROGRAMS = $(ELF_FILE)

time_sync_elf_SOURCES =			\
	src/cfg.cpp			\
	src/cfg.hpp			\
	src/clock_item.cpp		\
	src/clock_item.hpp		\
	src/core.cpp			\
	src/core.hpp			\
	src/curl.cpp			\
	src/curl.hpp			\
	src/http_client.cpp		\
	src/http_client.hpp		\
	src/main.cpp			\
	src/notify.cpp			\
	src/notify.hpp			\
	src/ntp.cpp			\
	src/ntp.hpp			\
	src/preview_screen.cpp		\
	src/preview_screen.hpp		\
	src/synchronize_item.cpp	\
	src/synchronize_item.hpp	\
	src/time_utils.cpp		\
	src/time_utils.hpp		\
	src/time_zone_offset_item.cpp	\
	src/time_zone_offset_item.hpp	\
	src/time_zone_query_item.cpp	\
	src/time_zone_query_item.hpp	\
	src/utc.cpp			\
	src/utc.hpp			\
	src/utils.cpp			\
	src/utils.hpp			\
	src/verbosity_item.cpp		\
	src/verbosity_item.hpp



EXTRA_time_sync_elf_DEPENDENCIES = \
	$(LIBCURLWRAPPER_LIB)


all-local: $(WPS_FILE)


.PHONY: run wiiu-install wiiu-uninstall company


wiiu-install: $(WPS_FILE)
	curl "ftp://wiiu:/fs/vol/external01/wiiu/environments/aroma/plugins/" --upload-file $<


uninstall-local:
	curl "ftp://wiiu" --quote "DELE /fs/vol/external01/wiiu/environments/aroma/plugins/$(WPS_FILE)"


run: $(WPS_FILE)
	WIILOAD=tcp:wiiu wiiload $(WPS_FILE)


company: compile_flags.txt

compile_flags.txt: Makefile
	printf "%s" "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS)" | xargs -n1 | sort -u > compile_flags.txt
	$(CPP) -xc++ /dev/null -E -Wp,-v 2>&1 | sed -n 's,^ ,-I,p' >> compile_flags.txt


DISTCLEANFILES = $(AMINCLUDE)

@INC_AMINCLUDE@

