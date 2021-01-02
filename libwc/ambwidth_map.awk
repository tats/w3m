BEGIN {
    FS = "[; ]";
    i = 0;
}
$2 == "A" { 
    code = code2 = strtonum(sprintf("0x%s", $1))
    if (match($1, /[.]+[0-9A-Fa-f]+/)) {
	s = substr($1, RSTART, RLENGTH)
	sub(/[.]+/, "0x", s)
	code2 = strtonum(s)
    }
    for (; code <= code2; code++) {
	if (code >= 0x10000) { break }
	map[i] = sprintf("0x%04X", code)
	i++;
    }
}
END {
    n = 0;
    start = map[0]
    prev = strtonum(map[0]);
    for (j = 1; j < i; j++) {
	cur = strtonum(map[j]);
	if (cur - prev > 1) {
	    map2[n] = sprintf("%s, %s", start, map[j - 1]);
	    n++;
	    start = map[j];
	}
	prev = cur;
    }
    if (i > 0) { map2[n] = sprintf("%s, %s", start, map[i - 1]); n++ }

    printf("static wc_map ucs_ambwidth_map[] = {\n");
    for (j = 0; j < n; j++) {
	printf("   { %s },\n", map2[j]);
    }
    printf("};\n");
    printf("#define N_ucs_ambwidth_map (sizeof(ucs_ambwidth_map) / sizeof(*ucs_ambwidth_map))\n");
}
