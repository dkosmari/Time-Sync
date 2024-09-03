# src/net/Makefile.am

AM_CPPFLAGS = $(DEVKITPRO_CPPFLAGS)

AM_CXXFLAGS = \
	-Wall -Wextra \
	$(DEVKITPRO_CFLAGS)


noinst_LIBRARIES = libnet.a


libnet_a_SOURCES = \
	address.cpp address.hpp		\
	addrinfo.cpp addrinfo.hpp	\
	error.cpp error.hpp		\
	socket.cpp socket.hpp
