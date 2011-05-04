
@NAME = ();
while(<DATA>) {
	chop;
	s/\s+$//;
	(($n, $m, $d) = split(" ", $_, 3)) >= 3 || next;
	push(@NAME, $n);
	$MAP{$n} = $m;
	$DESC{$n} = $d;
}

%to_ucs = ();
%from_ucs = ();

foreach $name (@NAME) {

$map = $MAP{$name};
$desc = $DESC{$name};
print STDERR "$name\t$map\t$desc\n";

open(MAP, "< $map");
while(<MAP>) {
	(($c, $u) = split(" ")) || next;
	$c = hex($c);
	$u = hex($u);
	if (($c < 0xA140 || $c > 0xF9FE) &&
	    ($u > 0x20000)) {
		$u &= 0xFFFF;
		$to_ucs{$c} = $u;
		$from_ucs{$u} = $c;
	}
}
close(MAP);

}

$name = $NAME[0];
$desc = $DESC{$name};

open(OUT, "> ${name}_ucs_p2.map");

@ucs = sort { $a <=> $b } keys %to_ucs;
$nucs = @ucs + 0;

print OUT <<EOF;
/* $desc */

#define N_${name}_ucs_p2_map $nucs
/*
    HKSCS   UCS - 0x20000
*/
static wc_map ${name}_ucs_p2_map[ N_${name}_ucs_p2_map ] = {
EOF
for(@ucs) {
  $x = $to_ucs{$_};
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $x;
}

@ucs = sort { $a <=> $b } keys %from_ucs;
$nucs = @ucs + 0;

print OUT <<EOF;
};

#define N_ucs_p2_${name}_map $nucs
/*
    UCS - 0x20000   HKSCS
*/
static wc_map ucs_p2_${name}_map[ N_ucs_p2_${name}_map ] = {
EOF
for(@ucs) {
  $x = $from_ucs{$_};
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $x;
}

print OUT <<EOF;
};
EOF

__END__
hkscs	private/hkscs_p2.txt	HKSCS (Chinese Hong Kong)
