
@NAME = ();
while(<DATA>) {
  chop;
  s/\s*$//;
  (($n, $m, $c) = split(" ", $_, 3)) >= 3 || next;
  push(@NAME, $n);
  $MAP{$n} = $m;
  $CODE{$n} = $c;
}

%from_ucs0 = ();
foreach $name (@NAME) {

$code = $CODE{$name};
$map = $MAP{$name};

print "$name\t$map\t$code\n";

%to_ucs = ();
%from_ucs = ();
open(MAP, "< $map");
while(<MAP>) {
  /^#/ && next;
  s/#.*//;
  (($i, $u) = split(" ")) || next;
  $i = hex($i);
  $u = hex($u);
  $from_ucs{$u} = $i;
  if (! $from_ucs0{$u}) {
    $to_ucs{$i} = $u;
  }
}

if ($name eq "gbk") {
  %from_ucs0 = %from_ucs;
  next;
}

$p = 0;
for $ub (0x81 .. 0xFE) {
  for $lb (0x40 .. 0x7E, 0x80 .. 0xFE) {
    $i = ($ub << 8) + $lb;
    if ($u = $to_ucs{$i}) {
      if ($u != $ou + 1) {
	if ($p) {
          $ucs2_end{$su} = $ou;
          $gbk_end{$s} = $og;
	}
	$p = 0;
      }
      if (! $p) {
        $to_ucs2{$i} = $u;
        $from_ucs2{$u} = $i;
        $s = $i;
        $su = $u;
      }
      $p = 1;
      $ou = $u;
    } else {
      if ($p) {
        $ucs2_end{$su} = $ou;
        $gbk_end{$s} = $og;
      }
      $p = 0;
    }
    $og = $i;
  }
}
if ($p) {
  $ucs2_end{$su} = $ou;
  $gbk_end{$s} = 0xFEFE;
}

%from_ucs4 = ();
$i = 0;
$p = 0;
for $u (0x0080 .. 0xD7FF, 0xE000 .. 0xFFFF) {
  if (! $from_ucs{$u}) {
    if (! $p) {
      $from_ucs4{$u} = $i;
      $s = $u;
    }
    $i++;
    $p = 1;
  } else {
    if ($p) {
      $ucs4_end{$s} = $u - 1;
    }
    $p = 0;
  }
  if ($u == 0xD7FF) {
    if ($p) {
      $ucs4_end{$s} = $u - 1;
    }
    $p = 0;
  }
}
if ($p) {
  $ucs4_end{$s} = 0xFFFF;
}

open(OUT, "> ${name}_ucs.map");

# print OUT <<EOF;
# /*
#   These conversion tables between $code and
#   Unicode were made from
# 
#     ftp://ftp.unicode.org/Public/MAPPINGS/$map.
# */
print OUT <<EOF;
/* $code */
EOF

@ucs = sort { $a <=> $b } keys %to_ucs2;
$nucs = @ucs + 0;

print OUT <<EOF;

#define N_gbk_ext_ucs_map $nucs

wc_map3 gbk_ext_ucs_map[ N_gbk_ext_ucs_map ] = {
EOF
for(@ucs) {
  printf OUT "  { 0x%.4X, 0x%.4X, 0x%.4X },\n", $_, $gbk_end{$_}, $to_ucs2{$_};
}

print OUT <<EOF;
};
EOF

@ucs = sort { $a <=> $b } keys %from_ucs2;
$nucs = @ucs + 0;

print OUT <<EOF;

#define N_ucs_gbk_ext_map $nucs

static wc_map3 ucs_gbk_ext_map[ N_ucs_gbk_ext_map ] = {
EOF
for(@ucs) {
  printf OUT "  { 0x%.4X, 0x%.4X, 0x%.4X },\n", $_, $ucs2_end{$_}, $from_ucs2{$_};
}

print OUT <<EOF;
};
EOF

@ucs = sort { $a <=> $b } keys %from_ucs4;
$nucs = @ucs + 0;

print OUT <<EOF;

#define N_ucs_${name}_map $nucs

static wc_map3 ucs_${name}_map[ N_ucs_${name}_map ] = {
EOF
for(@ucs) {
  printf OUT "  { 0x%.4X, 0x%.4X, 0x%.4X },\n", $_, $ucs4_end{$_}, $from_ucs4{$_};
}

print OUT <<EOF;
};
EOF

close(MAP);
}

__END__
gbk		VENDORS/MICSFT/WINDOWS/CP936.TXT		GBK (Chinese)
gb18030		GBK.TXT		GB18030 (Chinese)
