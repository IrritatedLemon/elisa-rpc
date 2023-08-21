PREFIX   := /usr/local

# Customize below to fit your system

ARCH     := $(shell uname -m)

LIBS     := gio-2.0

CXXFLAGS += -O2 -g -std=c++11 -xc++ -Wall -Werror -pedantic -pipe -Iexternal $(shell pkg-config --cflags $(LIBS))
LDFLAGS  += $(shell pkg-config --libs $(LIBS)) 