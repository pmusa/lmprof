SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR

cd ..
LMPROF_HOME=`pwd`

echo "====================================="
echo "==  Generating profile test files  =="
echo "====================================="

cd $LMPROF_HOME/test/
for f in *.lua;
do
  lua $f > /dev/null
  if [ "$f" = "simple.lua" ] || [ "$f" = "tailcall.lua" ] ||
     [ "$f" = "string.lua" ]; then
    mv lmprof_$f out/
  elif [ "$f" = "multistate.lua" ]; then
    mv lmprof_channel.lua lmprof_main.lua out/
    mv lmprof_receiver.lua lmprof_sender.lua out/
  else
    mv lmprof_default_output.lua out/lmprof_$f
  fi
  if [ $? -eq 0 ]; then
    echo -e "==   ${f%%.lua}\t OK"
  fi
done

echo "====================================="
