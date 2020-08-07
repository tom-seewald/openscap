#!/usr/bin/env bash
. $builddir/tests/test_common.sh

set -e
set -o pipefail

name=$(basename $0 _ds.sh)
xccdf=${name}.xccdf.xml

case $(uname) in
	FreeBSD)
		sds=$(mktemp /tmp/${name}.sds.XXXXXX)
		stderr=$(mktemp /tmp/${name}_ds.err.XXXXXX)
		result=$(mktemp /tmp/${name}_ds.out.XXXXXX)
		;;
	*)
		sds=$(mktemp -t ${name}.sds.XXXXXX)
		stderr=$(mktemp -t ${name}_ds.err.XXXXXX)
		result=$(mktemp -t ${name}_ds.out.XXXXXX)
		;;
esac

echo "sds file: $sds"
echo "Stderr file = $stderr"
echo "Results file = $result"

line1='^\W*part /tmp$'
line2='^\W*part /tmp --mountoptions=nodev$'
line3='^\W*passwd --minlen=14$'

$OSCAP ds sds-compose $srcdir/$xccdf $sds 2>&1 > $stderr
[ -f $stderr ]; [ ! -s $stderr ]
$OSCAP ds sds-validate $sds

datastream_id=scap_org.open-scap_datastream_from_xccdf_test_report_anaconda_fixes.xccdf.xml
component_id=scap_org.open-scap_cref_test_report_anaconda_fixes.xccdf.xml

$OSCAP info $sds | grep $datastream_id
$OSCAP info $sds | grep $component_id

$OSCAP xccdf generate fix --template urn:redhat:anaconda:pre \
	--datastream-id $datastream_id --xccdf-id $component_id \
	--output $result $sds 2>&1 > $stderr
[ -f $stderr ]; [ ! -s $stderr ]; :> $stderr
grep "$line1" $result
grep "$line2" $result
grep -v "$line1" $result | grep -v "$line2" | grep -v "$line3"

case $(uname) in
	FreeBSD)
		[ "`grep -v "$line1" $result | grep -v "$line2" | gsed 's/\W//g'`"x == x ]
		;;
	*)
		[ "`grep -v "$line1" $result | grep -v "$line2" | sed 's/\W//g'`"x == x ]
		;;
esac

:> $result

$OSCAP xccdf generate fix --template urn:redhat:anaconda:pre \
	--profile xccdf_moc.elpmaxe.www_profile_1 \
	--output $result $sds 2>&1 > $stderr
[ -f $stderr ]; [ ! -s $stderr ]; :> $stderr
grep "$line1" $result
grep "$line2" $result
grep "$line3" $result
grep -v "$line1" $result | grep -v "$line2" | grep -v "$line3"

case $(uname) in
	FreeBSD)
		[ "`grep -v "$line1" $result | grep -v "$line2" | grep -v "$line3" | gsed 's/\W//g'`"x == x ]
		;;
	*)
		[ "`grep -v "$line1" $result | grep -v "$line2" | grep -v "$line3" | sed 's/\W//g'`"x == x ]
		;;
esac

rm $result $sds
