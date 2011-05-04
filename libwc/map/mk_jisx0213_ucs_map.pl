
@NAME = ();
while(<DATA>) {
  chop;
  s/\s*$//;
  (($n, $m, $c) = split(" ", $_, 3)) >= 3 || next;
  push(@NAME, $n);
  $MAP{$n} = $m;
  $CODE{$n} = $c;
}

foreach $name (@NAME) {

$code = $CODE{$name};
$map = $MAP{$name};

print "$name\t$map\t$code\n";

open(MAP, "< $map");
while(<MAP>) {
  /^#/ && next;
  s/#.*//;
  (($u, $s, $i) = split(" ")) || next;
  $u =~ s/^U\+2//;
  if ($i =~ s/3-//) {
    $i = hex($i);
    $u = hex($u);
    $to_ucs1{$i} = $u;
    $from_ucs1{$u} = $i;
  } elsif ($i =~ s/4-//) {
    $i = hex($i);
    $u = hex($u);
    $to_ucs2{$i} = $u;
    $from_ucs2{$u} = $i;
  }
}
open(OUT, "> ${name}_ucs.map");

@ucs = sort { $a <=> $b } keys %to_ucs1;
$nucs = @ucs + 0;
print OUT <<EOF;
/* $code */

#define N_${name}1_ucs_p2_map $nucs

static wc_map ${name}1_ucs_p2_map[ N_${name}1_ucs_p2_map ] = {
EOF
for(@ucs) {
  $_ || next;
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $to_ucs1{$_};
}

@ucs = sort { $a <=> $b } keys %from_ucs1;
$nucs = @ucs + 0;
print OUT <<EOF;
};

#define N_ucs_p2_${name}1_map $nucs

static wc_map ucs_p2_${name}1_map[ N_ucs_p2_${name}1_map ] = {
EOF
for(@ucs) {
  $_ || next;
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $from_ucs1{$_};
}

@ucs = sort { $a <=> $b } keys %to_ucs2;
$nucs = @ucs + 0;
print OUT <<EOF;
};

#define N_${name}2_ucs_p2_map $nucs

static wc_map ${name}2_ucs_p2_map[ N_${name}2_ucs_p2_map ] = {
EOF
for(@ucs) {
  $_ || next;
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $to_ucs2{$_};
}

@ucs = sort { $a <=> $b } keys %from_ucs2;
$nucs = @ucs + 0;
print OUT <<EOF;
};

#define N_ucs_p2_${name}2_map $nucs

static wc_map ucs_p2_${name}2_map[ N_ucs_p2_${name}2_map ] = {
EOF
for(@ucs) {
  $_ || next;
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $from_ucs2{$_};
}
print OUT <<EOF;
};
EOF

close(MAP);
close(OUT);
}

__END__
jisx0213	JISX0213.TXT		JIS X 0213 (Japanese)

