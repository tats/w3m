
open(MAP, "> ucs_isdigit.map");
print MAP <<EOF;
/*
   File generated from UnicodeData-4.1.0.txt.
*/

EOF

for (<DATA>) {
  chop;
  ($name, $class) = split;
  
  @cp = ();
  
  open(UCD, "< private/UnicodeData-4.1.0.txt");
  while(<UCD>) {
    chop;
    @entry = split(';');
    last if $entry[0] =~ m/.{5,}/;
    if ($entry[2] eq $class) {
      push (@cp, $entry[0]);
    }
  }
  close UCD;

  @bs = ();
  $last = -1;
  $seq = -1;
  for my $e (@cp) {
    if (++$last != hex $e) {
      $seq = $e;
      $last = hex $e;
      push (@bs, $seq);
    }
    $end{$seq} = $e;
  }
  $nobs = @bs;
  
  print MAP <<EOF;

#define N_ucs_${name}_map ${nobs}

static wc_map ucs_${name}_map[ N_ucs_${name}_map ] = {
EOF
  
  for (@bs) {
    print MAP "  { 0x$_, 0x$end{$_} },\n";
  }
  print MAP <<EOF
};
EOF
}

__END__
isdigit Nd
