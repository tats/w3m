#!/usr/bin/perl
#
# [w3m-dev 03783]
# Install it in $LIB/utf8.cgi and configure keymap as
#   keymap  "x u"   GOTO file:/$LIB/utf8.cgi
#
$conv = "lv -Iu -Oe";
# $conv = "iconv -f UTF-8 -t EUC-JP";
$type = $ENV{W3M_TYPE} || "text/plain";
$url = $ENV{W3M_URL};
$file = $ENV{W3M_SOURCEFILE};
-f $file || exit;
$| = 1;
print <<EOF;
Content-Type: $type; charset=EUC-JP

EOF
if ($type =~ /^text\/html/i && $url) {
	print "<BASE HREF=\"$url\">\n";
}
exec split(" ", $conv), $file;
