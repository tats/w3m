
# gunzip -c xcin.linux.org.tw/pub/xcin/i18n/charset/BIG5HKSCS.gz \
# | perl mk_hkscs_txt.pl \
# > private/hkscs.txt

while(<>) {
	s/^%IRREVERSIBLE%//;
	s/^<U([0-9A-F]{4})\>\s+\/x([0-9a-f]{2})\/x([0-9a-f]{2})\s+// || next;
	($c, $u) = ("$2$3", $1);
	$c =~ tr/a-f/A-F/;
	print "$c\t$u\n";
}
