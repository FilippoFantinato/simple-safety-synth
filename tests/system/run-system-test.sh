#!/bin/bash

base="$(dirname -- "${BASH_SOURCE[0]}")"

exec=$1
examples=$2
outdir=$base/controllers
moduledir=$base/modules

for f in `ls $examples`
do
    name=${f%.*}.smv
    outfile=$outdir/$name
    formula=`sed "/--/d" $moduledir/$name | pcregrep -M "LTLSPEC(.|\n)*" | sed "s/LTLSPEC *//"`
    
    $exec --synthesize --smv main --output $outfile $examples/$f &> /dev/null
    echo "LTLSPEC G(formula)"   >> $outfile
    # echo "LTLSPEC $formula"    >> $outfile
    # echo "LTLSPEC !($formula)" >> $outfile
done

for f in `ls $outdir`
do
    formula=`sed "/--/d" $moduledir/$name | pcregrep -M "LTLSPEC(.|\n)*" | sed "s/LTLSPEC *//"`
    output=`NuSMV $outdir/$f`

    # echo "echo $output | grep -E -iq "$formula.*is true""

    if echo $output | grep -E -iq "is true"; then
        echo "$f correct"
    else
        echo "$f not correct"
    fi
done
