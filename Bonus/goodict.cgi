#!/usr/bin/perl

# ~/.w3m/urimethodmap
# e:      file:/cgi-bin/goodict.cgi?%s
# w:      file:/cgi-bin/goodict.cgi?%s
# j:      file:/cgi-bin/goodict.cgi?%s

use NKF;
#$mode = 0; # substring
$mode = 1;  # perfect match
#$mode = 3; # search body text
$url = "http://dictionary.goo.ne.jp";
$_ = $ENV{"QUERY_STRING"};
if (/^e:/) {
    $kind = 'ej';
} elsif (/^w:/) {
    $kind = 'je';
} elsif (/^j:/) {
    $kind = 'jn';
}
s@^[ewjs]:@@ && s@^//@@ && s@/$@@;
if ($_) {
	s/\+/ /g;
	s/%([\da-f][\da-f])/pack('C', hex($1))/egi;
	$_ = nkf("-e", $_);
	s/[\000-\040\+:#?&%<>"\177-\377]/sprintf('%%%02X', unpack('C', $&))/eg;
	$url .= "/search.php?MT=$_&kind=$kind&mode=$mode";
} else {
	$input = "w3m-control: GOTO_LINK";
}
print <<EOF;
w3m-control: GOTO $url
w3m-control: DELETE_PREVBUF
w3m-control: SEARCH \\[
w3m-control: MOVE_RIGHT
${input}

EOF
