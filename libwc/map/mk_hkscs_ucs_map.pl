
@NAME = ();
while(<DATA>) {
	chop;
	s/\s+$//;
	(($n, $m, $d) = split(" ", $_, 3)) >= 3 || next;
	push(@NAME, $n);
	$MAP{$n} = $m;
	$DESC{$n} = $d;
}

@to_ucs = ();
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
	    (($u > 0 && $u < 0xE000) || $u > 0xF8FF)) {
		$to_ucs[$c] = $u;
		$from_ucs{$u} = $c;
	}
}
close(MAP);

}

$name = $NAME[0];
$desc = $DESC{$name};

open(OUT, "> ${name}_ucs.map");

print OUT <<EOF;
/* $desc */

static wc_uint16 ${name}_ucs_map[ 0x1E * 0x9D ] = {
EOF

for $i (0x88 .. 0xA0, 0xFA .. 0xFE) {
for $j (0x40 .. 0x7E, 0xA1 .. 0xFE) {
  $_ = $i * 0x100 + $j;
  $u = $to_ucs[$_];
  if ($u) {
    printf OUT " 0x%.4X,", $u;
  } else {
    print OUT " 0,\t";
  }
  printf OUT "\t/* 0x%.4X */\n", $_;
}
}

@ucs = sort { $a <=> $b } keys %from_ucs;
$nucs = @ucs + 0;

print OUT <<EOF;
};

#define N_ucs_${name}_map $nucs
/*
    UCS-2   HKSCS
*/
static wc_map ucs_${name}_map[ N_ucs_${name}_map ] = {
EOF
for(@ucs) {
  $x = $from_ucs{$_};
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $x;
}

print OUT <<EOF;
};
EOF

__END__
hkscs	private/hkscs.txt	HKSCS (Chinese Hong Kong)
