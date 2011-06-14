use 5.010000;
use ExtUtils::MakeMaker;

my $inc = `pkg-config --cflags glib-2.0`;
my $libs = `pkg-config --libs glib-2.0`;

chomp $inc;
chomp $libs;

WriteMakefile(
    NAME              => 'Markdown_lib',
    VERSION_FROM      => 'lib/Markdown_lib.pm', # finds $VERSION
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ABSTRACT_FROM     => 'lib/Markdown_lib.pm', # retrieve abstract from module
    AUTHOR            => 'thinc <thinc@>',
    LIBS              => [$libs], # e.g., '-lm'
    DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
    INC               => "-I. $inc -I../../../", # e.g., '-I. -I/usr/include/other'
    OBJECT            => '$(O_FILES) ../../../markdown_lib.o ../../../markdown_parser.o ../../../markdown_output.o', # link all the C files too
    CCFLAGS           => '-Wall -ansi -O3 -fPIC',
);

# we need to tell it how to compile ../../../markdown_{lib,parser,output}.o
package MY;
sub MY::postamble {
    return <<'MAKE_FRAG';

../../../markdown_lib.o :
	make -C ../../../ markdown_lib.o

../../../markdown_parser.o :
	make -C ../../../ markdown_parser.o

../../../markdown_output.o :
	make -C ../../../ markdown_output.o

MAKE_FRAG
}

