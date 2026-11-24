#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
export PI_SYSROOT="${PI_SYSROOT:-$ROOT/.sysroot}"
"$ROOT/scripts/mount-sysroot.sh"

command -v aarch64-linux-gnu-g++ >/dev/null || { echo 'Install the aarch64-linux-gnu cross compiler.' >&2; exit 1; }
# CMake requires a host-side pkg-config binary. Install into the project if
# unavailable, without modifying the host system or target sysroot.
if ! command -v pkg-config >/dev/null 2>&1; then
    PKG_DIR="$ROOT/.tools/pkgconfig"
    if [[ ! -x "$PKG_DIR/usr/bin/pkg-config" ]]; then
        mkdir -p "$PKG_DIR"
        (cd "$PKG_DIR" && apt download pkg-config && dpkg-deb -x pkg-config_*.deb .)
    fi
    export PKG_CONFIG="$PKG_DIR/usr/bin/pkg-config"
fi
cmake -S "$ROOT" -B "$ROOT/build-zero2w" \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/toolchains/zero2w-arm64.cmake" \
    -DPI_SYSROOT="$PI_SYSROOT" \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_SDL2=OFF \
    -DPANEL_ENABLE_RPC="${PANEL_ENABLE_RPC:-OFF}" \
    -DPANEL_ENABLE_SENSORS="${PANEL_ENABLE_SENSORS:-OFF}" \
    -DPANEL_ENABLE_INA226="${PANEL_ENABLE_INA226:-OFF}" \
    -DPANEL_ENABLE_SGP30="${PANEL_ENABLE_SGP30:-OFF}" \
    -DPANEL_ENABLE_SHT3X="${PANEL_ENABLE_SHT3X:-OFF}" \
    -DPANEL_ENABLE_BMP280="${PANEL_ENABLE_BMP280:-OFF}" \
    -DPANEL_ENABLE_FAN="${PANEL_ENABLE_FAN:-ON}" \
    -DPANEL_ENABLE_RELAY="${PANEL_ENABLE_RELAY:-OFF}" \
    "$@"
cmake --build "$ROOT/build-zero2w" --parallel "${BUILD_JOBS:-$(nproc)}"
echo "Built: $ROOT/build-zero2w/raspberry_pi_oled_panel"
