
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

%to_ucs = ();
%from_ucs = ();
open(MAP, "< $map");
open(OUT, "> ${name}_ucs.map");
while(<MAP>) {
  /^#/ && next;
  s/#.*//;
  (($i, $u) = split(" ")) || next;
  $i = hex($i);
  $u = hex($u);
  $a = $i >> 8;
  $b = $i & 0x00FF;
  if ($a >= 0xA1 && $a <= 0xFE && $b >= 0xA1 && $b <= 0xFE) {
    ($i >= 0xA2E6 && $i <= 0xA2E7) || next;
  }
  ($u < 0x80) && next;
  $to_ucs{$i} = $u;
  $from_ucs{$u} = $i;
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

static wc_uint16 ${name}_ucs_map[ 0x20 * 0xB2 + 0x27 * 0x54 + 2 ] = {
EOF

for $ub (0x81 .. 0xA0) {
  for $lb (0x41 .. 0x5A, 0x61 .. 0x7A, 0x81 .. 0xFE) {
    $_ = ($ub << 8) + $lb;
    printf OUT "  0x%.4X,\t/* 0x%.4X */\n", $to_ucs{$_}, $_;
  }
}
for $ub (0xA1 .. 0xC7) {
  for $lb (0x41 .. 0x5A, 0x61 .. 0x7A, 0x81 .. 0xA0) {
    $_ = ($ub << 8) + $lb;
    printf OUT "  0x%.4X,\t/* 0x%.4X */\n", $to_ucs{$_}, $_;
  }
  if ($ub == 0xA2) {
  for $lb (0xE6 .. 0xE7) {
    $_ = ($ub << 8) + $lb;
    printf OUT "  0x%.4X,\t/* 0x%.4X */\n", $to_ucs{$_}, $_;
  }
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
uhc		VENDORS/MICSFT/WINDOWS/CP949.TXT	UHC/CP949 (Korean)
