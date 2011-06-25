package Text::MultiMarkdown::XS;

use 5.010000;
use strict;
use warnings;
use Carp;

our $VERSION = '3.0.1';

require XSLoader;
XSLoader::load('Text::MultiMarkdown::XS', $VERSION);

sub new
{
    my ( $class, %p ) = @_;
    $p{ext} ||= 0;
    $p{output_format} ||= 0;

    my $self =  bless \%p, $class;
    $self->output( $p{output} ) if exists $p{output};
    $self->add_extension( $p{extensions} ) if exists $p{extensions};

    return $self;
}

sub markdown
{
    my ( $self, $text ) = @_;
    return to_string( $text, $self->{ext}, $self->{output_format} );
}

sub output
{
    my ( $self, $output ) = @_;

    my $format = Text::MultiMarkdown::XS::Format->HTML;
    SWITCH: {
        $output eq 'html'     && do { $format = Text::MultiMarkdown::XS::Format->HTML;     last SWITCH };
        $output eq 'latex'    && do { $format = Text::MultiMarkdown::XS::Format->LATEX;    last SWITCH };
        $output eq 'memoir'   && do { $format = Text::MultiMarkdown::XS::Format->MEMOIR;   last SWITCH };
        $output eq 'beamer'   && do { $format = Text::MultiMarkdown::XS::Format->BEAMER;   last SWITCH };
        $output eq 'opml'     && do { $format = Text::MultiMarkdown::XS::Format->OPML;     last SWITCH };
        $output eq 'groff_mm' && do { $format = Text::MultiMarkdown::XS::Format->GROFF_MM; last SWITCH };
        $output eq 'odf'      && do { $format = Text::MultiMarkdown::XS::Format->ODF;      last SWITCH };
        $output eq 'odf_body' && do { $format = Text::MultiMarkdown::XS::Format->ODF_BODY; last SWITCH };
    };

    $self->{output_format} = $format;

    return $self;
}

sub add_extension
{
    my ( $self, $extensions ) = @_;

    if ( ref( $extensions ) eq q{} )
    {
        my @ex = split( q{ }, $extensions );
        $extensions = \@ex;
    }

    my $ext = $self->{ext};
    foreach my $e ( @{$extensions} )
    {
        SWITCH: {
            $e eq 'smart'         && do { $ext |= Text::MultiMarkdown::XS::Extensions->SMART;         last SWITCH };
            $e eq 'notes'         && do { $ext |= Text::MultiMarkdown::XS::Extensions->NOTES;         last SWITCH };
            $e eq 'filter_html'   && do { $ext |= Text::MultiMarkdown::XS::Extensions->FILTER_HTML;   last SWITCH };
            $e eq 'filter_styles' && do { $ext |= Text::MultiMarkdown::XS::Extensions->FILTER_STYLES; last SWITCH };
            $e eq 'compatibility' && do { $ext |= Text::MultiMarkdown::XS::Extensions->COMPATIBILITY; last SWITCH };
            $e eq 'process_html'  && do { $ext |= Text::MultiMarkdown::XS::Extensions->PROCESS_HTML;  last SWITCH };
        };
    }
    $self->{ext} = $ext;

    return $self;
}

sub remove_extension
{
    my ( $self, $extensions ) = @_;

    my $ext = $self->{ext};

    foreach my $e ( @{$extensions} )
    {
        SWITCH: {
            $e eq 'smart'         && do { $ext ^= Text::MultiMarkdown::XS::Extensions->SMART;         last SWITCH };
            $e eq 'notes'         && do { $ext ^= Text::MultiMarkdown::XS::Extensions->NOTES;         last SWITCH };
            $e eq 'filter_html'   && do { $ext ^= Text::MultiMarkdown::XS::Extensions->FILTER_HTML;   last SWITCH };
            $e eq 'filter_styles' && do { $ext ^= Text::MultiMarkdown::XS::Extensions->FILTER_STYLES; last SWITCH };
            $e eq 'compatibility' && do { $ext ^= Text::MultiMarkdown::XS::Extensions->COMPATIBILITY; last SWITCH };
            $e eq 'process_html'  && do { $ext ^= Text::MultiMarkdown::XS::Extensions->PROCESS_HTML;  last SWITCH };
        };
    }
    $self->{ext} = $ext;

    return $self;
}

1;
__END__

=head1 NAME

Text::MultiMarkdown::XS - an extension of the multimarkdown c library.

=head1 SYNOPSIS

  use Text::MultiMarkdown::XS;

  my $m = Text::MultiMarkdown->new;
  my $html = $m->markdown($text);

=head1 DESCRIPTION

MultiMarkdown, or MMD, is a tool to help turn minimally marked-up plain text
into well formatted documents, including HTML, PDF (by way of LaTeX),
OPML, or OpenDocument (specifically, Flat OpenDocument or ‘.fodt’, which
can in turn be converted into RTF, Microsoft Word, or virtually any other
word-processing format).

=head1 USAGE

=head2 new

Create a new Text::MultiMarkdown::XS object.

=over 4

=item * C<output> (optional, default: html)

Set the output format.  Valid options are html, latex, memoir, beamer, opml, groff_mm, odf, odf_body.

example:

  my $m = Text::MultiMarkdown::XS->new( output => 'latex' );

=item * C<extensions> (options, default none)

Set the valid extensions.  Valid options are smart, notes, filter_html, filter_styles, compatibility, process_html.

example:

  my $m = Text::MultiMarkdown::XS->new( extensions => 'smart notes' ); # add the smart and notes extensions.

=back

=head2 markdown

Perform the conversion.  Returns the converted string.

example:

  my $html = $m->markdown($text);

=head2 output

Set the output format.  See C<new> for valid options.

=head2 add_extension

Add extra formatting extension(s).

=head2 remove_extension

Remove extra formatting extensions.

=head1 SEE ALSO

For more information about (original) Markdown's syntax, see:

    http://daringfireball.net/projects/markdown/

This module implements MultiMarkdown, which is an extension to Markdown.
Documentation may be found here:

    http://fletcherpenney.net/multimarkdown/

=head1 AUTHOR

Tom Heady <cpan@punch.net>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2011 by Tom Heady

This library is released under both the GPL and MIT licenses.
You may pick the license that best fits your needs.

peg-multimarkdown
portions Copyright (c) 2010-2011 Fletcher T. Penney

based on:

markdown in c, implemented using PEG grammar
Copyright (c) 2008-2011 John MacFarlane
ODF output code (c) 2011 Fletcher T. Penney

peg-markdown is released under both the GPL and MIT licenses.
You may pick the license that best fits your needs.

Additional MultiMarkdown files
Copyright (c) 2005-2011 Fletcher T. Penney

=cut
