#!/usr/local/bin/perl

$SCRIPT_NAME = $ENV{'SCRIPT_NAME'} || $0;
$CGI = "file://$SCRIPT_NAME?";

if ($ENV{'QUERY_STRING'}) {
  $file = $ENV{'QUERY_STRING'};
} else {
  $file = $ARGV[0];
}
$file = &cleanup($file);

if (-d $file) {
  print <<EOF;
Location: file:$file
EOF
  exit;
}
if (! open(FILE, "< $file")) {
  $file = &html_quote($file);
  $_ = "$file: " . &html_quote($!);
  print <<EOF;
Content-Type: text/html

<head><title>$file</title></head>
<b>$_</b>
EOF
  exit 1;
}

$file = &html_quote($file);
($dir = $file) =~ s@[^/]*$@@;

print <<EOF;
Content-Type: text/html

<head><title>$file</title></head>
<pre>
EOF
while (<FILE>) {
  $_ = &html_quote($_);

  s/^(\#\s*include\s+)(\&quot;.*\&quot;|\&lt\;.*\&gt\;)/$1 . &header_ref($2)/ge;

  print;
}
close(FILE);
print "</pre>\n";

sub header_ref {
  local($_) = @_;
  local($d);

  if (s/^\&quot;//) {
    s/\&quot;$//;
    return "&quot;<a href=\"$CGI$dir$_\">$_</a>&quot;";
  }
  s/^\&lt\;//;
  s/\&gt\;$//;

  for $d (
	"/usr/include",
	"/usr/local/include",
	"/usr/X11R6/include",
	"/usr/X11/include",
	"/usr/X/include",
	"/usr/include/X11"
  ) {
    -f "$d/$_" && return "&lt;<a href=\"$CGI$d/$_\">$_</a>&gt;";
  }
  return $_;
}


sub html_quote {
  local($_) = @_;
  local(%QUOTE) = (
    '<', '&lt;',
    '>', '&gt;',
    '&', '&amp;',
    '"', '&quot;',
  );
  s/[<>&"]/$QUOTE{$&}/g;
  return $_;
}

sub cleanup {
  local($_) = @_;

  s@//+@/@g;
  s@/\./@/@g;
  while(m@/\.\./@) {
    s@^/(\.\./)+@/@;
    s@/[^/]+/\.\./@/@;
  }
  return $_;
}
