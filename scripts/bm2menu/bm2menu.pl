#!/usr/bin/perl

$PRE_MENU = "";
$POST_MENU = <<EOF;
 nop	"----------------------"
 func	"ブックマークに追加 (a)"	ADD_BOOKMARK	"aA"
EOF
# $POST_MENU = <<EOF;
#  nop	"----------------------"
#  func	"Add Bookmark       (a)"	ADD_BOOKMARK	"aA"
# EOF

@section = ();
%title = ();
%url = ();
while(<>) {
  if (/<h2>(.*)<\/h2>/) {
    $s = &unquote($1);
    push(@section, $s);
  } elsif (/<li><a href=\"(.*)\">(.*)<\/a>/) {
    $u = &unquote($1);
    $t = &unquote($2);
    $url{$s}   .= "$u\n";
    $title{$s} .= "$t\n";
  }
}

print "menu Bookmarks\n";
print $PRE_MENU;
foreach(@section) {
  print " popup\t\"$_\"\t\"$_\"\n"; 
}
print $POST_MENU;
print "end\n";

foreach(@section) {
  print "\n";
  print "menu \"$_\"\n";
  @ts = split("\n", $title{$_});
  @us = split("\n", $url{$_});
  while(@ts) {
    $t = shift @ts;
    $u = shift @us;
    print " func\t\"$t\"\tGOTO\t\"\"\t\"$u\"\n"; 
  }
  print "end\n";
}

sub unquote {
  local($_) = @_;

  s/\&lt;/\</g;
  s/\&gt;/\>/g;
  s/\&nbsp;/ /g;
  s/\&amp;/\&/g;

  return $_;
}
