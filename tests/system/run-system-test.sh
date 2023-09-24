#!/bin/bash

function exec_nusmv_no_warnings () {
    NuSMV -source $1 2> /dev/null
}

base="$(dirname -- "${BASH_SOURCE[0]}")"

exec=$1
examples=$2
outdir=$base/controllers
moduledir=$base/modules

for f in `ls $examples`
do
    name=${f%.*}.smv
    $exec --synthesize --smv main --output $outdir/$name $examples/$f &> /dev/null
done

formula_cnt=0
invariant_cnt=0
not_invariant_cnt=0
for f in `ls $outdir`
do
    name=${f%.*}
    nusmv_commands=`echo $name-commands.nusmv.txt`
    invariant=`sed "/--/d" $moduledir/$f | pcregrep -M "LTLSPEC(.|\n)*" | sed "s/LTLSPEC *//" | tr "\n" " "`

    # formula always true
    echo "read_model -i $outdir/$f; go; check_ltlspec -p \"G(formula)\"; quit;" > $base/$nusmv_commands
    output=`exec_nusmv_no_warnings $base/$nusmv_commands`
    if echo $output | grep -E -iq "is false"; then
        formula_cnt=$(( formula_cnt + 1 ))
        echo "$f: formula is not always true"
    fi

    # invariant true
    echo "read_model -i $outdir/$f; go; check_ltlspec -p \"$invariant\"; quit;" > $base/$nusmv_commands
    output=`exec_nusmv_no_warnings $base/$nusmv_commands`
    if echo $output | grep -E -iq "is false"; then
        invariant_cnt=$(( invariant_cnt + 1 ))
        echo "$f: invariant not true"
    fi

    # not invariant false
    echo "read_model -i $outdir/$f; go; check_ltlspec -p \"!($invariant)\"; quit;" > $base/$nusmv_commands
    output=`exec_nusmv_no_warnings $base/$nusmv_commands`
    if echo $output | grep -E -iq "is true"; then
        not_invariant_cnt=$(( not_invariant_cnt + 1 ))
        echo "$f: !invariant not false"
    fi

    rm -f $base/$nusmv_commands
done

echo ""
echo "Formula becomes false in $formula_cnt controllers."
echo "Invariant becomes false in $invariant_cnt controllers."
echo "!Invariant becomes true in $not_invariant_cnt controllers."


if [[ $formula_cnt == 0 && ($invariant_cnt == 0 && $not_invariant_cnt == 0) ]]
then
    echo "Everything is fine!"
fi
