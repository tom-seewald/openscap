#!/usr/bin/env bash
. $builddir/tests/test_common.sh

set -e
set -o pipefail

name=$(basename $0 .sh)

case $(uname) in
	FreeBSD)
		result=$(mktemp /tmp/${name}.out.XXXXXX)
		stderr=$(mktemp /tmp/${name}.out.XXXXXX)
		;;
	*)
		result=$(mktemp -t ${name}.out.XXXXXX)
		stderr=$(mktemp -t ${name}.out.XXXXXX)
		;;
esac

	# Workaround trac#245 for distcheck
	oval=${name}.oval.xml.result.xml
	if [ ! -f "$oval" ]; then
		ln -s $srcdir/$oval $oval
	fi
	# Workaround end

$OSCAP xccdf generate report --output $result $srcdir/${name}.xccdf.xml.result.xml 2> $stderr

	# Workaround
	if [ -L "$oval" ]; then
		rm $oval
	fi
	unset oval
	# Workaround end

echo "Stderr file = $stderr"
echo "Result file = $result"
[ -f $stderr ]; [ ! -s $stderr ]; rm $stderr

grep 'Testing file permissions of /etc/shadow' $result
grep '<code>---------' $result
echo $result
# TODO: Check for the current version, as given by autotools, instead of any version
case $(uname) in
	FreeBSD)
		grep -E '<a href="http://open-scap.org">OpenSCAP</a>.*[0-9]+\.[0-9]+\.[0-9]+' $result
		;;
	*)
		grep -P '<a href="http://open-scap.org">OpenSCAP</a> (([0-9]+)\.){2}[0-9]+' $result
		;;
esac
#rm $result
