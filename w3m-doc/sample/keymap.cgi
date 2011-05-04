#!/usr/local/bin/perl

$QUERY = $ENV{'QUERY_STRING'};
$KEYMAP = "$ENV{'HOME'}/.w3m/keymap";

if ($QUERY) {
	# &write_keymap($QUERY);
	print <<EOF;
Content-Type: text/html
w3m-control: DELETE_PREVBUF
w3m-control: BACK

EOF
	exit;
}
&init_func();
@key = ();
@func = ();
@data = ();
open(KEYMAP, $KEYMAP);
while (<KEYMAP>) {
	s/^keymap\s+// || next;
	(($k, $_) = &getQWord($_)) || next;
	(($f, $_) = &getWord($_)) || next;
	$FUNC_EXIST{$f} || next;
	($d, $_) = &getQWord($_);
	push(@key, $k);
	push(@func, $f);
	push(@data, $d);
}
close(KEYMAP);

$N = @key;

print <<EOF;
Content-Type: text/html

<head><title>Keymap Setting</title></head>
<h1>Keymap Setting</h1>
<form action="file:///\$LIB/keymap.cgi">
<table>
<tr><td>&nbsp;Key<td>&nbsp;Command<td>&nbsp;Argument
<tr><td><input name=k_$N size=6>
<td><select name=f_$N>
EOF
&print_func();
print <<EOF;
</select>
<td><input name=d_$N>
<td><input type=submit name=ok value=Ok>
<tr><td colspan=4><hr>
EOF
$i = 0;
while(@key) {
	$k = &Q(shift @key);
	$f = shift @func;
	$d = &Q(shift @data);
	print <<EOF;
<tr><td><input type=hidden name=k_$i value=\"$k\">&nbsp;$k
<td><select name=f_$i>
EOF
	&print_func($f);
	print <<EOF;
</select>
<td><input name=d_$i value=\"$d\">
<td><input type=checkbox name=del_$i>Delete
EOF
	$i++;
}
print <<EOF;
</table>
</form>
EOF

sub write_keymap {
	local($query) = @_;
	@key = ();
	@func = ();
	@data = ();

	for $q (split('&', $query)) {
		($_, $d) = split('=', $q);
		if (s/^k_//) {
			$key[$_] = $d;
		} elsif (s/^f_//) {
			$func[$_] = $d;
		} elsif (s/^d_//) {
			$data[$_] = $d;
		} elsif (s/^del_//) {
			$del[$_] = 1;
		}
	}
	open(KEYMAP, "> ${KEYMAP}") || next;
	while(@key) {
		$k = &UQ(shift @key);
		$f = shift @func;
		$d = &UQ(shift @data);
		($f =~ /^\w/) || next;
		(shift @del) && next;
		print KEYMAP "keymap\t$k\t$f";
		if ($d ne '') {
			if ($d =~ /[\"\'\\\s]/) {
				$d =~ s/([\"\\])/\\$1/g;
				print KEYMAP "\t\t\"$d\"";
			} else {
				$d =~ s/([\"\\])/\\$1/g;
				print KEYMAP "\t\t$d";
			}
		}
		print KEYMAP "\n";
	}
	close(KEYMAP);
}

sub UQ {
	local($_) = @_;
	s/\+/ /g;
	s/%([\da-f][\da-f])/pack('c', hex($1))/egi;
	return $_;
}

sub Q {
	local($_) = @_;
	s/\&/\&amp;/g;
	s/\</\&lt;/g;
	s/\>/\&gt;/g;
	s/\"/\&quot;/g;
	return $_;
}

sub getQWord {
	local($_) = @_;
	local($x) = '';
	s/^\s+//;
	while($_ ne '') {
		if (s/^\'(([^\'\\]|\\.)*)\'// ||
		    s/^\"(([^\"\\]|\\.)*)\"// ||
		    s/^([^\'\"\\\s]+)// || s/^\\(.)//) {
			$x .= $1;
		} else {
			last;
		}
	}
	return ($x, $_);
}

sub getWord {
	local($_) = @_;
	s/^\s+//;
	s/^(\S+)// || return ();
	return ($1, $_);
}

sub print_func {
	local($f) = @_;
	for(@FUNC_LIST) {
		if ($f eq $_) {
			print "<option selected>$_\n";
		} else {
			print "<option>$_\n";
		}
	}
}

sub init_func {
	@FUNC_LIST = ();
	%FUNC_EXIST = ();
	while(<DATA>) {
		chop;
		push(@FUNC_LIST, $_);
		$FUNC_EXIST{$_} = 1;
	}
}

__END__
- - - - - - - 
ABORT
ADD_BOOKMARK
BACK
BEGIN
BOOKMARK
CENTER_H
CENTER_V
COOKIE
DELETE_PREVBUF
DICT_WORD
DICT_WORD_AT
DOWN
DOWNLOAD
EDIT
EDIT_SCREEN
END
ESCBMAP
ESCMAP
EXEC_SHELL
EXIT
EXTERN
EXTERN_LINK
FRAME
GOTO
GOTO_LINE
GOTO_LINK
HELP
HISTORY
INFO
INIT_MAILCAP
INTERRUPT
LEFT
LINE_BEGIN
LINE_END
LINE_INFO
LINK_BEGIN
LINK_END
LOAD
MAIN_MENU
MARK
MARK_MID
MARK_URL
MENU
MOUSE
MOUSE_TOGGLE
MOVE_DOWN
MOVE_LEFT
MOVE_RIGHT
MOVE_UP	
NEXT_LINK
NEXT_MARK
NEXT_PAGE
NEXT_WORD
NOTHING
NULL
OPTIONS
PCMAP
PEEK
PEEK_LINK
PIPE_SHELL
PREV_LINK
PREV_MARK
PREV_PAGE
PREV_WORD
PRINT
QUIT
READ_SHELL
REDRAW
REG_MARK
RELOAD
RIGHT
SAVE
SAVE_IMAGE
SAVE_LINK
SAVE_SCREEN
SEARCH
SEARCH_BACK
SEARCH_FORE
SEARCH_NEXT
SEARCH_PREV
SELECT
SHELL
SHIFT_LEFT
SHIFT_RIGHT
SOURCE
SUSPEND
UP
VIEW
VIEW_BOOKMARK
VIEW_IMAGE
WHEREIS	
WRAP_TOGGLE
