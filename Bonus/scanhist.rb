#!/usr/local/bin/ruby

# scan history

def usage
  STDERR.print "usage: scanhist -h HISTORY ML-archive1 ML-archive2 ...\n"
  exit 1
end

def html_quote(s)
  s.gsub!(/&/,"&amp;")
  s.gsub!(/</,"&lt;")
  s.gsub!(/>/,"&gt;")
  s
end

if ARGV.size == 0 then
  usage
end

histfile = nil

while ARGV[0] =~ /^-/
  case ARGV.shift
  when "-h"
    histfile = ARGV.shift
  else
    usage
  end
end

if histfile.nil? then
  usage
end

patched = {}
histline = {}
f = open(histfile)
while f.gets
  if /Subject: (\[w3m-dev.*\])/ then
    patched[$1] = true
    histline[$1] = $.
  end
end
f.close

archive = {}
subject = nil
for fn in ARGV
  f = open(fn)
  while f.gets
    if /^From / then
       # beginning of a mail
       subject = nil
    elsif subject.nil? and /^Subject: / then
       $_ =~ /Subject: (\[w3m-dev.*\])/
       subject = $1
       archive[subject] = [$_.chop.sub(/^Subject:\s*/,""),false,fn+"#"+($.).to_s]
    elsif /^\+\+\+/ or /\*\*\*/ or  /filename=.*(patch|diff).*/ or /^begin \d\d\d/
       archive[subject][1] = true
    end
  end
  f.close
end

print "<html><head><title>w3m patch configuration\n</title></head><body>\n"
print "<pre>\n"
for sub in archive.keys.sort
  a = archive[sub]
  if a[1] then
    if patched[sub] then
      print "[<a href=\"#{histfile}\##{histline[sub]}\">+</a>]"
    else
      print "[-]"
    end
    print "<a href=\"#{a[2]}\">"
    print "<b>",html_quote(a[0]),"</b></a>\n"
  else
    if patched[sub] then
      print "[<a href=\"#{histfile}\##{histline[sub]}\">o</a>]"
    else
      print "   "
    end
    print "<a href=\"#{a[2]}\">"
    print "<b>",html_quote(a[0]),"</b></a>\n"
  end
end
print "</pre></body></html>\n"
