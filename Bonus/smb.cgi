#!/usr/bin/perl

# Workgroup list: file:/$LIB/smb.cgi
# Server list:    file:/$LIB/smb.cgi?workgroup
# Sahre list:     file:/$LIB/smb.cgi?//server
#                 file:/$LIB/smb.cgi/server
# Directory:      file:/$LIB/smb.cgi?//server/share
#                 file:/$LIB/smb.cgi?//server/share/dir...
#                 file:/$LIB/smb.cgi/server/share
# Get file:       file:/$LIB/smb.cgi?//server/share/dir.../file
#                 file:/$LIB/smb.cgi/server/share/dir.../file
#
# ----- ~/.w3m/smb -----
# workgroup = <workgroup>
# [ username = <username> ]
# [ password = <password> ]
# [ password_file = <password_file> ]
# ----------------------
# --- <password_file> ---
# <password>
# -----------------------
# default:
#  <username> = $USER
#  <password> = $PASSWD  (Don't use!)
#  <password_file> = $PASSWD_FILE

$DEBUG = 1;

$MIME_TYPE = "~/.mime.types";
$AUTH_FILE = "~/.w3m/smb";
$MIME_TYPE =~ s@^~/@$ENV{"HOME"}/@;
$AUTH_FILE =~ s@^~/@$ENV{"HOME"}/@;
$WORKGROUP = "-";
$USER = $ENV{"USER"};
$PASSWD = $ENV{"PASSWD"};
$PASSWD_FILE = $ENV{"PASSWD_FILE"};
&load_auth_file($AUTH_FILE);

$NMBLOOKUP = "nmblookup";
$SMBCLIENT = "smbclient";
@NMBLOOKUP_OPT = ("-T");
@SMBCLIENT_OPT = ("-N");
$USE_OPT_A = defined($PASSWD) && (-f $AUTH_FILE) && &check_opt_a();
if ($USE_OPT_A) {
	push(@SMBCLIENT_OPT, "-A", $AUTH_FILE);
} elsif (-f $PASSWD_FILE) {
	$USE_PASSWD_FILE = 1;
} elsif (defined($PASSWD)) {
	$USE_PASSWD_FD = 1;
	$PASSWD_FD = 0;
}
if (defined($PASSWD)) {
	$passwd = "*" x 8;
}
$DEBUG && print <<EOF;
DEBUG: NMBLOOKUP=$NMBLOOKUP @NMBLOOKUP_OPT
DEBUG: SMBCLIENT=$SMBCLIENT @SMBCLIENT_OPT
DEBUG: WORKGROUP=$WORKGROUP
DEBUG: USER=$USER
DEBUG: PASSWD=$passwd
DEBUG: PASSWD_FILE=$PASSWD_FILE
DEBUG: PASSWD_FD=$PASSWD_FD
EOF

$PAGER = "cat";
$FILE = "F000";

$CGI = "file://" . &file_encode($ENV{"SCRIPT_NAME"} || $0);
$QUERY = $ENV{"QUERY_STRING"};
$PATH_INFO = $ENV{"PATH_INFO"};

