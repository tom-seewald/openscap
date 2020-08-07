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

echo "Stderr file = $stderr"
echo "Result file = $result"

line1='^\W*part /var$'

$OSCAP xccdf generate fix --template urn:redhat:anaconda:pre \
	--output $result $srcdir/${name}.xccdf.xml 2>&1 > $stderr
[ -f $stderr ]; [ ! -s $stderr ]; :> $stderr
grep "$line1" $result

case $(uname) in
	FreeBSD)
		[ "`grep -v "$line1" $result | gsed 's/\W//g'`"x == x ]
		;;
	*)
		[ "`grep -v "$line1" $result | sed 's/\W//g'`"x == x ]
		;;
esac

rm $result
