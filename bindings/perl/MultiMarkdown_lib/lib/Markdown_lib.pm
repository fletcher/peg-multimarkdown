package Markdown_lib;

use 5.010000;
use strict;
use warnings;
use Carp;

our $VERSION = '0.01';

require XSLoader;
XSLoader::load('Markdown_lib', $VERSION);

1;
__END__

=head1 NAME

Markdown_lib - Perl extension for blah blah blah

=head1 SYNOPSIS

  use Markdown_lib;
  blah blah blah

=head1 DESCRIPTION

Stub documentation for Markdown_lib, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.

Blah blah blah.

=head2 EXPORT

None by default.

=head2 Exportable constants

  BEAMER_FORMAT
  EXT_COMPATIBILITY
  EXT_FILTER_HTML
  EXT_FILTER_STYLES
  EXT_NOTES
  EXT_PROCESS_HTML
  EXT_SMART
  GROFF_MM_FORMAT
  HTML_FORMAT
  LATEX_FORMAT
  MEMOIR_FORMAT
  ODF_BODY_FORMAT
  ODF_FORMAT
  OPML_FORMAT



=head1 SEE ALSO

Mention other useful documentation such as the documentation of
related modules or operating system documentation (such as man pages
in UNIX), or any relevant external documentation such as RFCs or
standards.

If you have a mailing list set up for your module, mention it here.

If you have a web site set up for your module, mention it here.

=head1 AUTHOR

thinc, E<lt>thinc@E<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2011 by thinc

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.10.0 or,
at your option, any later version of Perl 5 you may have available.


=cut
