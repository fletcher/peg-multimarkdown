
.PHONY: clean all perl

all: perl

perl: perl/Makefile
	$(MAKE) -C perl

perl/Makefile:
	cd perl && perl Makefile.PL

clean: perl/Makefile
	$(MAKE) -C perl/ clean

realclean: perl/Makefile
	$(MAKE) -C perl/ realclean
