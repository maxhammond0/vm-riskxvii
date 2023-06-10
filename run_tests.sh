INFILES="./tests/in/*"
for in in $INFILES
do
    binary=$"./tests/e2e/binaries"${in:10:-4}$".mi"
    out=$"./tests/e2e/out"${in:10:-3}$".out"

    diff=$(cat $in | ./vm_riskxvii $binary | diff $out -)
    if [ "$diff" == "" ]
    then
        echo "test pasted for $in"
    else
        echo "TESS FAILED FOR $in"
        echo "EXITNG!"
        exit $1
    fi

done

echo "Test passed for all files in ./tests/e2e/in"

gcov vm_riskxvii.c
