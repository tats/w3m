BEGIN {
    FS = "[; ]";
    i = 0;
}
$2 == "A" { 
    code = sprintf("0x%s", $1);
    if (strtonum(code) < 0x10000) {
	map[i] = code
	i++;
    }
}
END {
    n = 0;
    start = map[0]
    prev = strtonum(map[0]);
    for (j = 1; j < i; j++) {
	cur = strtonum(map[j]);
	if (match(map[j], "[.]+")) {
	    map2[n] = sprintf("%s, %s", start, map[j - 1]);
	    n++;
	    gsub("[.]+", ", 0x", map[j])
	    map2[n] = map[j];
	    n++;
	    start = map[j + 1];
	    cur = strtonum(start);
	} else {
	    if (cur - prev > 2) {
		map2[n] = sprintf("%s, %s", start, map[j - 1]);
		start = map[j];
		n++;
	    }

	    if (j == i - 1) {
		map2[n] = sprintf("%s, %s", start, map[j]);
		n++;
	    }
	}
	prev = cur;
    }

    printf("static wc_map ucs_ambwidth_map[] = {\n");
    for (j = 0; j < n; j++) {
	printf("   { %s },\n", map2[j]);
    }
    printf("};\n");
    printf("#define N_ucs_ambwidth_map (sizeof(ucs_ambwidth_map) / sizeof(*ucs_ambwidth_map))\n");
}
