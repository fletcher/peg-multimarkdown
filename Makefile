ALL : multimarkdown

VERSION=3.2
PROGRAM=multimarkdown

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
	
CFLAGS ?= -Wall -O3 -ansi -include GLibFacade.h -I ./ -D MD_USE_GET_OPT=1

OBJS=markdown_parser.o markdown_output.o markdown_lib.o GLibFacade.o
PEGDIR=peg-0.1.4
LEG=$(PEGDIR)/leg

$(LEG):
	CC=gcc make -C $(PEGDIR)

%.o : %.c markdown_peg.h
	$(CC) -c $(CFLAGS) -o $@ $<

$(PROGRAM) : markdown.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $<
	@echo "$(FINALNOTES)"

markdown_parser.c : markdown_parser.leg $(LEG) markdown_peg.h parsing_functions.c utility_functions.c
	$(LEG) -o $@ $<

.PHONY: clean test

clean:
	rm -f markdown_parser.c $(PROGRAM) $(OBJS); \
	make -C $(PEGDIR) clean; \
	rm -rf mac_installer/Package_Root/usr/local/bin; \
	rm -rf mac_installer/Support_Root; \
	rm mac_installer/Resources/*.html; \
	rm windows_installer/README.txt; \
	rm windows_installer/multimarkdown.exe; \
	rm windows_installer/multimarkdown.xml.backup; \
	rm windows_installer/LICENSE.html; \
	rm -rf mac_installer/*.pkg

distclean: clean
	make -C $(PEGDIR) spotless

test: $(PROGRAM)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --Tidy  --Flags="--compatibility"

mmd-test: $(PROGRAM)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MultiMarkdownTests

compat-test: $(PROGRAM)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=CompatibilityTests --Flags="--compatibility"

latex-test: $(PROGRAM)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MultiMarkdownTests --Flags="-t latex" --ext=".tex"; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=BeamerTests --Flags="-t latex" --ext=".tex"; \
	./MarkdownTest.pl --Script=../$(PROGRAM) --testdir=MemoirTests --Flags="-t latex" --ext=".tex"

xslt-test: $(PROGRAM)
	cd MarkdownTest; \
	./MarkdownTest.pl --Script=/bin/cat --testdir=MultiMarkdownTests \
	--TrailFlags="| ../Support/bin/mmd2tex-xslt" --ext=".tex"; \
	./MarkdownTest.pl --Script=/bin/cat --testdir=BeamerTests \
	--TrailFlags="| ../Support/bin/mmd2tex-xslt" --ext=".tex"; \
	./MarkdownTest.pl --Script=/bin/cat --testdir=MemoirTests \
	--TrailFlags="| ../Support/bin/mmd2tex-xslt" --ext=".tex"; \

leak-check: $(PROGRAM)
	valgrind --leak-check=full ./multimarkdown TEST.markdown > TEST.html


# Compile multimarkdown.exe and prep files necessary for installer

windows: $(PROGRAM)
	rm *.o
	/usr/bin/i586-mingw32msvc-cc -c -Wall -O3 -ansi markdown*.c GLibFacade.c
	/usr/bin/i586-mingw32msvc-cc markdown*.o GLibFacade.o \
	-Wl,--dy,--warn-unresolved-symbols,-lglib-2.0 -o multimarkdown.exe
	mkdir -p windows_installer_
	cp multimarkdown.exe windows_installer/
	cp README.markdown windows_installer/README.txt
	./multimarkdown LICENSE > windows_installer/LICENSE.html

# After building the installer with BitRock, this creates a properly named
# zipfile

win-installer:
	zip -r windows_installer/MultiMarkdown-Windows-$(VERSION).zip windows_installer/MMD-windows-$(VERSION).exe -x windows_installer/MultiMarkdown*.zip


# Build Mac installer - requires that you first build multimarkdown itself,
# either with "make" or with Xcode

mac-installer: 
	mkdir -p mac_installer/Package_Root/usr/local/bin
	mkdir -p mac_installer/Support_Root/Library/Application\ Support
	cp multimarkdown scripts/mmd* mac_installer/Package_Root/usr/local/bin/
	./multimarkdown README > mac_installer/Resources/README.html
	./multimarkdown mac_installer/Resources/Welcome.txt > mac_installer/Resources/Welcome.html
	./multimarkdown LICENSE > mac_installer/Resources/License.html
	./multimarkdown mac_installer/Resources/Support_Welcome.txt > mac_installer/Resources/Support_Welcome.html
	git clone Support mac_installer/Support_Root/Library/Application\ Support/MultiMarkdown
	cd mac_installer; /Developer/usr/bin/packagemaker \
	--doc "Make Support Installer.pmdoc" \
	--title "MultiMarkdown Support Files" \
	--version $(VERSION) \
	--filter "\.DS_Store" \
	--filter "\.git" \
	--id net.fletcherpenney.MMD-Support.pkg \
	--domain user \
	--out "MultiMarkdown-Support-Mac-$(VERSION).pkg" \
	--no-relocate; \
	/Developer/usr/bin/packagemaker \
	--doc "Make OS X Installer.pmdoc" \
	--title "MultiMarkdown" \
	--version $(VERSION) \
	--filter "\.DS_Store" \
	--filter "\.git" \
	--id net.fletcherpenney.multimarkdown.pkg \
	--out "MultiMarkdown-Mac-$(VERSION).pkg";
	

# Requires installation of the platypus command line tool to create
# a drag and drop application for Mac OS X

drop: 
	mkdir drag; rm -rf drag/*.app; \
	/usr/local/bin/platypus -D -a 'MMD to LaTeX' -o 'Text Window' -p '/bin/sh' -V '3.0'  -I 'net.fletcherpenney.MMD2LaTeX' -X '*' -T '****|fold'  -N 'PATH=/usr/local/bin'  -c 'scripts/mmd2tex' 'drag/MMD2LaTeX.app'; \
	/usr/local/bin/platypus -D -a 'MMD to HTML' -o 'Text Window' -p '/bin/sh' -V '3.0'  -I 'net.fletcherpenney.MMD2HTML' -X '*' -T '****|fold'  -N 'PATH=/usr/local/bin'  -c 'scripts/mmd' 'drag/MMD2HTML.app'; \
	/usr/local/bin/platypus -D -a 'MMD to OPML' -o 'Text Window' -p '/bin/sh' -V '3.0'  -I 'net.fletcherpenney.MMD2OPML' -X '*' -T '****|fold'  -N 'PATH=/usr/local/bin'  -c 'scripts/mmd2opml' 'drag/MMD2OPML.app'; \
	/usr/local/bin/platypus -D -a 'MMD to ODF' -o 'Text Window' -p '/bin/sh' -V '3.0'  -I 'net.fletcherpenney.MMD2ODF' -X '*' -T '****|fold'  -N 'PATH=/usr/local/bin'  -c 'scripts/mmd2odf' 'drag/MMD2ODF.app'; 

# Create HTML and PDF (if latex installed) documentation
docs: $(PROGRAM)
	cd documentation; \
	../Support/Utilities/mmd_merge.pl index.txt > manual.txt; \
	mkdir -p ../manual; \
	../multimarkdown manual.txt > ../manual/index.html; \
	../multimarkdown -b -t latex manual.txt; \
	latexmk -pdf manual.tex; \
	latexmk -c manual.tex; \
	mv manual.pdf ../manual/mmd-manual.pdf; \
	rm ../documentation/manual.t*;


# For me to push updated documentation to my github site
docs-live: docs
	cd ../manual; git add mmd-manual.pdf index.html; \
	git commit -m "update manual"; git push origin gh-pages; \
