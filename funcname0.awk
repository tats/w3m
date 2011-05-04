BEGIN {
  print "FuncList w3mFuncList[] = {";
  n = 0;
}
/^#/ { next }
{
  print "/*" n "*/ {\"" $1 "\"," $2 "},";
  n++;
} 
END {
  print "{ NULL, NULL }"
  print "};"
}
