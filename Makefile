NAME = sleigh
DEBUG = -DDEBUG -DDEBUG_STDERR
MACOS_VER = 10.9
CPPFLAGS += -std=c++11 -mmacosx-version-min=$(MACOS_VER) -MD -g $(DEBUG)
LIBS = -framework AudioToolbox -framework CoreMIDI -framework Foundation \
	-lc -lc++ -lncurses
LDFLAGS += $(LIBS) -macosx_version_min $(MACOS_VER)

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

SRC = $(wildcard src/*.cpp) $(wildcard src/curses/*.cpp)
OBJS = $(SRC:%.cpp=%.o)
TEST_SRC = $(wildcard test/*.cpp)
TEST_OBJS = $(TEST_SRC:%.cpp=%.o)
TEST_OBJ_FILTERS = src/$(NAME).o

CATCH_CATEGORY ?= ""

.PHONY: all test install tags clean distclean

all: $(NAME)

$(NAME): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

-include $(C_SRC:%.c=%.d)
-include $(CPP_SRC:%.cpp=%.d)

test: $(NAME)_test
	./$(NAME)_test

$(NAME)_test:	$(OBJS) $(TEST_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(filter-out $(TEST_OBJ_FILTERS),$^)

install:	$(bindir)/$(NAME)

$(bindir)/$(NAME):	$(NAME)
	cp ./$(NAME) $(bindir)
	chmod 755 $(bindir)/$(NAME)

tags:	TAGS

clean:
	rm -f $(NAME) $(NAME)_test $(OBJS) $(TEST_OBJS)

distclean: clean
	rm -f src/*.d src/curses/*.d test/*.d
