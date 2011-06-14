BOOT:
{
  HV * stash;
  stash = gv_stashpv( "Markdown_lib::Extensions", TRUE );

  newCONSTSUB( stash, "SMART",         newSViv( EXT_SMART         ) );
  newCONSTSUB( stash, "NOTES",         newSViv( EXT_NOTES         ) );
  newCONSTSUB( stash, "FILTER_HTML",   newSViv( EXT_FILTER_HTML   ) );
  newCONSTSUB( stash, "FILTER_STYLES", newSViv( EXT_FILTER_STYLES ) );
  newCONSTSUB( stash, "COMPATIBILITY", newSViv( EXT_COMPATIBILITY ) );
  newCONSTSUB( stash, "PROCESS_HTML",  newSViv( EXT_PROCESS_HTML  ) );

  HV * format;
  format = gv_stashpv( "Markdown_lib::Format", TRUE );

  newCONSTSUB( format, "HTML",     newSViv( HTML_FORMAT     ) );
  newCONSTSUB( format, "LATEX",    newSViv( LATEX_FORMAT    ) );
  newCONSTSUB( format, "MEMOIR",   newSViv( MEMOIR_FORMAT   ) );
  newCONSTSUB( format, "BEAMER",   newSViv( BEAMER_FORMAT   ) );
  newCONSTSUB( format, "OPML",     newSViv( OPML_FORMAT     ) );
  newCONSTSUB( format, "GROFF_MM", newSViv( GROFF_MM_FORMAT ) );
  newCONSTSUB( format, "ODF",      newSViv( ODF_FORMAT ) );
  newCONSTSUB( format, "ODF_BODY", newSViv( ODF_BODY_FORMAT ) );
}
