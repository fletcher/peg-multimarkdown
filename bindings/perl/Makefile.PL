
use lib qw(inc);
use Devel::CheckLib;
use ExtUtils::MakeMaker;

my $inc = `pkg-config --cflags glib-2.0`;
my $libs = `pkg-config --libs glib-2.0`;

chomp $inc, $libs;

check_lib_or_exit(
    header => ['markdown_lib.h','glib.h'],
    INC => "-I. $inc -I./ext",
);

WriteMakefile(
    NAME              => 'Text::MultiMarkdown::XS',
    VERSION_FROM      => 'lib/Text/MultiMarkdown/XS.pm',
    PREREQ_PM         => {
        'Test::Simple' => 0.44,
    },
    ABSTRACT_FROM     => 'lib/Text/MultiMarkdown/XS.pm',
    AUTHOR            => 'Tom Heady (cpan@punch.net)',
    LIBS              => [$libs],
    DEFINE            => '',
    INC               => "-I. $inc -I./ext",
    OBJECT            => '$(O_FILES) ./ext/markdown_lib.o ./ext/markdown_parser.o ./ext/markdown_output.o',
    CCFLAGS           => '-Wall -ansi -O3',
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

