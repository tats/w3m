BEGIN { n=0 }
/^#/ {next}
{
  if (cmd[$2] == "") {
    print "#define " $2 " " n;
    cmd[$2] = n;
  }
  n++;
}

