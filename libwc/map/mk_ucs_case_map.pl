
open(MAP, "> ucs_case.map");
print MAP <<EOF;
/*
   File generated from UnicodeData-4.1.0.txt.
*/

EOF

for (<DATA>) {
  chop;
  ($name, $col) = split;
  
  @cp = ();
  
  open(UCD, "< private/UnicodeData-4.1.0.txt");
  while(<UCD>) {
    chop;
    @entry = split(';');
    last if $entry[0] =~ m/.{5,}/;
    if ($entry[$col] ne '') {
      push (@cp, $entry[0]);
      $map{$entry[0]} = $entry[$col];
    }
  }
  close UCD;

  $nocp = @cp;
  
  print MAP <<EOF;

#define N_ucs_${name}_map ${nocp}

static wc_map ucs_${name}_map[ N_ucs_${name}_map ] = {
EOF
  
  for (@cp) {
    print MAP "  { 0x$_, 0x$map{$_} },\n";
  }
  print MAP <<EOF
};
EOF
}

__END__
toupper	12
tolower	13
totitle	14
