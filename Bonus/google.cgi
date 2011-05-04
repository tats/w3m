#!/usr/bin/perl

# ~/.w3m/urimethodmap
# g:      file:/cgi-bin/google.cgi?%s
# google: file:/cgi-bin/google.cgi?%s

$url = "http://www.google.com/";
$_ = $ENV{"QUERY_STRING"};
s@^g(oogle)?:@@ && s@^//@@ && s@/$@@;
if ($_) {
	s/\+/ /g;
	s/%([\da-f][\da-f])/pack('C', hex($1))/egi;
	s/[\000-\040\+:#?&%<>"\177-\377]/sprintf('%%%02X', unpack('C', $&))/eg;
	$url .= "search?q=$_&hl=ja&lr=lang_ja&ie=EUC-JP";
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
