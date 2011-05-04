
@NAME = ();
while(<DATA>) {
  chop;
  s/\s*$//;
  (($n, $m, $c) = split(" ", $_, 3)) >= 3 || next;
  push(@NAME, $n);
  $MAP{$n} = $m;
  $CODE{$n} = $c;
}

@to_ucs = ();
%to_ucs2 = ();
%from_ucs = ();
foreach $name (@NAME) {

$code = $CODE{$name};
$map = $MAP{$name};

print "$name\t$map\t$code\n";

open(MAP, "< $map");
while(<MAP>) {
  /^0/ || next;
  (($i, $u) = split(" ")) || next;
  $i = hex($i);
  $u = hex($u);
  if ($map =~ /^V/ && $u > 0x7f) {
    $to_ucs[$i] = $u;
    $from_ucs{$u} = $i;
  }
  if ($map =~ /^c/ && $i > 0x100) {
    $to_ucs2{$i} = $u;
    if (! defined($from_ucs{$u})) {
      $from_ucs{$u} = $i;
    }
  }
}
close(MAP);
}

$name = $NAME[0];
$code = $CODE{$name};
$map = $MAP{$name};
open(OUT, "> ${name}_ucs.map");

# print OUT <<EOF;
# /*
#  These conversion tables between $code and
#  Unicode were made from
#
#    ftp://ftp.unicode.org/Public/MAPPINGS/$map.
# */
print OUT <<EOF;
/* $code */

static wc_uint16 ${name}_ucs_map[ 0x80 ] = {
EOF

foreach $i (0x10 .. 0x1F) {
  print OUT " ";
foreach $j (0 .. 7) {
  $_ = $i * 8 + $j;
  $u = $to_ucs[$_];
  if ($u) {
    printf OUT " 0x%.4X,", $u;
  } else {
    print OUT " 0,     ";
  }
}
  print OUT "\n";
}

@cp = sort { $a <=> $b } keys %to_ucs2;
$cp = @cp + 0;

print OUT <<EOF;
};

#define N_${name}2_ucs_map $cp

static wc_map ${name}2_ucs_map[ N_${name}2_ucs_map ] = {
EOF
for(@cp) {
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $to_ucs2{$_};
}

@ucs = sort { $a <=> $b } keys %from_ucs;
$nucs = @ucs + 0;

print OUT <<EOF;
};

#define N_ucs_${name}_map $nucs

static wc_map ucs_${name}_map[ N_ucs_${name}_map ] = {
EOF
for(@ucs) {
  printf OUT "  { 0x%.4X, 0x%.2X },\n", $_, $from_ucs{$_};
}

print OUT <<EOF;
};
EOF

__END__
cp1258		VENDORS/MICSFT/WINDOWS/CP1258.TXT	CP1258 (Vietnamese)
cp1258_2	cp1258_uni.txt				CP1258 (Vietnamese)
