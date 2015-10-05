#!/bin/bash
# start_trial qvz idx
# startet Versuch mit Bezeichnung idx des Quellverzeichnisses qvz
# legt ein temporäres Versuchverzeichnis an mit einer Bezeichnung aus 
# den letzten beiden Verzeichnissen des Grundversuches
# löscht das wieder 

EXPECTED_ARGS=2
E_BADARGS=65

if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Usage: `basename $0` {trial_dir_name} {idx}"
  exit $E_BADARGS
fi


pushd . >>/dev/null
cd $1
#qvz=$1
qvz=$PWD
popd  >>/dev/null

idx=$2

path=${qvz%/*}
#echo $path
superlocaldirname=${path##*/}
#echo $superlocaldirname
localdirname=${qvz##*/}
#echo $localdirname
TrialDirName=$superlocaldirname"_"$localdirname"_"$idx
#echo $TrialDirName
pushd . >>/dev/null
rm -rf $TrialDirName
cp -r $qvz ./$TrialDirName
cd $TrialDirName
randomSeed=$idx*1000 # maybe replace this by sth. else
clsquare train.cls $randomSeed 
mkdir -p $qvz/results  # if results not exists, mkdir qvz/results
cp results/test1.stat $qvz/results/test1.$idx.stat 
cd ..
tar cvfz trial.$idx.tar.gz $TrialDirName >>/dev/null
mv trial.$idx.tar.gz $qvz/results #evtl.
rm -rf $TrialDirName
popd >>/dev/null
