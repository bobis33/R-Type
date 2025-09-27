#!/usr/bin/env bash
# nc-client.sh
# Usage: ./nc-client.sh <host> <port> [message] [udp|tcp]
# Example: ./nc-client.sh 127.0.0.1 12345 "Hello server" udp

set -euo pipefail

HOST="${1:-127.0.0.1}"
PORT="${2:-2560}"
MSG="${3:-Hello server}"
MODE="${4:-udp}"

if command -v nc >/dev/null 2>&1; then
    NETCAT="nc"
elif command -v ncat >/dev/null 2>&1; then
    NETCAT="ncat"
else
    echo "[ERROR] netcat (nc or ncat) not found." >&2
    exit 2
fi

echo "[INFO] Testing ${MODE^^} -> ${HOST}:${PORT} : sending '${MSG}'"

TMP_OUT=$(mktemp)

success=false

if [ "$MODE" = "udp" ]; then
    echo "[INFO] Sending UDP message..."
    if printf "%s" "$MSG" | $NETCAT -u -w1 "$HOST" "$PORT" >"$TMP_OUT" 2>&1; then
        success=true
    fi
else
    echo "[INFO] Connecting TCP..."
    if printf "%s" "$MSG" | $NETCAT -w2 "$HOST" "$PORT" >"$TMP_OUT" 2>&1; then
        success=true
    fi
fi

if $success; then
    echo "[INFO] Message sent successfully."
else
    echo "[WARNING] Failed to send message (server down or unreachable)."
fi

if [ -s "$TMP_OUT" ]; then
    echo "[INFO] Server response:"
    cat "$TMP_OUT"
else
    echo "[INFO] No response from server."
fi

rm -f "$TMP_OUT"
echo "[INFO] Done."
