
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
    ($i >= 0xA2A1 && $i <= 0xA2AA) ||
    ($i >= 0xA6E0 && $i <= 0xA6F5) ||
    ($i >= 0xA8BB && $i <= 0xA8BB) ||
    ($i >= 0xA8BD && $i <= 0xA8BD) ||
    ($i >= 0xA8BE && $i <= 0xA8BE) ||
    ($i >= 0xA8C0 && $i <= 0xA8C0) || next;
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

#define N_ucs_gbk_80_map 1

static wc_map ucs_gbk_80_map[ N_ucs_gbk_80_map ] = {
  { 0x20AC, 0x0080 },
};

static wc_uint16 ${name}_ucs_map[ 0x7E * 0xBE - 0x5E * 0x5E + 0x0A + 0x16 + 0x06 ] = {
EOF

for $ub (0x81 .. 0xA0) {
  for $lb (0x40 .. 0x7E, 0x80 .. 0xFE) {
    $_ = ($ub << 8) + $lb;
    printf OUT "  0x%.4X,\t/* 0x%.4X */\n", $to_ucs{$_}, $_;
  }
}
for $ub (0xA1 .. 0xFE) {
  for $lb (0x40 .. 0x7E, 0x80 .. 0xA0) {
    $_ = ($ub << 8) + $lb;
    printf OUT "  0x%.4X,\t/* 0x%.4X */\n", $to_ucs{$_}, $_;
  }
  if ($ub == 0xA2) {
  for $lb (0xA1 .. 0xAA) {	# 0x0A
    $_ = ($ub << 8) + $lb;
    printf OUT "  0x%.4X,\t/* 0x%.4X */\n", $to_ucs{$_}, $_;
  }
  } elsif ($ub == 0xA6) {
  for $lb (0xE0 .. 0xF5) {	# 0x16
    $_ = ($ub << 8) + $lb;
    printf OUT "  0x%.4X,\t/* 0x%.4X */\n", $to_ucs{$_}, $_;
  }
  } elsif ($ub == 0xA8) {
  for $lb (0xBB .. 0xC0) {	# 0x06
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
gbk		VENDORS/MICSFT/WINDOWS/CP936.TXT	GBK/CP936 (Chinese)
