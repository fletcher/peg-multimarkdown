#!/usr/bin/perl

use strict;
use warnings;

use Test::More;
BEGIN { use_ok('Markdown_lib') };

is( Markdown_lib::sanity_check() , 12356894 , 'sanity check passes' );

is( Markdown_lib::Extensions->SMART,         1,  'const SMART defined' );
is( Markdown_lib::Extensions->NOTES,         2,  'const NOTES defined' );
is( Markdown_lib::Extensions->FILTER_HTML,   4,  'const FILTER_HTML defined' );
is( Markdown_lib::Extensions->FILTER_STYLES, 8,  'const FILTER_STYLES defined' );
is( Markdown_lib::Extensions->COMPATIBILITY, 16, 'const COMPATIBILITY defined' );
is( Markdown_lib::Extensions->PROCESS_HTML,  32, 'const PROCESS_HTML defined' );

is( Markdown_lib::Format->HTML,     0, 'const HTML defined' );
is( Markdown_lib::Format->LATEX,    1, 'const LATEX defined' );
is( Markdown_lib::Format->MEMOIR,   2, 'const MEMOIR defined' );
is( Markdown_lib::Format->BEAMER,   3, 'const BEAMER defined' );
is( Markdown_lib::Format->OPML,     4, 'const OPML_MM defined' );
is( Markdown_lib::Format->GROFF_MM, 5, 'const GROFF_MM defined' );
is( Markdown_lib::Format->ODF,      6, 'const ODF defined' );
is( Markdown_lib::Format->ODF_BODY, 7, 'const ODF_BODY_FORMAT defined' );

my $mmd = '**bold text**';

{
    my $wanted = '<p><strong>bold text</strong></p>';
    my $given = Markdown_lib::to_string( $mmd, 0, Markdown_lib::Format->HTML );
    is ( $given, $wanted, 'conversion to html works' );
}
{
    my $wanted = '\textbf{bold text}';
    my $given = Markdown_lib::to_string( $mmd, 0, Markdown_lib::Format->LATEX );
    is ( $given, $wanted, 'conversion to latex works' );
}

done_testing();
