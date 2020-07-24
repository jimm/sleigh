NAME = sleigh
# DEBUG = -DDEBUG -DDEBUG_STDERR

WXFLAGS := $(shell wx-config --cxxflags)
WXLIBS := $(shell wx-config --libs)

MACOS_VER = 10.9
CPP = $(shell wx-config --cxx)
CPPFLAGS += -std=c++11 -MD -MP -g $(DEBUG) -mmacosx-version-min=$(MACOS_VER) $(WXFLAGS)

LD = clang++
LIBS =	 -framework CoreMIDI -framework Foundation -lc -lc++
LDFLAGS += $(LIBS) $(WXLIBS)

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

SRC = $(wildcard src/*.cpp) $(wildcard src/wx/*.cpp)
OBJS = $(SRC:%.cpp=%.o)
TEST_SRC = $(wildcard test/*.cpp)
TEST_OBJS = $(TEST_SRC:%.cpp=%.o)
TEST_OBJ_FILTERS = src/wx/app_main.o

CATCH_CATEGORY ?= ""

.PHONY: all test install tags clean distclean

all: $(NAME)

$(NAME): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

-include $(SRC:%.cpp=%.d)
-include $(TEST_SRC:%.cpp=%.d)

test: $(NAME)_test
	./$(NAME)_test --use-colour no $(CATCH_CATEGORY)

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
