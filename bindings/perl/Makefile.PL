use 5.010000;
use ExtUtils::MakeMaker;

my $inc = `pkg-config --cflags glib-2.0`;
my $libs = `pkg-config --libs glib-2.0`;

chomp $inc;
chomp $libs;

WriteMakefile(
    NAME              => 'Text::MultiMarkdown::XS',
    VERSION_FROM      => 'lib/Text/MultiMarkdown/XS.pm', # finds $VERSION
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ABSTRACT_FROM     => 'lib/Text/MultiMarkdown/XS.pm', # retrieve abstract from module
    AUTHOR            => 'thinc <thinc@>',
    LIBS              => [$libs], # e.g., '-lm'
    DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
    INC               => "-I. $inc -I./ext", # e.g., '-I. -I/usr/include/other'
    OBJECT            => '$(O_FILES) ./ext/markdown_lib.o ./ext/markdown_parser.o ./ext/markdown_output.o', # link all the C files too
    CCFLAGS           => '-Wall -ansi -O3 -fPIC',
);

# we need to tell it how to compile ../../markdown_{lib,parser,output}.o
package MY;
sub MY::postamble {
    return <<'MAKE_FRAG';

./ext/markdown_lib.o :
	make -C ./ext/ markdown_lib.o

./ext/markdown_parser.o :
	make -C ./ext/ markdown_parser.o

./ext/markdown_output.o :
	make -C ./ext/ markdown_output.o

MAKE_FRAG
}

