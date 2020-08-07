#!/usr/bin/env bash
. $builddir/tests/test_common.sh

# Test loading of OVAL results files located relatively to XCCDF file

set -e
set -o pipefail

# cd to directory and run test
function testInDirectory(){
	local directory="$1"
	local xccdf="$2"

	pushd "$directory"

	case $(uname) in
		FreeBSD)
			local result=$(mktemp /tmp/${name}.out.result.XXXXXX)
			local report=$(mktemp /tmp/${name}.out.report.XXXXXX)
			local stderr=$(mktemp /tmp/${name}.err.XXXXXX)
			;;
		*)
			local result=$(mktemp -t ${name}.out.result.XXXXXX)
			local report=$(mktemp -t ${name}.out.report.XXXXXX)
			local stderr=$(mktemp -t ${name}.err.XXXXXX)
			;;
	esac

	echo "Stderr file = $stderr"
	echo "Result file = $result"
	echo "Report file = $report"

	$OSCAP xccdf eval --oval-results --results $result --report $report "$xccdf" 2> $stderr 

	# Check existence of oval-results
	[ -f $directory/*oval*always-fail*oval.xml.result.xml ]

	[ -f $stderr ]; [ ! -s $stderr ]; rm $stderr

	# Check result
	assert_exists 2 '//rule-result/result'
	assert_exists 2 '//rule-result/result[text()="pass" or text()="fail"]'

	# Check report
	grep "Testing permissions on /not_executable" "$report" --quiet

	rm -f $result $report

	popd
}

name=$(basename $0 .sh)

case $(uname) in
	FreeBSD)
		ORIGINAL_XCCDF="$(readlink -f "$srcdir/${name}.xccdf.xml")"
		;;
	*)
		ORIGINAL_XCCDF="$(readlink -e "$srcdir/${name}.xccdf.xml")"
		;;
esac

### test in XCCDF's directory

# We don't have write access to directory with XCCDF (after make distcheck),
# so we have to copy required files to directory with right access and do tests there
testDir=`mktemp -d`
mkdir -p "$testDir/oval/always-fail"
cp "$ORIGINAL_XCCDF" "$srcdir/test_default_selector.oval.xml" "$testDir"
cp "$srcdir/oval/always-fail/oval.xml" "$testDir/oval/always-fail"

case $(uname) in
	FreeBSD)
		XCCDF="$(readlink -f "$testDir/${name}.xccdf.xml")"
		;;
	*)
		XCCDF="$(readlink -e "$testDir/${name}.xccdf.xml")"
		;;
esac

testInDirectory "$testDir" "$XCCDF"
rm -rf "$testDir"


### test in random directory - different from directory of XCCDF
RANDOM_DIRECTORY=`mktemp -d`
testInDirectory "$RANDOM_DIRECTORY" "$ORIGINAL_XCCDF"
rm -rf $RANDOM_DIRECTORY
