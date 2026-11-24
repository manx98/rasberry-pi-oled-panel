#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${ROOT}/build-zero2w/raspberry_pi_oled_panel"
PACKAGE=raspberry-pi-oled-panel
ARCH=arm64
SKIP_BUILD=false

usage() {
    cat <<'HELP'
Usage: ./scripts/build-deb.sh [--skip-build]

Build a Raspberry Pi Zero 2 W ARM64 executable, then pack a Debian 13 .deb.
  --skip-build  Package the existing build-zero2w/raspberry_pi_oled_panel.

Environment:
  PI_SSH, PI_SYSROOT, BUILD_JOBS and PANEL_ENABLE_*: as build-zero2w.sh
  DEB_VERSION: override package version (default: 0.1.0+gitYYYYMMDD.HASH-1)
  DEB_OUTPUT_DIR: output directory (default: project dist/)
HELP
}

for arg in "$@"; do
    case "$arg" in
        --skip-build) SKIP_BUILD=true ;;
        --help|-h) usage; exit 0 ;;
        *) echo "Unknown argument: $arg" >&2; usage >&2; exit 2 ;;
    esac
done

for required in dpkg-deb aarch64-linux-gnu-readelf install mktemp; do
    command -v "$required" >/dev/null 2>&1 || {
        echo "Missing build tool: $required" >&2
        exit 1
    }
done

if [[ "$SKIP_BUILD" == false ]]; then
    "$ROOT/scripts/build-zero2w.sh"
fi
[[ -f "$BUILD" ]] || { echo "ARM64 executable not found: $BUILD" >&2; exit 1; }
if ! aarch64-linux-gnu-readelf -h "$BUILD" | grep -Eq 'Machine:[[:space:]]+AArch64'; then
    echo "Not an ARM64 executable: $BUILD" >&2
    exit 1
fi

COMMIT="$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo local)"
VERSION="${DEB_VERSION:-0.1.0+git$(date -u +%Y%m%d).${COMMIT}-1}"
dpkg --validate-version "$VERSION"
OUTPUT_DIR="${DEB_OUTPUT_DIR:-$ROOT/dist}"
mkdir -p "$OUTPUT_DIR"
OUTPUT_DIR="$(cd "$OUTPUT_DIR" && pwd)"

STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
mkdir -p "$STAGE/DEBIAN" "$STAGE/usr/bin" "$STAGE/usr/lib/systemd/system"
# mktemp uses mode 0700. Debian control and filesystem directories must be
# traversable with conventional package directory permissions.
chmod 0755 "$STAGE" "$STAGE/DEBIAN" "$STAGE/usr" "$STAGE/usr/bin" \
    "$STAGE/usr/lib" "$STAGE/usr/lib/systemd" "$STAGE/usr/lib/systemd/system"
install -m 0755 "$BUILD" "$STAGE/usr/bin/$PACKAGE"
install -m 0644 "$ROOT/packaging/$PACKAGE.service" \
    "$STAGE/usr/lib/systemd/system/$PACKAGE.service"
for script in postinst prerm postrm; do
    install -m 0755 "$ROOT/packaging/debian/$script" "$STAGE/DEBIAN/$script"
done

INSTALLED_SIZE="$(du -sk "$STAGE/usr" | awk '{print $1}')"
cat > "$STAGE/DEBIAN/control" <<CONTROL
Package: $PACKAGE
Version: $VERSION
Section: utils
Priority: optional
Architecture: $ARCH
Maintainer: manx98 <1323517022@qq.com>
Installed-Size: $INSTALLED_SIZE
Depends: libc6 (>= 2.38), libstdc++6, libgcc-s1, libnm0, libglib2.0-0t64, network-manager, systemd
Homepage: https://github.com/manx98/rasberry-pi-oled-panel
Description: SH1106 OLED system monitor and controller for Raspberry Pi
 Two-button GPIO23/GPIO24 menu, NetworkManager Wi-Fi and interfaces,
 CPU temperature-controlled GPIO12 fan, and power management.
 The systemd service runs as root and starts automatically after installation.
CONTROL
chmod 0644 "$STAGE/DEBIAN/control"

OUT="$OUTPUT_DIR/${PACKAGE}_${VERSION}_${ARCH}.deb"
dpkg-deb --build --root-owner-group "$STAGE" "$OUT"
echo "Built Debian package: $OUT"
dpkg-deb --field "$OUT" Package Version Architecture Depends
echo 'Install on Raspberry Pi: sudo apt install ./<package>.deb'
