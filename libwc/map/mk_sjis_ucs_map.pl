
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
  ($i >= 0x8740 && $i <= 0x87FC) ||
  ($i >= 0xED40 && $i <= 0xEEFC) ||
  ($i >= 0xFA40 && $i <= 0xFCFC) || next;
  $to_ucs{$i} = $u;
  if ($u > 0 && (! $from_ucs{$u} || ($from_ucs{$u} >= 0xED40 && $from_ucs{$u} <= 0xEEFC))) {
    $from_ucs{$u} = $i;
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

static wc_uint16 ${name}_ucs_map[ 0x5E * 10 ] = {
EOF

for $ub (0x87, 0xed, 0xee, 0xfa, 0xfb, 0xfc) {
  for $lb (0x40 .. 0x7E, 0x80 .. 0x9E) {
    $_ = ($ub << 8) + $lb;
    printf OUT "  0x%.4X,\t/* 0x%.4X */\n", $to_ucs{$_}, $_;
  }
  if ($ub == 0x87 || $ub == 0xfc) {
    next;
  }
  for $lb (0x9F .. 0xFC) {
    $_ = ($ub << 8) + $lb;
    printf OUT "  0x%.4X,\t/* 0x%.4X */\n", $to_ucs{$_}, $_;
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
sjis_ext	VENDORS/MICSFT/WINDOWS/CP932.TXT	Shift_JIS/CP932 (Japanese)
