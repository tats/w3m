#!/usr/bin/perl
# keymap "x \\" GOTO file:/$LIB/backslash_to_slash.cgi 

$_ = $ENV{W3M_CURRENT_LINK} || exit;
s@\\@/@g;
print <<EOF;
Location: $_

EOF