if ($PATH_INFO =~ m@^/@) {
	$_ = $PATH_INFO;
	if (! m@^//@) {
		$_ = "/$_";
	}
	s@[\r\n\0\\"]@@g;
	$DEBUG && print "DEBUG: PATH_INFO=\"$_\"\n";
	$Q = "";
}
else {
	$_ = &file_decode($QUERY);
	$DEBUG && print "DEBUG: QUERY_STRING=\"$_\"\n";
	$Q = "?";
}
if (s@^//([^/]+)@@) {
	$server = $1;
#	if (!$USE_OPT_A && !defined($PASSWD)) {
#		&print_form("//$server$_");
#		exit;
#	}
	if (s@^/([^/]+)@@) {
		&file_list("//$server/$1", &cleanup($_));
	} else {
		&share_list($server);
	}
} elsif (m@^[^/]@) {
	&server_list($_);
} else {
	&group_list();
}

sub file_list {
	local($service, $file) = @_;
	local(@files) = ();
	local($dir, $qservice, $qfile); 
	local($_, $c);

$DEBUG && print "DEBUG: service=\"$service\" file=\"$file\"\n";
	if ($file eq "/") {
		goto get_list;
	}
	$_ = $file;
	s@/@\\@g;
	@cmd = ($SMBCLIENT, $service, @SMBCLIENT_OPT, "-c", "ls \"$_\"");
	$F = &open_pipe(1, @cmd);
	while (<$F>) {
$DEBUG && print "DEBUG: $_";
		/^\s/ && last;
	}
	close($F);
	if (s/\s+([A-Z]*) {1,8}\d+  (\w{3} ){2}[ \d]\d \d\d:\d\d:\d\d \d{4}\s*$//
		&& $1 !~ /D/) {
		&get_file($service, $file);
		exit;
	}

    get_list:
	$_ = "$file/*";
	s@/+@\\@g;
	@cmd = ($SMBCLIENT, $service, @SMBCLIENT_OPT, "-c", "ls \"$_\"");
	$F = &open_pipe(1, @cmd);
	while (<$F>) {
		/^\s*$/ && last;
$DEBUG && print "DEBUG: $_";
		/^cd\s+/ && last;
		/^\S/ && next;
		s/\r?\n//;
		push(@files, $_);
	}
	close($F);

	$qservice = &html_quote($service);
	$service = &file_encode($service);
	$qfile = &html_quote($file);
	$file = &file_encode($file);

	print "Content-Type: text/html\n\n";
	print "<title>$qservice$qfile</title>\n";
	print "<b>$qservice$qfile</b>\n";
	print "<pre>\n";
	for (sort @files) {
		s/\s+([A-Z]*) {1,8}\d+  (\w{3} ){2}[ \d]\d \d\d:\d\d:\d\d \d{4}\s*$// || next;
		$c = $&;
		s/^  //;
		$_ eq "." && next;
		print "<a href=\"$CGI$Q$service"
			. &cleanup("$file/" . &file_encode($_)) . "\">"
			. &html_quote($_) . "</a>"
			. &html_quote($c) . "\n";
	}
	print "</pre>\n";
}

sub get_file {
	local($service, $file) = @_;
	local($encoding, $type);
	local($_, @cmd);

	$_ = $file;
	s@/@\\@g;
	@cmd = ($SMBCLIENT, $service, @SMBCLIENT_OPT, "-E", "-c", "more \"$_\"");
$DEBUG && print "DEBUG: @cmd\n";

	($encoding, $type) = &guess_type($file);
	$file =~ s@^.*/@@;
	$| = 1;
	print "Content-Encoding: $encoding\n" if $encoding;
	print "Content-Type: $type; name=\"$file\"\n\n";

	$ENV{"PAGER"} = $PAGER if $PAGER;
	&exec_cmd(1, @cmd);
}

sub share_list {
	local($server) = @_;
	local(@share);
	local($qserver, $_, $d, @c);

	@share = &get_list(1, $server, "Share");

	$qserver = &html_quote($server);
	$server = &file_encode($server);

	print "Content-Type: text/html\n\n";
	print "<title>Share list: $qserver</title>\n";
	print "<table>\n";
	print "<tr><td colspan=3><b>$qserver</b>";
	for (sort @share) {
		($_, $d, @c) = split(" ");
		if ($d eq 'Disk') {
			print "<tr><td>+ <a href=\"$CGI$Q//$server/"
				. &file_encode($_) . "\">"
				. &html_quote($_) . "</a>";
		} else {
			print "<tr><td>+ "
				. &html_quote($_);
		}
		print "<td><td>"
			. &html_quote($d) . "<td><td>"
			. &html_quote("@c") . "\n";
	}
	print "</table>\n";
}

sub server_list {
	local($group) = @_;
	local($master, @server);
	local($_, @c);

	$master = &get_master($group);
	@server = &get_list(0, $master, "Server");

	$group = &html_quote($group);

	print "Content-Type: text/html\n\n";
	print "<title>Server list: $group</title>\n";
	print "<table>\n";
	print "<tr><td colspan=3><b>$group</b>\n";
	for (sort @server) {
		($_, @c) = split(" ");
		print "<tr><td>+ <a href=\"$CGI$Q//"
			. &file_encode($_) . "\">"
			. &html_quote($_) . "</a><td><td>"
			. &html_quote("@c") . "\n";
	}
	print "</table>\n";
}

sub group_list {
	local($master, @group);
	local($_, @c);

	$master = &get_master($WORKGROUP || "-");
	@group = &get_list(0, $master, "Workgroup");

	print "Content-Type: text/html\n\n";
	print "<title>Workgroup list</title>\n";
	print "<table>\n";
	for (sort @group) {
		($_, @c) = split(" ");
		print "<tr><td><a href=\"$CGI?"
			. &file_encode($_) . "\">"
			. &html_quote($_) . "</a><td><td>"
			. &html_quote("@c") . "\n";
	}
	print "</table>\n";
}

sub check_opt_a {
	local($_, $F, @cmd);

	@cmd = ($SMBCLIENT, "-h");
	$F = &open_pipe(0, @cmd);
	while (<$F>) {
		if (/^\s*-A\s/) {
$DEBUG && print "DEBUG: $_";
			close($F);
			return 1;
		}
	}
	close($F);
	return 0;
}

sub get_master {
	local($group) = @_;
	local($_, $F, @cmd);

	@cmd = ($NMBLOOKUP, "-M", @NMBLOOKUP_OPT, $group);
	$F = &open_pipe(0, @cmd);
	$_ = <$F>;
	$_ = <$F>;
	close($F);
	($_) = split(/[,\s]/);
	s/\.*$//;
	return $_;
}

sub get_list {
	local($passwd, $server, $header) = @_;
	local(@list) = ();
	local($_, @cmd, $F);

	@cmd = ($SMBCLIENT, @SMBCLIENT_OPT, "-L", $server);
	$F = &open_pipe($passwd, @cmd);
	while (<$F>) {
		if (/^\s*$header/) {
$DEBUG && print "DEBUG: $_";
			last;
		}
	}
	while (<$F>) {
		/^\s*$/ && last;
$DEBUG && print "DEBUG: $_";
		/^\S/ && last;
		/^\s*-/ && next;
		push(@list, $_);
	}
	close($F);
	return @list;
}

sub open_pipe {
	local($passwd, @cmd) = @_;
	local($F) = $FILE++;

$DEBUG && print "DEBUG: @cmd\n";
	open($F, "-|") || &exec_cmd($passwd, @cmd);
	return $F;
}

sub exec_cmd {
	local($passwd, @cmd) = @_;

	$ENV{"LC_ALL"} = "C";
	$ENV{"USER"} = $USER;
	if ($passwd && !$USE_OPT_A) {
		if ($USE_PASSWD_FILE) {
			$ENV{"PASSWD_FILE"} = $PASSWD_FILE;
		} elsif ($USE_PASSWD_FD) {
			$ENV{"PASSWD_FD"} = $PASSWD_FD;
			if (open(W, "|-")) {
				print W $PASSWD;
				close(W);
				exit;
			}
		}
	}
	open(STDERR, ">/dev/null");
	exec @cmd;
	exit 1;
}

sub print_form {
	local($_) = @_;
	local($q) = &html_quote($_);
	$_ = &file_encode($_);

	print <<EOF;
Content-Type: text/html

<h1>$q</h1>
<form action="$CGI$Q$_" method=POST>
<table>
<tr><td>Workgroup	<td>User	<td>Password
<tr><td><input type=text size=8 name=group value="$WORKGROUP">
    <td><input type=text size=8 name=user value="$USER">
    <td><input type=password size=8 name=passwd value="$PASSWD">
    <td><input type=submit name=OK value=OK>
</table>
</form>
EOF
}

sub load_auth_file {
	local($_) = @_;

	if ($USER =~ s/%(.*)$//) {
		$PASSWD = $1 unless $PASSWD;
	}
	open(F, $_) || return;
	while (<F>) {
		s/\s+$//;
		if (s/^workgroup\s*=\s*//i) {
			$WORKGROUP = $_;
		} elsif (s/^user(name)?\s*=\s*//i) {
			$USER = $_;
		} elsif (s/^passw(or)?d\s*=\s*//i) {
			$PASSWD = $_;
		} elsif (s/^passw(or)?d_file\s*=\s*//i) {
			$PASSWD_FILE = $_;
		}
	}
	close(F);
}

sub load_mime_type {
	local($_) = @_;
	local(%mime) = ();
	local($type, @suffix);

	open(F, $_) || return ();
	while(<F>) {
		/^#/ && next;
		chop;
		(($type, @suffix) = split(" ")) >= 2 || next;
		for (@suffix) {
			$mime{$_} = $type;
		}
	}
	close(F);
	return %mime;
}

sub guess_type {
	local($_) = @_;
	local(%mime) = &load_mime_type($MIME_TYPE);
	local($encoding) = undef;

	if (s/\.gz$//i) {
		$encoding = "gzip";
	} elsif (s/\.Z$//i) {
		$encoding = "compress";
	} elsif (s/\.bz2?$//i) {
		$encoding = "bzip2";
	}
	/\.(\w+)$/;
	$_ = $1;
	tr/A-Z/a-z/;
	return ($encoding, $mime{$_} || "text/plain");
}

sub cleanup {
	local($_) = @_;

	$_ .= "/";
	s@//+@/@g;
	s@/\./@/@g;
	while(m@/\.\./@) {
		s@^/(\.\./)+@/@;
		s@/[^/]+/\.\./@/@;
	}
	s@(.)/$@$1@;
	return $_;
}

sub file_encode {
	local($_) = @_;
	s/[\000-\040\+:#?&%<>"\177-\377]/sprintf('%%%02X', unpack('C', $&))/eg;
	return $_;
}

sub file_decode {
	local($_) = @_;
	s/\+/ /g;
	s/%([\da-f][\da-f])/pack('C', hex($1))/egi;
	s@[\r\n\0\\"]@@g;
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
