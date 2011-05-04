
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
  if ($u >= 0xa0) {
    $to_ucs[$i] = $u;
    if ($i < 0x80) {
      print STDERR "$map $i $u\n";
#     $from_ucs{$u} = $i;
    } else {
      $from_ucs{$u} = $i;
    }
  }
}

#  print OUT <<EOF;
# /*
#   These conversion tables between $code and
#   Unicode were made from
# 
#     ftp://ftp.unicode.org/Public/MAPPINGS/$map.
# */
# 
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

close(MAP);
}

__END__
jisx0201k	EASTASIA/JIS/JIS0201.TXT	JIS X 0201 (Japanese Kana)
iso88591	ISO8859/8859-1.TXT	ISO-8859-1 (Latin 1)
iso88592	ISO8859/8859-2.TXT	ISO-8859-2 (Latin 2)
iso88593	ISO8859/8859-3.TXT	ISO-8859-3 (Latin 3)
iso88594	ISO8859/8859-4.TXT	ISO-8859-4 (Latin 4)
iso88595	ISO8859/8859-5.TXT	ISO-8859-5 (Cyrillic)
iso88596	ISO8859/8859-6.TXT	ISO-8859-6 (Arabic)
iso88597	ISO8859/8859-7.TXT	ISO-8859-7 (Greek)
iso88598	ISO8859/8859-8.TXT	ISO-8859-8 (Hebrew)
iso88599	ISO8859/8859-9.TXT	ISO-8859-9 (Latin 5)
iso885910	ISO8859/8859-10.TXT	ISO-8859-10 (Latin 6)
iso885913	ISO8859/8859-13.TXT	ISO-8859-13 (Latin 7)
iso885914	ISO8859/8859-14.TXT	ISO-8859-14 (Latin 8)
iso885915	ISO8859/8859-15.TXT	ISO-8859-15 (Latin 9)
iso885916	ISO8859/8859-16.TXT	ISO-8859-16 (Romanian)

cp856	VENDORS/MISC/CP856.TXT		CP856 (Hebrew)                  
cp1006	VENDORS/MISC/CP1006.TXT		IBM CP1006 (Arabic)
koi8r	VENDORS/MISC/KOI8-R.TXT		KOI8-R (Cyrillic)

nextstep	VENDORS/NEXT/NEXTSTEP.TXT	NeXTSTEP

cp437	VENDORS/MICSFT/PC/CP437.TXT	CP437 (Latin)
cp737	VENDORS/MICSFT/PC/CP737.TXT	CP737 (Greek)
cp775	VENDORS/MICSFT/PC/CP775.TXT	CP775 (Baltic Rim)
cp850	VENDORS/MICSFT/PC/CP850.TXT	CP850 (Latin 1)
cp852	VENDORS/MICSFT/PC/CP852.TXT	CP852 (Latin 2)
cp855	VENDORS/MICSFT/PC/CP855.TXT	CP855 (Cyrillic)
cp857	VENDORS/MICSFT/PC/CP857.TXT	CP857 (Turkish)
cp860	VENDORS/MICSFT/PC/CP860.TXT	CP860 (Portuguese)
cp861	VENDORS/MICSFT/PC/CP861.TXT	CP861 (Icelandic)
cp862	VENDORS/MICSFT/PC/CP862.TXT	CP862 (Hebrew)
cp863	VENDORS/MICSFT/PC/CP863.TXT	CP863 (Canada French)
cp864	VENDORS/MICSFT/PC/CP864.TXT	CP864 (Arabic)
cp865	VENDORS/MICSFT/PC/CP865.TXT	CP865 (Nordic)
cp866	VENDORS/MICSFT/PC/CP866.TXT	CP866 (Cyrillic Russian)
cp869	VENDORS/MICSFT/PC/CP869.TXT	CP869 (Greek 2)
cp874	VENDORS/MICSFT/PC/CP874.TXT	CP874 (Thai)

cp1250	VENDORS/MICSFT/WINDOWS/CP1250.TXT	CP1250 (Latin 2)
cp1251	VENDORS/MICSFT/WINDOWS/CP1251.TXT	CP1251 (Cyrillic)
cp1252	VENDORS/MICSFT/WINDOWS/CP1252.TXT	CP1252 (Latin 1)
cp1253	VENDORS/MICSFT/WINDOWS/CP1253.TXT	CP1253 (Greek)
cp1254	VENDORS/MICSFT/WINDOWS/CP1254.TXT	CP1254 (Turkish)
cp1255	VENDORS/MICSFT/WINDOWS/CP1255.TXT	CP1255 (Hebrew)
cp1256	VENDORS/MICSFT/WINDOWS/CP1256.TXT	CP1256 (Arabic)
cp1257	VENDORS/MICSFT/WINDOWS/CP1257.TXT	CP1257 (Baltic Rim)

