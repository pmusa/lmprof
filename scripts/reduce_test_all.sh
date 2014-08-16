SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR

cd ..
LMPROF_HOME=`pwd`


echo "======================"
echo "==  Testing reduce  =="
echo "======================"

SIZE=19
TMP_FILE=tmp
cd $LMPROF_HOME/test/
for f in *.lua;
do
  LMPROF_FILE=$LMPROF_HOME/test/out/lmprof_$f
  REDUCE_FILE=$LMPROF_HOME/test/out/reduce_${f%%.lua}.txt
  cd $LMPROF_HOME/src/reduce/
  lua text.lua all $LMPROF_FILE $SIZE > $TMP_FILE
  diff -bB tmp $REDUCE_FILE > /dev/null
  if [ $? -eq 0 ]; then
    echo -e "==   ${f%%.lua}\tOK"
  else
    echo -e "==   ${f%%.lua} \tFAILED"
  fi
  rm $TMP_FILE
done

echo "======================"
