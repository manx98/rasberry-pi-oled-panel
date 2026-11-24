#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PI_SSH="${PI_SSH:-wenyiyu@192.168.137.71}"
PI_SYSROOT="${PI_SYSROOT:-$ROOT/.sysroot}"
mkdir -p "$PI_SYSROOT"
if mountpoint -q "$PI_SYSROOT"; then
    printf 'Existing SSHFS sysroot: %s\n' "$PI_SYSROOT"
    exit 0
fi

# Install sshfs in the unprivileged project tool directory if unavailable.
# This leaves the host system packages untouched (requires apt package metadata).
if command -v sshfs >/dev/null 2>&1; then
    SSHFS="$(command -v sshfs)"
else
    TOOLS="$ROOT/.tools/sshfs"
    SSHFS="$TOOLS/usr/bin/sshfs"
    if [[ ! -x "$SSHFS" ]]; then
        mkdir -p "$TOOLS"
        (cd "$TOOLS" && apt download sshfs && dpkg-deb -x sshfs_*.deb .)
    fi
fi

command -v fusermount3 >/dev/null || { echo 'fusermount3 is required (fuse3)' >&2; exit 1; }
"$SSHFS" -o ro,reconnect,ServerAliveInterval=15,ServerAliveCountMax=3 \
    -o ssh_command='ssh -o BatchMode=yes -o ConnectTimeout=10' \
    "$PI_SSH:/" "$PI_SYSROOT"
if [[ ! -f "$PI_SYSROOT/usr/include/linux/gpio.h" ]]; then
    echo 'Target Linux headers are missing from mounted sysroot.' >&2
    exit 1
fi
printf 'Mounted %s:/ read-only on %s\n' "$PI_SSH" "$PI_SYSROOT"
