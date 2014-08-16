SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR

SCRIPT_FILE=$SCRIPT_DIR/profile_compare_output.lua

cd ..
LMPROF_HOME=`pwd`

ACCEPTED_ERRORS=5

echo "====================================================="
echo "==  Testing profile (less than $ACCEPTED_ERRORS errors means ok)  =="
echo "====================================================="

cd $LMPROF_HOME/test/
for f in *.lua;
do
  lua $f > /dev/null
  lua $SCRIPT_FILE lmprof_default_output.lua out/lmprof_$f
  errors=$?
  if [ $errors -eq 0 ]; then
    echo -e "==   ${f%%.lua}\tOK"
  elif [ $errors -le $ACCEPTED_ERRORS ]; then
    echo -e "==   ${f%%.lua}\tOK ($errors errors)"
  else
    echo -e "==   ${f%%.lua}\tFAILED, $errors errors"
  fi
  rm $LMPROF_HOME/test/lmprof_default_output.lua
done

echo "====================================================="
