
# gunzip -c xcin.linux.org.tw/pub/xcin/i18n/charset/BIG5.gz \
# | perl mk_big5_txt.pl \
# > private/big5.txt

while(<>) {
	s/^%IRREVERSIBLE%//;
	s/^<U([0-9A-F]{4})\>\s+\/x([0-9a-f]{2})\/x([0-9a-f]{2})\s+// || next;
	($c, $u) = ("$2$3", $1);
	$c =~ tr/a-f/A-F/;
	print "$c\t$u\n";
}
