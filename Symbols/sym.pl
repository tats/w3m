
while(<>) {
	for(split('')) {
		$n = ord($_);
		if ($n & 0x80) {
			printf("\\%.3o", $n);
		} else {
			print $_;
		}
	}
}
