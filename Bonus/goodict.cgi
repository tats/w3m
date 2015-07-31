#!/usr/bin/perl

# ~/.w3m/urimethodmap
# e:      file:/cgi-bin/goodict.cgi?%s    Englich-Japanese
# w:      file:/cgi-bin/goodict.cgi?%s    Japanese-English
# j:      file:/cgi-bin/goodict.cgi?%s    Japanese
# a:      file:/cgi-bin/goodict.cgi?%s    All
#
# e:0:word  start with word
# e:1:word  perfect match
# e:2:word  end with word
# e:3:word  search body text
# e:6:word  search title
# e:word    perfect match

use Encode;
use Encode::Guess qw/euc-jp utf8/;
$url = "http://dictionary.goo.ne.jp";
$_ = $ENV{"QUERY_STRING"};
if (/^e:/) {
    $kind = 'ej';
} elsif (/^w:/) {
    $kind = 'je';
} elsif (/^j:/) {
    $kind = 'jn';
} elsif (/^a:/) {
    $kind = 'all'
}
s@^[ewja]:@@ && s@^//@@ && s@/$@@;
if (/^([01236]):/) {
    $mode=$1;
    s/^[01236]://;
}else{
    $mode="1";
}
if ($_) {
	s/\+/ /g;
	s/%([\da-f][\da-f])/pack('C', hex($1))/egi;
	$_ = encode("utf8", decode("Guess", $_));
	s/[\000-\040\+:#?&%<>"\177-\377]/sprintf('%%%02X', unpack('C', $&))/eg;
	$url .= "/srch/$kind/$_/m$mode"."u/";
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
