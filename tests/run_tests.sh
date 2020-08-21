total=0
pass=0
fail=0
w3m="../w3m -o ignore_null_img_alt=false"
for i in *.html; do
	if $w3m -I utf-8 -O utf-8 -T text/html < "$i" | diff - "`basename "$i" .html`.expected"; then
		pass="`expr 1 + "$pass"`"
	else
		fail="`expr 1 + "$fail"`"
	fi
	total="`expr 1 + "$total"`"
done
echo "TOTAL: $total test(s)"
echo "PASS : $pass"
echo "FAIL : $fail"
test 0 -eq "$fail"
