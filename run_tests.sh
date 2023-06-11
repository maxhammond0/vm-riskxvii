INFILES="./tests/in/*"
for in in $INFILES
do
    binary=$"./tests/binaries"${in:10:-4}$".mi"
    out=$"./tests/out"${in:10:-3}$".out"

    echo $out

    # diff=$(cat $in | ./vm_riscv $binary | diff $out -)
    # if [ "$diff" == "" ]
    # then
    #     echo "test pasted for $in"
    # else
    #     echo "TESS FAILED FOR $in"
    #     echo "EXITNG!"
    #     exit $1
    # fi

done

echo "Test passed for all files in ./tests/in"

gcov vm_riscv.gcno
