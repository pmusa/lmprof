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
  if [ "$f" = "simple.lua" ] || [ "$f" = "tailcall.lua" ] ||
     [ "$f" = "string.lua" ]; then
    lua $SCRIPT_FILE lmprof_$f out/lmprof_$f
  elif [ "$f" = "multistate.lua" ]; then
    lua $SCRIPT_FILE lmprof_channel.lua out/lmprof_channel.lua
    lua $SCRIPT_FILE lmprof_main.lua out/lmprof_main.lua
    lua $SCRIPT_FILE lmprof_receiver.lua out/lmprof_receiver.lua
    lua $SCRIPT_FILE lmprof_sender.lua out/lmprof_sender.lua
  else
    lua $SCRIPT_FILE lmprof_default_output.lua out/lmprof_$f
  fi
  errors=$?
  if [ $errors -eq 0 ]; then
    echo -e "==   ${f%%.lua}\tOK"
  elif [ $errors -le $ACCEPTED_ERRORS ]; then
    echo -e "==   ${f%%.lua}\tOK ($errors errors)"
  else
    echo -e "==   ${f%%.lua}\tFAILED, $errors errors"
  fi
  if [ "$f" = "simple.lua" ] || [ "$f" = "tailcall.lua" ] ||
     [ "$f" = "string.lua" ]; then
    rm $LMPROF_HOME/test/lmprof_$f
  elif [ "$f" = "multistate.lua" ]; then
    rm $LMPROF_HOME/test/lmprof_channel.lua
    rm $LMPROF_HOME/test/lmprof_main.lua
    rm $LMPROF_HOME/test/lmprof_receiver.lua
    rm $LMPROF_HOME/test/lmprof_sender.lua
  else
    rm $LMPROF_HOME/test/lmprof_default_output.lua
  fi
done

echo "====================================================="
