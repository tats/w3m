#!/usr/bin/perl

# ~/.w3m/urimethodmap
# e:      file:/cgi-bin/goodict.cgi?%s
# w:      file:/cgi-bin/goodict.cgi?%s
# j:      file:/cgi-bin/goodict.cgi?%s
# s:      file:/cgi-bin/goodict.cgi?%s

use NKF;
$url = "http://dictionary.goo.ne.jp";
$_ = $ENV{"QUERY_STRING"};
if (/^e:/) {
    $switch = 0;
} elsif (/^w:/) {
    $switch = 1;
} elsif (/^j:/) {
    $switch = 2;
} elsif (/^s:/) {
    $switch = 3;
}
s@^[ewjs]:@@ && s@^//@@ && s@/$@@;
if ($_) {
	s/\+/ /g;
	s/%([\da-f][\da-f])/pack('C', hex($1))/egi;
	$_ = nkf("-e", $_);
	s/[\000-\040\+:#?&%<>"\177-\377]/sprintf('%%%02X', unpack('C', $&))/eg;
	$url .= "/cgi-bin/dict_search.cgi?MT=$_&sw=$switch";
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
