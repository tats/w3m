BEGIN {
  print "#include <stdio.h>"
  print "#include \"funcname1.h\""
  print "%%"
}
/^#/ { next }
{
  print $1 " FUNCNAME_" $2;
} 
