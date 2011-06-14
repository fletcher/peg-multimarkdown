#!/usr/bin/perl

use strict;
use warnings;

use Test::More;
BEGIN { use_ok('Text::MultiMarkdown::XS') };

is( Text::MultiMarkdown::XS::sanity_check() , 12356894 , 'sanity check passes' );

is( Text::MultiMarkdown::XS::Extensions->SMART,         1,  'const SMART defined' );
is( Text::MultiMarkdown::XS::Extensions->NOTES,         2,  'const NOTES defined' );
is( Text::MultiMarkdown::XS::Extensions->FILTER_HTML,   4,  'const FILTER_HTML defined' );
is( Text::MultiMarkdown::XS::Extensions->FILTER_STYLES, 8,  'const FILTER_STYLES defined' );
is( Text::MultiMarkdown::XS::Extensions->COMPATIBILITY, 16, 'const COMPATIBILITY defined' );
is( Text::MultiMarkdown::XS::Extensions->PROCESS_HTML,  32, 'const PROCESS_HTML defined' );

is( Text::MultiMarkdown::XS::Format->HTML,     0, 'const HTML defined' );
is( Text::MultiMarkdown::XS::Format->LATEX,    1, 'const LATEX defined' );
is( Text::MultiMarkdown::XS::Format->MEMOIR,   2, 'const MEMOIR defined' );
is( Text::MultiMarkdown::XS::Format->BEAMER,   3, 'const BEAMER defined' );
is( Text::MultiMarkdown::XS::Format->OPML,     4, 'const OPML_MM defined' );
is( Text::MultiMarkdown::XS::Format->GROFF_MM, 5, 'const GROFF_MM defined' );
is( Text::MultiMarkdown::XS::Format->ODF,      6, 'const ODF defined' );
is( Text::MultiMarkdown::XS::Format->ODF_BODY, 7, 'const ODF_BODY_FORMAT defined' );

my $mmd = '**bold text**';

{
    my $wanted = '<p><strong>bold text</strong></p>';
    my $given = Text::MultiMarkdown::XS::to_string( $mmd, 0, Text::MultiMarkdown::XS::Format->HTML );
    is ( $given, $wanted, 'conversion to html works' );
}
{
    my $wanted = '\textbf{bold text}';
    my $given = Text::MultiMarkdown::XS::to_string( $mmd, 0, Text::MultiMarkdown::XS::Format->LATEX );
    is ( $given, $wanted, 'conversion to latex works' );
}

done_testing();
