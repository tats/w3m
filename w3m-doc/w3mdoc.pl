#!/usr/local/bin/perl

package w3mdoc;

sub CHECK {
  my($a, @b) = @_;
  for(@b) {
    defined($a->{$_}) || die("\"$a->{id}.$_\" is not defined.\n");
  }
}

sub DEF {
  my($a, $b, $c) = @_;

  if (! defined($data->{$a})) {
     $data->{$a} = bless { id => $a };
  } 
  $data->{$a}{$b} = $c;
}

sub SUB {
  local($_) = @_;
  my($a, $b);

  if (/^\@(\w+)\.(\w+)\@$/) {
    ($a, $b) = ($1, $2);
    defined($data->{$a}) || die("\"$a.$b\" is not defined.\n");
    $data->{$a}->CHECK($b);
    return $data->{$a}{$b};
  }
  if (/^\@(\w+)\((\w+)\)\@$/) {
    ($a, $b) = ($1, $2);
    defined(&{$a}) || die("\"$a()\" is not defined.\n");
    defined($data->{$b}) || die("\"$a($b)\" is not defined.\n");
    return $data->{$b}->$a();
  }
  return '@';
}

package main;

@ARGV || unshift(@ARGV, "-");
while(@ARGV) {
  $file = shift @ARGV;
  &make_doc($file);
}

sub make_doc {
  my($file) = @_;
  my($in_def, $in_code, $code, $a, $b);
  local(*F);
  local($_);

  open(F, $file) || die("$file: $!\n");
  $in_def = 0;
  $in_code = 0;
  while(<F>) {
    if ($in_def) {
      if (/^\@end/) {
        $in_def = 0;
        next;
      }
      s/^\s+//;
      s/^(\w+)\.(\w+)// || next;
      ($a, $b) = ($1, $2);
      s/^\s+//;
      s/\s+$//;
      &w3mdoc::DEF($a, $b, $_);
      next;
    }
    if ($in_code) {
      if (/^\@end/) {
        eval "package w3mdoc; $code";
        $in_code = 0;
        next;
      }
      $code .= $_;
      next;
    }
    if (/^\@define/) {
      $in_def = 1;
      next;
    }
    if (/^\@code/) {
      $in_code = 1;
      $code = "";
      next;
    }
    if (s/^\@include\s+//) {
      s/\s+$//;
      &make_doc($_);
      next;
    }
    if (/^\@/) {
      die("unknown command: $_");
    }
    s/(\\@|\@(\w+(\.\w+|\(\w+\)))\@)/&w3mdoc::SUB($1)/eg;
    print;
  }
  close(F);
}

