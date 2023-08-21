include config.mk

SRCDIR   := src
OBJDIR   := build

EXE      := $(OBJDIR)/elisa-rpc

SRC      := $(shell find $(SRCDIR) external/discord -type f -name '*.cpp')
HEADERS  := $(shell find $(SRCDIR) external/discord -type f -name '*.h*')
VPATH    := $(shell find $(SRCDIR) external/discord -type d -print | tr '\n' ':' | sed 's/:$$//')

OBJ      := $(addprefix $(OBJDIR)/, $(notdir $(SRC:.cpp=.o)))
DEP      := $(addprefix $(OBJDIR)/, $(notdir $(SRC:.cpp=.d)))

LDFLAGS  += -Lexternal/discord/lib/$(ARCH) -l:discord_game_sdk.so

all: $(EXE)

options:
	@echo elisa-rpc build options:
	@echo "CXXFLAGS = $(CXXFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CXX      = $(CXX)"
	@echo $(ARCH)

$(OBJDIR): ; mkdir -p $@

$(OBJ): $(OBJDIR)/%.o : %.cpp $(OBJDIR)/%.d | $(OBJDIR)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

# https://stackoverflow.com/a/39003791
$(DEP): $(OBJDIR)/%.d: %.cpp | $(OBJDIR)
	@set -e; rm -f $@; \
         $(CC) -M $(CXXFLAGS) $< > $@.$$$$; \
         sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
         rm -f $@.$$$$

$(EXE): $(OBJ) $(HEADERS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ) $(shell find external/discord -type f -name '*.o')

clean:
	rm -rf build

install: $(EXE)
	install -D -m 755 -t $(PREFIX)/lib external/discord/lib/$(ARCH)/discord_game_sdk.so
	install -D -m 755 -t $(PREFIX)/bin $(EXE)

uninstall:
	rm -rf $(PREFIX)/lib/discord_game_sdk.so
	rm -rf $(PREFIX)/bin/$(notdir $(EXE))

run: $(OBJDIR)/elisa-rpc
	@$< || true

gdb: $(OBJDIR)/elisa-rpc
	@gdb ./$<

valgrind: $(OBJDIR)/elisa-rpc
	@valgrind --leak-check=full --track-origins=yes ./$<

include $(wildcard $(DEP))

.PHONY: all options clean install uninstall run gdb valgrind
