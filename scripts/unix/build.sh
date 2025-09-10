#!/bin/bash
set -e

show_help() {
    cat <<EOF
Usage: $0 <release | debug> [options]

Optional arguments:
  --tests              Build tests
  --tidy               Enable clang-tidy
  --format             Run clang-format
  --doc                Build documentation (Doxygen)
  -h, --help           Show this help message and exit

Examples:
  $0 release --tests --tidy
  $0 debug --format
EOF
}

if [[ $# -lt 1 ]]; then
    echo "[ERROR] Missing build_type argument"
    show_help
    exit 1
fi

case "$1" in
    -h|--help)
        show_help
        exit 0
        ;;
esac

BUILD_TYPE=$1
shift

case "$BUILD_TYPE" in
    release)
        BUILD_DIR="cmake-build-release"
        CMAKE_BUILD_TYPE="Release"
        ;;
    debug)
        BUILD_DIR="cmake-build-debug"
        CMAKE_BUILD_TYPE="Debug"
        ;;
    *)
        echo "[ERROR] First argument must be 'release' or 'debug'"
        exit 1
        ;;
esac

CMAKE_CMD=(cmake -S . -B "$BUILD_DIR" -G "Ninja" -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc)

RUN_TIDY=OFF
RUN_TESTS=OFF
RUN_FORMAT=OFF
RUN_DOC=OFF

while [[ $# -gt 0 ]]; do
    case "$1" in
        --tidy)
            RUN_TIDY=ON
            CMAKE_CMD+=(-DUSE_CLANG_TIDY=ON)
            ;;
        --tests)
            RUN_TESTS=ON
            CMAKE_CMD+=(-DBUILD_TESTS=ON)
            ;;
        --format)
            RUN_FORMAT=ON
            CMAKE_CMD+=(-DUSE_CLANG_FORMAT=ON)
            ;;
        --doc)
            RUN_DOC=ON
            CMAKE_CMD+=(-DBUILD_DOC=ON)
            ;;
        *)
            echo "[WARNING] Unknown option: $1"
            ;;
    esac
    shift
done

echo "[INFO] Configuring project..."
"${CMAKE_CMD[@]}"

BUILD_TARGETS=()

if [[ "$RUN_TESTS" == "OFF" &&
      "$RUN_DOC" == "OFF" &&
      "$RUN_FORMAT" == "OFF" ||
      "$RUN_TIDY" == "ON" ]]; then
    BUILD_TARGETS+=("all")
fi

if [[ "$RUN_FORMAT" == "ON" ]]; then
    BUILD_TARGETS+=("clang-format" "clang-format-check")
fi
if [[ "$RUN_TESTS" == "ON" ]]; then
    BUILD_TARGETS+=("rtype-tests")
fi
if [[ "$RUN_DOC" == "ON" ]]; then
    BUILD_TARGETS+=("doxygen")
fi

if [[ ${#BUILD_TARGETS[@]} -eq 0 ]]; then
    echo "[INFO] Nothing to build."
    exit 0
fi

echo "[INFO] Building targets: ${BUILD_TARGETS[*]}"
cmake --build "$BUILD_DIR" --target "${BUILD_TARGETS[@]}" -- -j 4

echo "[INFO] Build completed successfully."
