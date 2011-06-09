#define PERL_constant_NOTFOUND	1
#define PERL_constant_NOTDEF	2
#define PERL_constant_ISIV	3
#define PERL_constant_ISNO	4
#define PERL_constant_ISNV	5
#define PERL_constant_ISPV	6
#define PERL_constant_ISPVN	7
#define PERL_constant_ISSV	8
#define PERL_constant_ISUNDEF	9
#define PERL_constant_ISUV	10
#define PERL_constant_ISYES	11

#ifndef NVTYPE
typedef double NV; /* 5.6 and later define NVTYPE, and typedef NV to it.  */
#endif
#ifndef aTHX_
#define aTHX_ /* 5.6 or later define this for threading support.  */
#endif
#ifndef pTHX_
#define pTHX_ /* 5.6 or later define this for threading support.  */
#endif

static int
constant_15 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     EXT_FILTER_HTML GROFF_MM_FORMAT ODF_BODY_FORMAT */
  /* Offset 6 gives the best switch position.  */
  switch (name[6]) {
  case 'D':
    if (memEQ(name, "ODF_BODY_FORMAT", 15)) {
    /*                     ^               */
#ifdef ODF_BODY_FORMAT
      *iv_return = ODF_BODY_FORMAT;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'L':
    if (memEQ(name, "EXT_FILTER_HTML", 15)) {
    /*                     ^               */
#ifdef EXT_FILTER_HTML
      *iv_return = EXT_FILTER_HTML;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'M':
    if (memEQ(name, "GROFF_MM_FORMAT", 15)) {
    /*                     ^               */
#ifdef GROFF_MM_FORMAT
      *iv_return = GROFF_MM_FORMAT;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant (pTHX_ const char *name, STRLEN len, IV *iv_return) {
  /* Initially switch on the length of the name.  */
  /* When generated this function returned values for the list of names given
     in this section of perl code.  Rather than manually editing these functions
     to add or remove constants, which would result in this comment and section
     of code becoming inaccurate, we recommend that you edit this section of
     code, and use it to regenerate a new set of constant functions which you
     then use to replace the originals.

     Regenerate these constant functions by feeding this entire source file to
     perl -x

#!/usr/bin/perl -w
use ExtUtils::Constant qw (constant_types C_constant XS_constant);

my $types = {map {($_, 1)} qw(IV)};
my @names = (qw(BEAMER_FORMAT EXT_COMPATIBILITY EXT_FILTER_HTML
	       EXT_FILTER_STYLES EXT_NOTES EXT_PROCESS_HTML EXT_SMART
	       GROFF_MM_FORMAT HTML_FORMAT LATEX_FORMAT MEMOIR_FORMAT
	       ODF_BODY_FORMAT ODF_FORMAT OPML_FORMAT));

print constant_types(), "\n"; # macro defs
foreach (C_constant ("Markdown_lib", 'constant', 'IV', $types, undef, 3, @names) ) {
    print $_, "\n"; # C constant subs
}
print "\n#### XS Section:\n";
print XS_constant ("Markdown_lib", $types);
__END__
   */

  switch (len) {
  case 9:
    /* Names all of length 9.  */
    /* EXT_NOTES EXT_SMART */
    /* Offset 8 gives the best switch position.  */
    switch (name[8]) {
    case 'S':
      if (memEQ(name, "EXT_NOTE", 8)) {
      /*                       S     */
#ifdef EXT_NOTES
        *iv_return = EXT_NOTES;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'T':
      if (memEQ(name, "EXT_SMAR", 8)) {
      /*                       T     */
#ifdef EXT_SMART
        *iv_return = EXT_SMART;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 10:
    if (memEQ(name, "ODF_FORMAT", 10)) {
#ifdef ODF_FORMAT
      *iv_return = ODF_FORMAT;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 11:
    /* Names all of length 11.  */
    /* HTML_FORMAT OPML_FORMAT */
    /* Offset 1 gives the best switch position.  */
    switch (name[1]) {
    case 'P':
      if (memEQ(name, "OPML_FORMAT", 11)) {
      /*                ^                */
#ifdef OPML_FORMAT
        *iv_return = OPML_FORMAT;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'T':
      if (memEQ(name, "HTML_FORMAT", 11)) {
      /*                ^                */
#ifdef HTML_FORMAT
        *iv_return = HTML_FORMAT;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 12:
    if (memEQ(name, "LATEX_FORMAT", 12)) {
#ifdef LATEX_FORMAT
      *iv_return = LATEX_FORMAT;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 13:
    /* Names all of length 13.  */
    /* BEAMER_FORMAT MEMOIR_FORMAT */
    /* Offset 3 gives the best switch position.  */
    switch (name[3]) {
    case 'M':
      if (memEQ(name, "BEAMER_FORMAT", 13)) {
      /*                  ^                */
#ifdef BEAMER_FORMAT
        *iv_return = BEAMER_FORMAT;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'O':
      if (memEQ(name, "MEMOIR_FORMAT", 13)) {
      /*                  ^                */
#ifdef MEMOIR_FORMAT
        *iv_return = MEMOIR_FORMAT;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  case 15:
    return constant_15 (aTHX_ name, iv_return);
    break;
  case 16:
    if (memEQ(name, "EXT_PROCESS_HTML", 16)) {
#ifdef EXT_PROCESS_HTML
      *iv_return = EXT_PROCESS_HTML;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 17:
    /* Names all of length 17.  */
    /* EXT_COMPATIBILITY EXT_FILTER_STYLES */
    /* Offset 6 gives the best switch position.  */
    switch (name[6]) {
    case 'L':
      if (memEQ(name, "EXT_FILTER_STYLES", 17)) {
      /*                     ^                 */
#ifdef EXT_FILTER_STYLES
        *iv_return = EXT_FILTER_STYLES;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'M':
      if (memEQ(name, "EXT_COMPATIBILITY", 17)) {
      /*                     ^                 */
#ifdef EXT_COMPATIBILITY
        *iv_return = EXT_COMPATIBILITY;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

