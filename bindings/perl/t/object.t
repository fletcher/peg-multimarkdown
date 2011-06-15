#!/usr/bin/perl

use strict;
use warnings;

use Test::More;
BEGIN { use_ok('Text::MultiMarkdown::XS') };

my $text = '**bold text**';

{
    my $m = Text::MultiMarkdown::XS->new;
    my $html = $m->markdown($text);
    my $wanted = '<p><strong>bold text</strong></p>';
    is ( $html, $wanted, 'markdown method works' );
}
{
    my $m = Text::MultiMarkdown::XS->new( output => 'latex' );
    my $latex = $m->markdown($text);

    my $wanted = '\textbf{bold text}';
    is ( $latex, $wanted, 'conversion to latex works' );
}

{
    my $m = Text::MultiMarkdown::XS->new;
    $m->output( 'latex' );
    my $latex = $m->markdown($text);

    my $wanted = '\textbf{bold text}';
    is ( $latex, $wanted, 'conversion to latex works' );
}

{
    my $text = q{this -- is 'single quoted' text};

    my $m = Text::MultiMarkdown::XS->new( output => 'html', extensions => [qw(smart)] );
    my $html = $m->markdown($text);

    my $wanted = q{<p>this &#8211; is &#8216;single quoted&#8217; text</p>};
    is ( $html, $wanted, 'conversion to html with smart extension works' );
}

{
    my $text = q{this -- is 'single quoted' text};

    my $m = Text::MultiMarkdown::XS->new( output => 'html' );
    $m->add_extension(['smart']);
    my $html = $m->markdown($text);

    my $wanted = q{<p>this &#8211; is &#8216;single quoted&#8217; text</p>};
    is ( $html, $wanted, 'add_extension works' );

    $m->remove_extension(['smart']);
    my $html2 = $m->markdown($text);
    my $wanted2 = qq{<p>$text</p>};
    $m->markdown($text);
    is ( $html2, $wanted2, 'remove_extension works' );
}

{
    my $text = q{this -- is 'single quoted' text};

    my $m = Text::MultiMarkdown::XS->new( extensions => 'smart notes' );
    my $html = $m->markdown($text);

    my $wanted = q{<p>this &#8211; is &#8216;single quoted&#8217; text</p>};
    is ( $html, $wanted, 'extensions can be a string' );
}

done_testing();
