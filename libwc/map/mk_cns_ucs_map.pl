
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

@to_ucs1 = ();
@to_ucs2 = ();
%from_ucs = ();
open(MAP, "< $map");
open(OUT, "> ${name}_ucs.map");
while(<MAP>) {
  /^#/ && next;
  s/#.*//;
  (($i, $u) = split(" ")) || next;
  $i =~ s/0x(.)/0x/;
  $p = $1;
  $i = hex($i);
  $u = hex($u);
  if ($p == 1) {
    $to_ucs1[$i] = $u;
    $from_ucs{$u} = $i;
  } elsif ($p == 2) {
    $to_ucs2[$i] = $u;
    $from_ucs{$u} = $i + 0x8000;
  }
}

# print OUT <<EOF;
# /*
#   These conversion tables between $code and
#   Unicode were made from
# 
#     ftp://ftp.unicode.org/Public/MAPPINGS/$map.
# */
print OUT <<EOF;
/* $code */

static wc_uint16 ${name}1_ucs_map[ 0x5E * 0x5E ] = {
EOF

for $i (0x21 .. 0x7E) {
for $j (0x21 .. 0x7E) {
  $_ = $i * 0x100 + $j;
  $u = $to_ucs1[$_];
  if ($u) {
    printf OUT " 0x%.4X,", $u;
  } else {
    print OUT " 0,\t";
  }
  printf OUT "\t/* 0x%.4X */\n", $_;
}
}

print OUT <<EOF;
};

static wc_uint16 ${name}2_ucs_map[ 0x5E * 0x5E ] = {
EOF

for $i (0x21 .. 0x7E) {
for $j (0x21 .. 0x7E) {
  $_ = $i * 0x100 + $j;
  $u = $to_ucs2[$_];
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
    UCS-2   CNS 11643-1
    UCS-2   CNS 11643-2 | 0x8000
*/
static wc_map ucs_${name}_map[ N_ucs_${name}_map ] = {
EOF
for(@ucs) {
  printf OUT "  { 0x%.4X, 0x%.4X },", $_, $from_ucs{$_};
  if ($from_ucs{$_} & 0x8000) {
    print OUT "\t/* CNS 11643-2 */\n";
  } else {
    print OUT "\t/* CNS 11643-1 */\n";
  }
}

print OUT <<EOF;
};
EOF

close(MAP);
}

__END__
cns11643	EASTASIA/OTHER/CNS11643.TXT	CNS 11643 (Chinese Taiwan)
