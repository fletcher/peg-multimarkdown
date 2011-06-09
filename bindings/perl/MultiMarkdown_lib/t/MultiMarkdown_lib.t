# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl Markdown_lib.t'

#########################

# change 'tests => 2' to 'tests => last_test_to_print';

use Test::More tests => 2;
BEGIN { use_ok('Markdown_lib') };


my $fail = 0;
foreach my $constname (qw(
	BEAMER_FORMAT EXT_COMPATIBILITY EXT_FILTER_HTML EXT_FILTER_STYLES
	EXT_NOTES EXT_PROCESS_HTML EXT_SMART GROFF_MM_FORMAT HTML_FORMAT
	LATEX_FORMAT MEMOIR_FORMAT ODF_BODY_FORMAT ODF_FORMAT OPML_FORMAT)) {
  next if (eval "my \$a = $constname; 1");
  if ($@ =~ /^Your vendor has not defined Markdown_lib macro $constname/) {
    print "# pass: $@";
  } else {
    print "# fail: $@";
    $fail = 1;
  }

}

ok( $fail == 0 , 'Constants' );
#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

