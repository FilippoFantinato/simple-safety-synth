#!/bin/bash

function exec_nusmv_no_warnings {
    NuSMV -source $1 2> /dev/null
}

base=`dirname -- "${BASH_SOURCE[0]}"`
proj_base=$base/../..

exec=$proj_base/build/simple-synth
realizable_examples=$proj_base/examples/safety/realizable
unrealizable_examples=$proj_base/examples/safety/unrealizable
moduledir=$proj_base/examples/safety/modules
outdir=$base/safety-controllers

mkdir -p $outdir

realizable=0
for f in `ls $unrealizable_examples`; do
    name=${f%.*}.smv
    output=`$exec $unrealizable_examples/$f`

    if grep -E -q "Realizable" <<< $output ; then
        realizable=$(( realizable + 1 ))
    fi
done

unrealizable=0
for f in `ls $realizable_examples`; do
    name=${f%.*}.smv
    output=`$exec --synthesize --smv main --output $outdir/$name $realizable_examples/$f`

    if grep -E -q "Unrealizable" <<< $output ; then
        unrealizable=$(( unrealizable + 1 ))
    fi
done

echo "Realizable formula that should be unrealizable: $realizable" 
echo "Unrealizable formula that should be realizable: $unrealizable"

formula_cnt=0
invariant_cnt=0
not_invariant_cnt=0
for f in `ls $outdir`; do
    name=${f%.*}
    nusmv_commands=$base/$name.commands.txt
    invariant=`sed "/--/d" $moduledir/$f | pcregrep -M "LTLSPEC(.|\n)*" | sed "s/LTLSPEC *//" | tr "\n" " "`

    # formula always true
    echo "read_model -i $outdir/$f; go; check_ltlspec -p \"G(formula)\"; quit;" > $nusmv_commands
    output=`exec_nusmv_no_warnings $nusmv_commands`
    if grep -E -iq "is false" <<< $output ; then
        formula_cnt=$(( formula_cnt + 1 ))
        echo "$f: formula is not always true"
    fi

    # invariant true
    echo "read_model -i $outdir/$f; go; check_ltlspec -p \"$invariant\"; quit;" > $nusmv_commands
    output=`exec_nusmv_no_warnings $nusmv_commands`
    if grep -E -iq "is false" <<< $output ; then
        invariant_cnt=$(( invariant_cnt + 1 ))
        echo "$f: invariant not true"
    fi

    # not invariant false
    echo "read_model -i $outdir/$f; go; check_ltlspec -p \"!($invariant)\"; quit;" > $nusmv_commands
    output=`exec_nusmv_no_warnings $nusmv_commands`
    if grep -E -iq "is true" <<< $output ; then
        not_invariant_cnt=$(( not_invariant_cnt + 1 ))
        echo "$f: !invariant not false"
    fi

    rm -f $nusmv_commands
done

rm -r $outdir

echo "Formula becomes false in $formula_cnt controllers."
echo "Invariant becomes false in $invariant_cnt controllers."
echo "!Invariant becomes true in $not_invariant_cnt controllers."

if [[ $realizable == 0 && ($unrealizable == 0 && ($formula_cnt == 0 && ($invariant_cnt == 0 && $not_invariant_cnt == 0))) ]]
then
    echo "Everything is fine!"
else
    echo "There is something wrong!"
    exit 1
fi
