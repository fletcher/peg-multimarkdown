
VERSION=3.2
LUALIBRARY=multimarkdownlualib.so


UNAME=$(shell uname)

ifeq ($(UNAME), Darwin)
define FINALNOTES
 ***\n\
*** WARNING: Since you are on Darwin, you probably meant to use the Xcode\n\
*** version instead.\n\
*** It produces a version of the binary that is capable of running on\n\
*** multiple versions of Mac OS X and on PPC, i386, or x86_64 machines.\n\
***
endef
else
	FINALNOTES=Build complete.
endif
	
CFLAGS ?= -fPIC -Wall -O3 -ansi -include ../GLibFacade.h -I ../ -D MD_USE_GET_OPT=1
LIB_OPTION= -shared #for Linux
LUA_INC= /usr/include/lua5.1

OBJS=markdown_parser.o markdown_output.o markdown_lib.o GLibFacade.o
PEGDIR=../peg-0.1.4
LEG=$(PEGDIR)/leg

ALL : $(LUALIBRARY)

$(LEG):
	CC=gcc make -C $(PEGDIR)

markdown_parser.c : ../markdown_parser.leg $(LEG) ../markdown_peg.h ../parsing_functions.c ../utility_functions.c
	$(LEG) -o $@ $<

markdown_parser.o: markdown_parser.c ../markdown_peg.h
	$(CC) -c $(CFLAGS) -o $@ $<

%.o : ../%.c ../markdown_peg.h
	$(CC) -c $(CFLAGS) -o $@ $<


INCS= -I$(LUA_INC)
$(LUALIBRARY) : luamarkdown.c $(OBJS)
	$(CC) $(CFLAGS) $(INCS) -o $@ $(LIB_OPTION) $(OBJS) $<

.PHONY: clean test lualib

lualib: $(LUALIBRARY)

clean:
	rm -f markdown_parser.c $(LUALIBRARY) $(OBJS); \
	make -C $(PEGDIR) clean; \

distclean: clean
	make -C $(PEGDIR) spotless

latex-test: $(LUALIBRARY)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MultiMarkdownTests --Flags="-t latex" --ext=".tex"; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=BeamerTests --Flags="-t latex" --ext=".tex"; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MemoirTests --Flags="-t latex" --ext=".tex"

