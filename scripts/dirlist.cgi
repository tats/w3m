#!/usr/local/bin/perl
#
# Directory list CGI by Hironori Sakamoto (hsaka@mth.biglobe.ne.jp)
#

$CYGWIN = 0;
$RC_DIR = '~/.w3m/';

$RC_DIR =~ s@^~/@$ENV{'HOME'}/@;
$CONFIG = "$RC_DIR/dirlist";
$CGI = $ENV{'SCRIPT_NAME'} || $0;
$CGI = &html_quote(&form_encode("file:$CGI"));

$AFMT = '<a href="%s"><nobr>%s</nobr></a>';
$NOW = time();

@OPT = &init_option($CONFIG);

$query = $ENV{'QUERY_STRING'};
$cmd = '';
$cgi = 0;
if ($query eq '') {
  $_ = `pwd`;
  chop;
  s/\r$//;
  $dir = $_;
  $cgi = 0;
} elsif ($query =~ /^(opt\d+|dir|cmd)=/) {
  foreach(split(/\&/, $query)) {
    if (s/^dir=//) {
      $dir = &form_decode($_);
    } elsif (s/^opt(\d+)=//) {
      $OPT[$1] = $_;
    } elsif (s/^cmd=//) {
      $cmd = $_;
    }
  }
  $cgi = 1;
} else {
  $dir = $query;
  if (($dir !~ m@^/@) &&
      ($CYGWIN && $dir !~ /^[a-z]:/i)) {
    $_ = `pwd`;
    chop;
    s/\r$//;
    $dir = "$_/$dir";
  }
  $cgi = -1;
}
if ($dir !~ m@/$@) {
  $dir .= '/';
}
$ROOT = '';
if ($CYGWIN) {
  if (($dir =~ s@^//[^/]+@@) || ($dir =~ s@^[a-z]:@@i)) {
    $ROOT = $&;
  }
}
if ($cgi) {
  $dir = &cleanup($dir);
}

$TYPE   = $OPT[$OPT_TYPE];
$FORMAT = $OPT[$OPT_FORMAT];
$SORT   = $OPT[$OPT_SORT];
if ($cmd) {
  &update_option($CONFIG);
}

$qdir = &html_quote("$ROOT$dir");
$edir = &html_quote(&form_encode("$ROOT$dir"));
if (! opendir(DIR, "$ROOT$dir")) {
  print <<EOF;
Content-Type: text/html

<html>
<head>
<title>Directory list of $qdir</title>
</head>
<body>
<b>$qdir</b>: $! !
</body>
</html>
EOF
  exit 1;
}

# ($cgi > 0) && print <<EOF;
# w3m-control: DELETE_PREVBUF
# EOF
print <<EOF;
Content-Type: text/html

<html>
<head>
<title>Directory list of $qdir</title>
</head>
<body>
<h1>Directory list of $qdir</h1>
EOF
&print_form($qdir, @OPT);
print <<EOF;
<hr>
EOF
$dir =~ s@/$@@;
@sdirs = split('/', $dir);
$_ = $sdirs[0];
if ($_ eq '') {
  $_ = '/';
}
if ($TYPE eq $TYPE_TREE) {
  print <<EOF;
<table hborder width="640">
<tr valign=top><td width="160">
<pre>
EOF
  $q = &html_quote("$ROOT$_");
  $e = &html_quote(&form_encode("$ROOT$_"));
  if ($dir =~ m@^$@) {
    $n = "\" name=\"current";
  } else {
    $n = '';
  }
  printf("$AFMT\n", "$q$n", "<b>$q</b>");
  $N = 0;
  $SKIPLINE = "";

  &left_dir('', @sdirs);

  print <<EOF;
</pre>
</td><td width="400">
<pre>$SKIPLINE
EOF
} else {
  print <<EOF;
<pre>
EOF
}

&right_dir($dir);

if ($TYPE eq $TYPE_TREE) {
  print <<EOF;
</pre>
</td></tr>
</table>
</body>
</html>
EOF
} else {
  print <<EOF;
</pre>
</body>
</html>
EOF
}

sub left_dir {
  local($pre, $dir, @sdirs) = @_;
  local($ok) = (@sdirs == 0);
  local(@cdirs) = ();
  local($_, $dir0, $d, $qdir, $q, $edir, $e);

  $dir0 = "$dir/";
  $dir = "$ROOT$dir0";
  opendir(DIR, $dir) || return;

  foreach(sort readdir(DIR)) {
    -d "$dir$_" || next;
    /^\.$/ && next;
    /^\.\.$/ && next;
    push(@cdirs, $_);
  }
  closedir(DIR);

  $qdir = &html_quote($dir);
  $edir = &html_quote(&form_encode($dir));
  while(@cdirs) {
    $_ = shift @cdirs;
    $q = &html_quote($_);
    $e = &html_quote(&form_encode($_));
    $N++;
    if (!$ok && $_ eq $sdirs[0]) {
      $d = $dir0 . shift @sdirs;
      if (!@sdirs) {
        $n = "\" name=\"current";
        $SKIPLINE = "\n" x $N;
      } else {
        $n = '';
      }
      printf("${pre}o-$AFMT\n", "$qdir$q$n", "<b>$q</b>");
      &left_dir(@cdirs ? "$pre| " : "$pre  ", $d, @sdirs);
      $ok = 1;
    } else {
      printf("${pre}+-$AFMT\n", "$qdir$q", $q);
    }
  }
}

sub right_dir {
  local($dir) = @_;
  local(@list);
  local($_, $qdir, $q, $edir, $e, $f, $max, @d, $type, $u, $g);
  local($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
        $atime,$mtime,$ctime,$blksize,$blocks);
  local(%sizes, %ctimes, %prints);

  $dir = "$ROOT$dir/";
  opendir(DIR, $dir) || return;

  $qdir = &html_quote($dir);
  $edir = &html_quote(&form_encode($dir));
  if ($TYPE eq $TYPE_TREE) {
    print "<b>$qdir</b>\n";
  }
  @list = ();
  $max = 0;
  foreach(readdir(DIR)) {
    /^\.$/ && next;
#    if ($TYPE eq $TYPE_TREE) {
#      /^\.\.$/ && next;
#    }
    $f = "$dir$_";
    (($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
      $atime,$mtime,$ctime,$blksize,$blocks) = lstat($f)) || next;
    push(@list, $_);
    $sizes{$_} = $size;
    $ctimes{$_} = $ctime;

    if ($FORMAT eq $FORMAT_COLUMN)  {
      if (length($_) > $max) {
        $max = length($_);
      }
      next;
    }
    $type = &utype($mode);
    if ($FORMAT eq $FORMAT_SHORT)  {
      $prints{$_} = sprintf("%-6s ", "[$type]");
      next;
    }
    if ($type =~ /^[CB]/) {
      $size = sprintf("%3u, %3u", ($rdev >> 8) & 0xff, $rdev & 0xffff00ff);
    }
    if ($FORMAT eq $FORMAT_LONG) {
      $u = $USER{$uid} || ($USER{$uid} = getpwuid($uid) || $uid);
      $g = $GROUP{$gid} || ($GROUP{$gid} = getgrgid($gid) || $gid);
      $prints{$_} = sprintf( "%s %-8s %-8s %8s %s ",
		&umode($mode), $u, $g, $size, &utime($ctime));
#   } elsif ($FORMAT eq $FORMAT_STANDARD) {
    } else {
      $prints{$_} = sprintf("%-6s %8s %s ", "[$type]", $size, &utime($ctime));
    }
  }
  closedir(DIR);
  if ($SORT eq $SORT_SIZE) { 
    @list = sort { $sizes{$b} <=> $sizes{$a} || $a <=> $b } @list;
  } elsif ($SORT eq $SORT_TIME) { 
    @list = sort { $ctimes{$b} <=> $ctimes{$a} || $a <=> $b } @list;
  } else {
    @list = sort @list;
  }
  if ($FORMAT eq $FORMAT_COLUMN) {
    local($COLS, $l, $nr, $n);
    if ($TYPE eq $TYPE_TREE) {
      $COLS = 60;
    } else {
      $COLS = 80;
    }
    $l = int($COLS / ($max + 2)) || 1;
    $nr = int($#list / $l + 1);
    $n = 0;
    print "<table>\n<tr valign=top>";
    foreach(@list) {
      $f = "$dir$_";
      $q = &html_quote($_);
      $e = &html_quote(&form_encode($_));
      if ($n % $nr == 0) {
        print "<td>";
      }
      if (-d $f) {
        printf($AFMT, "$qdir$q", "$q/");
      } else {
        printf($AFMT, "$qdir$q", $q);
      }
      $n++;
      if ($n % $nr == 0) {
        print "</td>\n";
      } else {
        print "<br>\n";
      }
    }
    print "</tr></table>\n";
    return;
  }
  foreach(@list) {
    $f = "$dir$_";
    $q = &html_quote($_);
    $e = &html_quote(&form_encode($_));
    print $prints{$_};
    if (-d $f) {
      printf($AFMT, "$qdir$q", "$q/");
    } else {
      printf($AFMT, "$qdir$q", $q);
    }
    if (-l $f) {
      print " -> ", &html_quote(readlink($f));
    }
    print "\n";
  }
}

sub init_option {
  local($config) = @_;
  $OPT_TYPE   = 0;
  $OPT_FORMAT = 1;
  $OPT_SORT   = 2;
  $TYPE_TREE    = 't';
  $TYPE_STARDRD = 'd';
  $FORMAT_SHORT    = 's';
  $FORMAT_STANDRAD = 'd';
  $FORMAT_LONG     = 'l';
  $FORMAT_COLUMN   = 'c';
  $SORT_NAME = 'n';
  $SORT_SIZE = 's';
  $SORT_TIME = 't';
  local(@opt) = ($TYPE_TREE, $FORMAT_STANDRAD, $SORT_NAME);
  local($_);

  open(CONFIG, "< $config") || return @opt;
  while(<CONFIG>) {
    chop;
    s/^\s+//;
    tr/A-Z/a-z/;
    if (/^type\s+(\S)/i) {
      $opt[$OPT_TYPE] = $1;
    } elsif (/^format\s+(\S)/i) {
      $opt[$OPT_FORMAT] = $1
    } elsif (/^sort\s+(\S)/i) {
      $opt[$OPT_SORT] = $1;
    }
  }
  close(CONFIG);
  return @opt;
}

sub update_option {
  local($config) = @_;

  open(CONFIG, "> $config") || return;
  print CONFIG <<EOF;
type $TYPE
format $FORMAT
sort $SORT
EOF
  close(CONFIG); 
}

sub print_form {
  local($d, @OPT) = @_;
  local(@disc) = ('Type', 'Format', 'Sort');
  local(@val) = (
	"('t', 'd')",
	"('s', 'd', 'c')",
	"('n', 's', 't')",
  );
  local(@opt) = (
	"('Tree', 'Standard')",
	"('Short', 'Standard', 'Column')",
	"('By Name', 'By Size', 'By Time')"
  );
  local($_, @vs, @os, $v, $o);

  print <<EOF;
<form action=\"$CGI\">
<center>
<table>
<tr valign=top>
EOF
  foreach(0 .. 2) {
    print "<td align>&nbsp;$disc[$_]</td>\n";
  }
  print "</tr><tr>\n";
  foreach(0 .. 2) {
    print "<td><select name=opt$_>\n";
    eval "\@vs = $val[$_]";
    eval "\@os = $opt[$_]";
    foreach $v (@vs) {
      $o = shift(@os);
      if ($v eq $OPT[$_]) {
        print "<option value=$v selected>$o\n";
      } else {
        print "<option value=$v>$o\n";
      }
    }
    print "</select></td>\n";
  }
  print <<EOF;
<td><input type=submit name=cmd value="Update"></td>
</tr>
</table>
</center>
<input type=hidden name=dir value="$d">
</form>
EOF
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
sub form_encode {
  local($_) = @_;
  s/[\t\r\n%#&=+]/sprintf('%%%2x', unpack('c', $&))/eg;
  s/ /+/g;
  return $_;
}

sub form_decode {
  local($_) = @_;
  s/\+/ /g;
  s/%([\da-f][\da-f])/pack('c', hex($1))/egi;
  return $_;
}

sub cleanup {
  local($_) = @_;

  s@//+@/@g;
  s@/\./@/@g;
  while(m@/\.\./@) {
    s@^/(\.\./)+@/@;
    s@/[^/]+/\.\./@/@;
  }
  return $_;
}

sub utype {
  local($_) = @_;
  local(%T) = (
    0010000, 'PIPE',
    0020000, 'CHR',
    0040000, 'DIR',
    0060000, 'BLK',
    0100000, 'FILE',
    0120000, 'LINK',
    0140000, 'SOCK',
  );
  return $T{($_ & 0170000)} || 'FILE';
}

sub umode {
  local($_) = @_;
  local(%T) = (
    0010000, 'p',
    0020000, 'c',
    0040000, 'd',
    0060000, 'b',
    0100000, '-',
    0120000, 'l',
    0140000, 's',
  );

  return ($T{($_ & 0170000)} || '-')
     . (($_ & 00400) ? 'r' : '-')
     . (($_ & 00200) ? 'w' : '-')
     . (($_ & 04000) ? 's' :
       (($_ & 00100) ? 'x' : '-'))
     . (($_ & 00040) ? 'r' : '-')
     . (($_ & 00020) ? 'w' : '-')
     . (($_ & 02000) ? 's' :
       (($_ & 00010) ? 'x' : '-'))
     . (($_ & 00004) ? 'r' : '-')
     . (($_ & 00002) ? 'w' : '-')
     . (($_ & 01000) ? 't' :
       (($_ & 00001) ? 'x' : '-'));
}

sub utime {
  local($_) = @_;
  local(@MON) = (
    'Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
    'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'
  );
  local($sec,$min,$hour,$mday,$mon,
        $year,$wday,$yday,$isdst) = localtime($_);

  if ($_ > $NOW - 182*24*60*60 && $_ < $NOW + 183*24*60*60) {
    return sprintf("%3s %2d %.2d:%.2d", $MON[$mon], $mday, $hour, $min);
  } else {
    return sprintf("%3s %2d %5d", $MON[$mon], $mday, 1900+$year);
  }
}

