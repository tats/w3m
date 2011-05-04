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

s@/(\d+)(/([^/]*))?$@/$1@ || exit;
my $datnum = $1;
$label = $3;
$cgi = "$CGI?$_";

s@^http://([^/]+)/test/read.cgi/([^/]+)/@$1/$2/dat/@ || exit;
$subback = "$CGI?http://$1/$2/subback.html";
$bbs = $2;
if ($ENV{REQUEST_METHOD} eq "POST") {
	&post();
	exit;
}

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
<form method=POST action="$cgi"><input type=submit value="書き込む" name=submit> 名前： <input name=FROM size=19> E-mail<font size=1> (省略可) </font>: <input name=mail size=19><br><textarea rows=5 cols=70 wrap=off name=MESSAGE></textarea><input type=hidden name=bbs value=$bbs><input type=hidden name=key value=$datnum><input type=hidden name=time value=@{[time]}></form></body></html>
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

sub post {
	my $debug = 0;

	$| = 1;
	use IO::Socket;
	my @POST = <>;
	$QUERY_STRING =~ m@^http://([^/]+)@;
	my $host = $1;
	my $sock = IO::Socket::INET->new("$host:80") or die;
	# retrieve posting cookie; this may not work
	print "Content-Type: text/html\n\n";
	print $sock
	    "HEAD /test/bbs.cgi HTTP/1.1\n",
	    "Host: $host\n",
	    "Connection: keep-alive\n",
	    "\n";
	my $posting_cookie = undef;
	while (<$sock>) {
		print if ($debug);
		s/[\n\r]+$//;
		last if (/^$/);
		if (/^set-cookie:.*(PON=[^;]+)/i) {
			$posting_cookie = $1;
		}
	}
	#$sock = IO::Socket::INET->new("$host:80") or die;
	my $submit =
	    "POST /test/bbs.cgi HTTP/1.1\n" .
	    "Host: $host\n" .
	    "Accept-Language: ja\n" .
	    "User-Agent: $UserAgent\n" .
	    "Referer: $QUERY_STRING\n" .
	    "Cookie: $posting_cookie; NAME=nobody; MAIL=sage\n" .
	    "Content-Length: " . length(join("", @POST)) . "\n" .
	    "\n@POST";
	print $sock $submit or die;
	print "\n-- POSTed contents --\n${submit}\n-- POSTed contents --\n"
	    if ($debug);
	my $chunked = 0;
	while (<$sock>) {
		s/[\n\r]*$//;
		last if (/^$/);
		$chunked = 1 if (/^transfer-encoding:\s*chunked/i);
	}
	my $post_response = "";
	while (<$sock>) {
		if ($chunked) {
			s/[ \r\n]*$//;
			my $len = hex($_);
			$len > 0 or last;
			read($sock, $_, $len);
			<$sock>;	#skip empty line at the end of chunk.
		}
		$post_response .= $_;
	}
	$post_response =~ s/<META content=(\d+);URL=(\S+) http-equiv=refresh>/<META content=$1;URL=$cgi http-equiv=refresh>/im;
	print $post_response;
	exit;
}
