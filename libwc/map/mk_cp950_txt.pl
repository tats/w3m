
# perl mk_cp950_txt.pl \
# www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP950.TXT \
# > private/cp950.txt

while(<>) {
	s/^0x([0-9A-F]{4})\s+0x([0-9A-F]{4})\s+#// || next;
	($c, $u) = ($1, $2);
	print "$c\t$u\n";
}
