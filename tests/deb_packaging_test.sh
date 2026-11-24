#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if (($# > 1)); then
    echo "Usage: $0 [path/to/raspberry-pi-oled-panel_arm64.deb]" >&2
    exit 2
fi

if (($# == 1)); then
    DEB="$1"
else
    shopt -s nullglob
    packages=("$ROOT"/dist/raspberry-pi-oled-panel_*_arm64.deb)
    if ((${#packages[@]} == 0)); then
        echo "No package in dist/. Run ./scripts/build-deb.sh first." >&2
        exit 1
    fi
    DEB="$(ls -t "${packages[@]}" | head -1)"
fi

command -v dpkg-deb >/dev/null
command -v aarch64-linux-gnu-readelf >/dev/null
[[ -f "$DEB" ]] || { echo "Not found: $DEB" >&2; exit 1; }
[[ "$(dpkg-deb -f "$DEB" Package)" == raspberry-pi-oled-panel ]]
[[ "$(dpkg-deb -f "$DEB" Architecture)" == arm64 ]]
DEPS="$(dpkg-deb -f "$DEB" Depends)"
for dependency in 'libc6' 'libstdc++6' 'libnm0' 'libglib2.0-0t64' 'network-manager' 'systemd'; do
    grep -Fq "$dependency" <<< "$DEPS"
done

TEMP="$(mktemp -d)"
trap 'rm -rf "$TEMP"' EXIT
dpkg-deb --extract "$DEB" "$TEMP/root"
dpkg-deb --control "$DEB" "$TEMP/control"
BIN="$TEMP/root/usr/bin/raspberry-pi-oled-panel"
UNIT="$TEMP/root/usr/lib/systemd/system/raspberry-pi-oled-panel.service"

[[ -x "$BIN" ]]
[[ -f "$UNIT" ]]
aarch64-linux-gnu-readelf -h "$BIN" | grep -Eq 'Machine:[[:space:]]+AArch64'
grep -Fxq 'Type=simple' "$UNIT"
grep -Fxq 'User=root' "$UNIT"
grep -Fxq 'Group=root' "$UNIT"
grep -Fxq 'ExecStart=/usr/bin/raspberry-pi-oled-panel' "$UNIT"
grep -Fxq 'ExecStopPost=/usr/bin/raspberry-pi-oled-panel --clear-display' "$UNIT"
grep -Fxq 'WantedBy=multi-user.target' "$UNIT"
grep -Fxq 'Restart=always' "$UNIT"
grep -Fxq 'RestartSec=5s' "$UNIT"
grep -Fxq 'StartLimitIntervalSec=0' "$UNIT"
grep -Fxq 'StateDirectory=raspberry-pi-oled-panel' "$UNIT"

for script in postinst prerm postrm; do
    [[ -x "$TEMP/control/$script" ]]
    sh -n "$TEMP/control/$script"
done
grep -Fq 'systemctl enable "$SERVICE"' "$TEMP/control/postinst"
grep -Fq 'systemctl restart "$SERVICE"' "$TEMP/control/postinst"
grep -Fq 'systemctl stop raspberry-pi-oled-panel.service' "$TEMP/control/prerm"
grep -Fq 'systemctl disable raspberry-pi-oled-panel.service' "$TEMP/control/prerm"

# Ensure ownership is root (not the WSL builder's user) and only the intended
# binary and unit file are installed by the Debian data archive.
contents="$(dpkg-deb --contents "$DEB")"
if grep -Ev '^d' <<< "$contents" | grep -v 'root/root' >/dev/null; then
    echo 'Unexpected package file ownership' >&2
    exit 1
fi
[[ "$(grep -cE '^-.* \./usr/(bin/raspberry-pi-oled-panel|lib/systemd/system/raspberry-pi-oled-panel.service)$' <<< "$contents")" == 2 ]]

echo 'Debian package validation PASS'
printf '  File: %s\n  Version: %s\n  Arch: arm64\n  Service: root/systemd auto-start\n' \
    "$DEB" "$(dpkg-deb -f "$DEB" Version)"
