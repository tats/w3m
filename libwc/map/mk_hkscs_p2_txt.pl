
# gunzip -c www.unicode.org/Public/BETA/Unicode3.2/Unihan-3.2.0d2.txt.gz \
# | perl mk_hkscs_p2_txt.pl \
# | sort > private/hkscs_p2.txt

while(<>) {
	/^U\+([0-9A-F]{4,5})\s+kHKSCS\s+([0-9A-F]{4})/ || next;
	($c, $u) = ($2, $1);
	print "$c\t$u\n";
}
