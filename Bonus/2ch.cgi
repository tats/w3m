#!/usr/bin/perl

$WGET = "wget";
$SCRIPT_NAME = $ENV{'SCRIPT_NAME'} || $0;
$CGI = "file://$SCRIPT_NAME";
$_ = $QUERY_STRING = $ENV{"QUERY_STRING"};
$UserAgent = "Monazilla/1.00 (w3m/2ch.cgi)";

if (/subback.html$/) {
	&subback();
	exit;
}

s@(/\d+)(/([^/]*))?$@$1@ || exit;
$label = $3;
$cgi = "$CGI?$_";
s@^http://([^/]+)/test/read.cgi/([^/]+)/@$1/$2/dat/@ || exit;
$subback = "$CGI?http://$1/$2/subback.html";
$_ .= ".dat";
$dat = "http://$_";
$tmp = $ENV{"HOME"} . "/.w3m2ch/$_";
$dat =~ s/([^\w\/.\:\-])/\\$1/g;
$tmp =~ s/([^\w\/.\:\-])/\\$1/g;
($dir = $tmp) =~ s@/[^/]+$@@;
$cmd = "mkdir -p $dir; $WGET -c -U \"$UserAgent\" -O $tmp $dat >/dev/null 2>&1";
system $cmd;
$lines = (split(" ", `wc $tmp`))[0];
$lines || exit;

@ARGV = ($tmp);
if ($label =~ /^l(\d+)/) {
	$start = $lines - $1 + 1;
	if ($start < 1) {
		$start = 1;
	}
	$end = $lines;
} elsif ($label =~ /^(\d+)-(\d+)/) {
	$start = $1;
	$end = $2;
} elsif ($label =~ /^(\d+)-/) {
	$start = $1;
	$end = $start + 100 - 1;
} elsif ($label =~ /^(\d+)/) {
	$start = $1;
	$end = $1;
} else {
	$start = 1;
	$end = $lines;
}
$head = "<a href=\"$subback\">■掲示板に戻る■</a>\n";
$head .= "<a href=\"$cgi/\">全部</a>\n";
for (0 .. ($lines - 1) / 100) {
	$n = $_ * 100 + 1;
	$head .= "<a href=\"$cgi/$n-\">$n-</a>\n";
}
$head .= "<a href=\"$cgi/l50\">最新50</a>\n";
print <<EOF;
Content-Type: text/html

EOF
$i = 1;
while (<>) {
	s/\r?\n$//;
	($name, $mail, $date, $_, $title) = split(/\<\>/);
	if ($i == 1) {
		if (!$title) {
			print <<EOF;
このスレッドは過去ログ倉庫に格納されています。
<p>
<a href="$QUERY_STRING">$QUERY_STRING</a>
EOF
			unlink($tmp);
			exit
		}
		print <<EOF;
<title>$title</title>
$head
<p>$title</p>
<dl>
EOF
	}
	if ($mail) {
		$name = "<a href=\"mailto:$mail\">$name</a>";
	}
	s@http://ime.nu/@http://@g;
	s@(h?ttp:)([#-~]+)@"<a href=\"" . &link("http:$2") .  "\">$1$2</a>"@ge;
	s@(ftp:[#-~]+)@<a href="$1">$1</a>@g;
	s@<a href="../test/read.cgi/\w+/\d+/@<a href="$cgi/@g;
	if ($i == 1 || ($i >= $start && $i <= $end)) {
		print <<EOF;
<dt><a name="$i">$i</a> ：$name：$date
<dd>
$_
<p>
EOF
	}
	$i++;
}
print <<EOF;
</dl>
<hr>
EOF

sub link {
	local($_) = @_;
	if (m@/test/read.cgi/@) {
		return "$CGI?$_";
	}
	return $_;
}

sub subback {
	$dat = $_;
	s@http://@@ || exit;
	$tmp = $ENV{"HOME"} . "/.w3m2ch/$_";
	$dat =~ s/([^\w\/.\:\-])/\\$1/g;
	$tmp =~ s/([^\w\/.\:\-])/\\$1/g;
	($dir = $tmp) =~ s@/[^/]+$@@;
	$cmd = "mkdir -p $dir; $WGET -O $tmp $dat >/dev/null 2>&1";
	system $cmd;
print <<EOF;
Content-Type: text/html

EOF
	@ARGV = ($tmp);
	while (<>) {
		if (/<base href="([^"]+)"/) {
			$base = $1;
		} elsif ($base) {
			s@^<a href="@<a href="$CGI?$base@;
		}
		print;
	}
	unlink($tmp);
}
