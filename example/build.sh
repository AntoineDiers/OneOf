SCRIPT_DIR=$(dirname $(realpath "$0"))

mkdir -p "$SCRIPT_DIR/build"
cd "$SCRIPT_DIR/build" || exit 1
cmake .. || exit 1
cmake --build . || exit 1