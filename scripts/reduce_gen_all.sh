SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR

cd ..
LMPROF_HOME=`pwd`

echo "===================================="
echo "==  Generating reduce test files  =="
echo "===================================="

SIZE=19  # skip 2html identical parent conflict
cd $LMPROF_HOME/test/out/
for f in lmprof_*.lua;
do
  f=${f#lmprof_}
  LMPROF_FILE=$LMPROF_HOME/test/out/lmprof_$f
  REDUCE_FILE=$LMPROF_HOME/test/out/reduce_${f%%.lua}.txt
  if [ ! -f $LMPROF_FILE ]; then
    echo -e "==   ${f%%.lua} \tFAILED (input file does not exist)"
  else
    cd $LMPROF_HOME/src/reduce/
    lua text.lua all $LMPROF_FILE $SIZE > $REDUCE_FILE
    echo -e "==   ${f%%.lua} \tOK"
  fi
done

echo "===================================="
