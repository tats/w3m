#! /bin/sh

while :
do
  case $1 in
  -m)
    mode=$2
    shift; shift
    ;;
  -*)
    shift
    ;;
  *)
    break
  esac
done

if [ $# -lt 2 ]; then
  echo "usage: $0 [-m mode] file1 file2"
  exit 1
fi

file=$1
dest=$2

cp $file $dest
if [ -n "$mode" ]; then
  if [ -d $dest ]; then
    chmod $mode $dest/$file
  else
    chmod $mode $dest
  fi
fi
