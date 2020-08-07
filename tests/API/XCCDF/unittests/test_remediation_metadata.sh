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

ret=0

input_xml="$srcdir/${name}.xccdf.xml"

echo "Stderr file = $stderr"
echo "Result file = $result"
[ -f $stderr ]; [ ! -s $stderr ]; :> $stderr

$OSCAP xccdf generate fix --fix-type bash --profile 'common' --output "$result" "$input_xml" 2> $stderr

case $(uname) in
	FreeBSD)
		grep -q "^#.*Profile title on one line" "$result"
		grep -q "^#.*Profile description" "$result"
		grep -q "^#.*that spans two lines" "$result"
		;;
	*)
		grep -q "^\s*#.*Profile title on one line" "$result"
		grep -q "^\s*#\s*Profile description" "$result"
		grep -q "^\s*#\s*that spans two lines" "$result"
		;;
esac

rm "$result"

$OSCAP xccdf generate fix --fix-type ansible --profile 'second' --output "$result" "$input_xml" 2> $stderr


case $(uname) in
	FreeBSD)
		grep -q "^#.*Second profile title on one line" "$result"
		grep -q "^#.*Profile description" "$result"
		grep -q "^#.*that spans two lines" "$result"
		;;
	*)
		grep -q "^\s*#.*Second profile title on one line" "$result"
		grep -q "^\s*#\s*Profile description" "$result"
		grep -q "^\s*#\s*that spans two lines" "$result"
		;;
esac

rm "$result"
