#!/usr/bin/env bash
# restage-env.sh — turn-start restaging for the [ephemeral] sandbox (protocol v3.6 §1.5)
#
# The durable surface is /mnt/agents (persistent across turns). Everything under
# /tmp is scratch and may be wiped at any turn boundary. This script restores the
# toolchain and prints the environment exports every build command needs.
#
# Idempotent: safe to run at the start of every turn.
#
# Usage:
#   bash tools/restage-env.sh          # restore + print exports
#   eval "$(bash tools/restage-env.sh --exports)"   # restore + apply exports
#
# Ordering invariants baked into this script (learned first-hand; do not relearn):
#   * CMake IGNORES environment-variable dependency overrides at configure time
#     (CPLUS_INCLUDE_PATH, LIBRARY_PATH, PKG_CONFIG_PATH do not enter the cache).
#     Either export these for EVERY build command, or pass them as -D cache
#     arguments at configure time. We export them here so every command in the
#     same shell inherits them.
#   * The durable mount strips exec bits on copy — chmod +x after every restore
#     and after every relinked binary (ctest fails with BAD_COMMAND otherwise).
#   * The durable mount may serve stale bytes and does not update mtimes: never
#     build inside /mnt/agents; copy sources to /tmp and verify by content.
#   * Long builds must run setsid-detached (nohup jobs die with the tool call).

set -euo pipefail

DURABLE="${DURABLE:-/mnt/agents}"
SYSROOT_TAR="$DURABLE/sysroot/sysroot.tar"
CMAKE_TAR="$DURABLE/sysroot/cmake-root.tar"
SYSROOT=/tmp/sysroot
CMAKE_ROOT=/tmp/cmake-root

if [ ! -f "$SYSROOT_TAR" ] || [ ! -f "$CMAKE_TAR" ]; then
    echo "ERROR: toolchain tarballs missing on the durable surface:" >&2
    echo "  $SYSROOT_TAR" >&2
    echo "  $CMAKE_TAR" >&2
    echo "Restore them before building (see plan/ENVIRONMENT.md §1.5)." >&2
    exit 1
fi

mkdir -p "$SYSROOT" "$CMAKE_ROOT"

# Restore only if absent (idempotent; /tmp may be wiped between turns).
if [ ! -x "$CMAKE_ROOT/usr/bin/cmake" ]; then
    echo "restoring cmake-root from $CMAKE_TAR"
    rm -rf "$CMAKE_ROOT" && mkdir -p "$CMAKE_ROOT"
    tar -xf "$CMAKE_TAR" -C "$CMAKE_ROOT"
fi
if [ ! -d "$SYSROOT/usr/lib/x86_64-linux-gnu" ]; then
    echo "restoring sysroot from $SYSROOT_TAR"
    rm -rf "$SYSROOT" && mkdir -p "$SYSROOT"
    tar -xf "$SYSROOT_TAR" -C "$SYSROOT"
fi

# Exec-bit restoration (the mount strips exec bits on copy).
chmod -R +x "$CMAKE_ROOT/usr/bin" 2>/dev/null || true
find "$SYSROOT/usr" -name "*.so*" -exec chmod +x {} + 2>/dev/null || true

# Verify the restored toolchain actually runs (a check that cannot fail is not a check).
"$CMAKE_ROOT/usr/bin/cmake" --version | head -1 || { echo "ERROR: restored cmake does not run" >&2; exit 1; }

EXPORTS=$(cat <<'EOF'
export PATH=/tmp/cmake-root/usr/bin:/usr/local/bin:/usr/bin:/bin
export LD_LIBRARY_PATH=/tmp/sysroot/usr/lib/x86_64-linux-gnu
export CPLUS_INCLUDE_PATH=/tmp/sysroot/usr/include
export LIBRARY_PATH=/tmp/sysroot/usr/lib/x86_64-linux-gnu
export PKG_CONFIG_PATH=/tmp/sysroot/usr/lib/x86_64-linux-gnu/pkgconfig
EOF
)

if [ "${1:-}" = "--exports" ]; then
    printf '%s\n' "$EXPORTS"
else
    cat <<EOF
restaged OK: cmake $("$CMAKE_ROOT/usr/bin/cmake" --version | head -1 | awk '{print $3}')

Run these exports in every shell that builds (or: eval "\$(bash tools/restage-env.sh --exports)"):

$EXPORTS

Build recipe (never build inside /mnt/agents — stale-byte hazard):
  cp -a /mnt/agents/guitar-amp-vst /tmp/gsrc
  mkdir -p /tmp/gbuild && cd /tmp/gbuild
  cmake -G Ninja -DCMAKE_BUILD_TYPE=Release /tmp/gsrc
  # long link: setsid ninja < /dev/null > build.log 2>&1 &   then poll the log
  # after every relink: chmod +x tests/grimamp_tests tests/grimamp_bench
EOF
fi
