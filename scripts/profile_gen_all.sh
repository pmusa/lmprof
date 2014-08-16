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
  mv lmprof_default_output.lua out/lmprof_$f
  if [ $? -eq 0 ]; then
    echo -e "==   ${f%%.lua}\t OK"
  fi
done

echo "====================================="
