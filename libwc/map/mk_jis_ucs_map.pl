
@NAME = ();
while(<DATA>) {
  chop;
  s/\s*$//;
  (($n, $m, $c) = split(" ", $_, 3)) >= 3 || next;
  push(@NAME, $n);
  $MAP{$n} = $m;
  $CODE{$n} = $c;
}

$name0 = "jisx0208x0212x0213";
# print OUT <<EOF;
# /*
#   These conversion tables between $code and
#   Unicode were made from
# 
#     ftp://ftp.unicode.org/Public/MAPPINGS/$MAP{$NAME[0]},
#     ftp://ftp.unicode.org/Public/MAPPINGS/$MAP{$NAME[1]}.
# 
# */
# EOF
# Unicode(CP932)				Unicode(JIS X 0208)
%from_ucs = (
  0x00A5, 0x216F, # YEN SIGN
  0x00B5, 0x264C, # MICRO SIGN
  0xFF3C, 0x2140, # FULLWIDTH REVERSE SOLIDUS	0x005C REVERSE SOLIDUS
  0xFF5E, 0x2141, # FULLWIDTH TILDE		0x301C WAVE DASH
  0x2225, 0x2142, # PARALLEL TO			0x2016 DOUBLE VERTICAL LINE
  0xFF0D, 0x215D, # FULLWIDTH HYPHEN-MINUS	0x2212 MINUS SIGN
  0xFFE0, 0x2171, # FULLWIDTH CENT SIGN		0x00A2 CENT SIGN
  0xFFE1, 0x2172, # FULLWIDTH POUND SIGN	0x00A3 POUND SIGN
  0xFFE2, 0x224C, # FULLWIDTH NOT SIGN		0x00AC NOT SIGN
);
for(keys %from_ucs) {
  ($_ == 0x00A5) && next;
  ($_ == 0x00B5) && next;
  $to_ucs[$from_ucs{$_}] = $_;
}

open(OUT, "> ${name0}_ucs.map");
foreach $name (@NAME) {

$code = $CODE{$name};
$map = $MAP{$name};

print "$name\t$map\t$code\n";

open(MAP, "< $map");
while(<MAP>) {
  /^#/ && next;
  s/#.*//;
  if ($name =~ /0208/) {
    (($s, $i, $u) = split(" ")) || next;
    $i = hex($i);
    $u = hex($u);
    if ($u == 0x5C) {
	$u = 0xFF3C;
    }
    $to_ucs[$i] || ($to_ucs[$i] = $u);
    $to_ucs_jis[$i] = 0;
    $from_ucs{$u} = $i;
  } elsif ($name =~ /0212/) {
    (($i, $u) = split(" ")) || next;
    $i = hex($i);
    $u = hex($u);
    if ($u == 0x7E) {
	$u = 0xFF5E;
    }
    $to_ucs2[$i] = $u;
    $to_ucs2_jis[$i] = 0;
    $from_ucs2{$u} = $i;
  } else {
    /^\d/ || next;
    (($p,$i,$e,$s,$u) = split(" ")) || next;
    $i =~ s/j-/0x/;
    $u =~ s/u-/0x/;
    $i = hex($i);
    $u = hex($u);
    if ($u == 0xffff) {
	$u = 0;
    }
    if ($u == 0x7E) {
	$u = 0xFF5E;
    }
    if ($p =~ /^1/) {
      $to_ucs[$i] && next;
      $to_ucs[$i] = $u;
      $to_ucs_jis[$i] = 1;
      $from_ucs3{$u} = $i;
    } elsif ($p =~ /^2/) {
      $to_ucs2[$i] = $u;
      $to_ucs2_jis[$i] = 1;
      $from_ucs4{$u} = $i;
    }
  }
}

}

print OUT <<EOF;
/* JIS X 0208, JIS X 0212, JIS X 0213 (Japanese) */

static wc_uint16 jisx0208x02131_ucs_map[ 0x5E * 0x5E ] = {
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
  printf OUT "\t/* %s 0x%.4X */\n", $to_ucs_jis[$_] ? "JIS X 0213-1" : "JIS X 0208", $_;
}
}

print OUT <<EOF;
};

static wc_uint16 jisx0212x02132_ucs_map[ 0x5E * 0x5E ] = {
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
  printf OUT "\t/* %s 0x%.4X */\n", $to_ucs2_jis[$_] ? "JIS X 0213-2" : "JIS X 0212", $_;
}
}

print OUT <<EOF;
};
EOF

@ucs = sort { $a <=> $b } keys %from_ucs;
$nucs = @ucs + 0;

print OUT <<EOF;

#define N_ucs_jisx0208_map $nucs

static wc_map ucs_jisx0208_map[ N_ucs_jisx0208_map ] = {
EOF
for(@ucs) {
  $_ || next;
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $from_ucs{$_};
}

@ucs = sort { $a <=> $b } keys %from_ucs2;
$nucs = @ucs + 0;

print OUT <<EOF;
};

#define N_ucs_jisx0212_map $nucs

static wc_map ucs_jisx0212_map[ N_ucs_jisx0212_map ] = {
EOF
for(@ucs) {
  $_ || next;
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $from_ucs2{$_};
}

@ucs = sort { $a <=> $b } keys %from_ucs3;
$nucs = @ucs + 0;

print OUT <<EOF;
};

#define N_ucs_jisx02131_map $nucs

static wc_map ucs_jisx02131_map[ N_ucs_jisx02131_map ] = {
EOF
for(@ucs) {
  $_ || next;
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $from_ucs3{$_};
}

@ucs = sort { $a <=> $b } keys %from_ucs4;
$nucs = @ucs + 0;

print OUT <<EOF;
};

#define N_ucs_jisx02132_map $nucs

static wc_map ucs_jisx02132_map[ N_ucs_jisx02132_map ] = {
EOF
for(@ucs) {
  $_ || next;
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $from_ucs4{$_};
}

print OUT <<EOF;
};
EOF

close(MAP);

__END__
jisx0208	EASTASIA/JIS/JIS0208.TXT	JIS X 0208 (Japanese)
jisx0212	EASTASIA/JIS/JIS0212.TXT	JIS X 0212 (Japanese)
jisx0213	jisx0213code.txt		JIS X 0213 (Japanese)

