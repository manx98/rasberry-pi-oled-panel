#!/usr/bin/env bash
# Manual root launcher for Raspberry Pi Zero 2 W. No systemd unit/autostart.
set -euo pipefail

PANEL=/home/wenyiyu/raspberry_pi_oled_panel
PID_FILE=/run/raspberry-pi-oled-panel.pid
LOG_FILE=/var/log/raspberry-pi-oled-panel.log
ACTION="${1:-start}"

if [[ "$EUID" -ne 0 ]]; then
    echo "This panel must run as root. Requesting sudo credentials..." >&2
    exec sudo -- /bin/bash "$0" "$@"
fi

find_panel_pids() {
    # Exact command line; do not match the launcher itself or diagnostic jobs.
    pgrep -f '^/home/wenyiyu/raspberry_pi_oled_panel$' || true
}

stop_panel() {
    mapfile -t pids < <(find_panel_pids)
    if ((${#pids[@]} == 0)); then return; fi
    for pid in "${pids[@]}"; do
        kill -TERM "$pid" 2>/dev/null || true
    done
    for n in {1..15}; do
        if [[ -z "$(find_panel_pids)" ]]; then break; fi
        sleep 1
    done
    if [[ -n "$(find_panel_pids)" ]]; then
        echo "Panel did not exit cleanly; will not launch a duplicate." >&2
        return 1
    fi
}

case "$ACTION" in
    start)
        test -x "$PANEL" || { echo "Panel missing: $PANEL" >&2; exit 1; }
        stop_panel
        : > "$LOG_FILE"
        chmod 0640 "$LOG_FILE"
        nohup "$PANEL" >>"$LOG_FILE" 2>&1 </dev/null &
        pid=$!
        printf '%s\n' "$pid" > "$PID_FILE"
        sleep 2
        if ! kill -0 "$pid" 2>/dev/null; then
            echo "Panel failed to start. Check $LOG_FILE" >&2
            tail -25 "$LOG_FILE" >&2
            exit 1
        fi
        printf 'Started OLED panel as root: PID %s (log: %s)\n' "$pid" "$LOG_FILE"
        ps -o pid,user,stat,args -p "$pid"
        ;;
    stop)
        stop_panel
        rm -f "$PID_FILE"
        echo 'Stopped OLED panel.'
        ;;
    foreground)
        test -x "$PANEL" || { echo "Panel missing: $PANEL" >&2; exit 1; }
        stop_panel
        exec "$PANEL"
        ;;
    status)
        ps -eo pid,user,stat,args | grep -E '[r]aspberry_pi_oled_panel' || true
        ;;
    *)
        echo "Usage: $0 {start|stop|foreground|status}" >&2
        exit 2
        ;;
esac
