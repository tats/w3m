
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

@to_ucs = ();
%from_ucs = ();
open(MAP, "< $map");
open(OUT, "> ${name}_ucs.map");
while(<MAP>) {
  /^#/ && next;
  s/#.*//;
  (($i, $u) = split(" ")) || next;
  $i = hex($i);
  $u = hex($u);
  $to_ucs[$i] = $u;
  if ($u > 0) {
    $from_ucs{$u} = $i;
  }
}

# compatibility with GBK(CP936), GB18030
delete $from_ucs{$to_ucs[0x2124]};
delete $from_ucs{$to_ucs[0x212A]};
$from_ucs{0x00B7} = 0x2124;
$from_ucs{0x2014} = 0x212A;
$to_ucs[0x2124] = 0x00B7;
$to_ucs[0x212A] = 0x2014;

# print OUT <<EOF;
# /*
#   These conversion tables between $code and
#   Unicode were made from
# 
#     ftp://ftp.unicode.org/Public/MAPPINGS/$map.
# */
print OUT <<EOF;
/* $code */

static wc_uint16 ${name}_ucs_map[ 0x5E * 0x5E ] = {
EOF

for $i (0x21 .. 0x7E) {
for $j (0x21 .. 0x7E) {
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

static wc_map ucs_${name}_map[ N_ucs_${name}_map ] = {
EOF
for(@ucs) {
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $from_ucs{$_};
}

print OUT <<EOF;
};
EOF

close(MAP);
}

__END__
gb2312		EASTASIA/GB/GB2312.TXT		GB 2312 (Chinese)
gb12345		EASTASIA/GB/GB12345.TXT		GB 12345 (Chinese)
