CURRENT_DIR=`pwd`

# get the directory of the script (from: http://stackoverflow.com/a/246128)
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
SCRIPT_DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

cd $SCRIPT_DIR/swig/python
sh prepare.sh
python setup.py.in build_ext --inplace

mkdir $SCRIPT_DIR/release
cp simstring.py $SCRIPT_DIR/release/
cp _simstring*.so $SCRIPT_DIR/release/

cd $CURRENT_DIR
