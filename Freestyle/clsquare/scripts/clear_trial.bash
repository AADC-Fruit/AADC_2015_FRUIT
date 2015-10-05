#!/bin/bash
# clear_trial.bash <dir>: räumt Verzeichnis auf

EXPECTED_ARGS=1
E_BADARGS=65

if [ $# -ne $EXPECTED_ARGS ]
then
  echo "Usage: `basename $0` {dir_name}"
  exit $E_BADARGS
fi

pushd . >> /dev/null
cd $1
echo "Clearing Directory" $1
rm -rf *~ "#"*
rm -rf nets/*
rm -rf tmp/*
rm -rf results/*
popd  >> /dev/null



