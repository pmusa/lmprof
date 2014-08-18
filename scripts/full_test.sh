SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR

rm -rf lmprof
mkdir lmprof

cp $SCRIPT_DIR/../src/reduce/*.lua lmprof

./profile_gen_all.sh
./profile_teste_all.sh
./reduce_gen_all.sh
./reduce_test_all.sh

rm -rf lmprof
