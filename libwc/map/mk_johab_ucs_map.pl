
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
  ($u >= 0xAC00 && $u <= 0xD7A3) && next;
  ($i >= 0xD800) && next;
  ($u < 0x80) && next;
  $from_ucs{$u} = $i;
  if ($i >= 0x80) {
    $to_ucs{$i} = $u;
  }
}

# print OUT <<EOF;
# /*
#   These conversion tables between $code and
#   Unicode were made from
# 
#     ftp://ftp.unicode.org/Public/MAPPINGS/$map.
# */

@ucs = sort { $a <=> $b } keys %to_ucs;
$nucs = @ucs + 0;

print OUT <<EOF;
/* $code */

#define N_${name}2_ucs_map $nucs

static wc_map ${name}2_ucs_map[ N_${name}2_ucs_map ] = {
EOF
for(@ucs) {
  printf OUT "  { 0x%.4X, 0x%.4X },\n", $_, $to_ucs{$_};
}

@ucs = sort { $a <=> $b } keys %from_ucs;
$nucs = @ucs + 0;

print OUT <<EOF;
};

#define N_ucs_${name}2_map $nucs

static wc_map ucs_${name}2_map[ N_ucs_${name}2_map ] = {
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
johab		EASTASIA/KSC/JOHAB.TXT		Johab (Korean)
