#!/bin/bash
#
# Drives the server-client binary over the loopback interface for each test
# size and captures the transferred data.
#
# For every size in 32/64/256/1024/4096:
#   - starts the app as a SERVER (sends server.txt, writes what it receives)
#   - runs  the app as a CLIENT (sends client.txt, writes what it receives)
#
# Inputs : tests/<size>/{client.txt,server.txt}
# Outputs: tests/output/<size>/{server_received.txt,client_received.txt,*.log}
#
# Override the binary location with: BIN=/path/to/server_client ./run_tests.sh

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TESTS_DIR="$SCRIPT_DIR"
OUTPUT_DIR="$SCRIPT_DIR/output"

# Locate the server_client binary (BIN env var wins; otherwise probe common spots)
BIN="${BIN:-}"
if [ -z "$BIN" ]; then
  for cand in \
      /app/server_client \
      "$SCRIPT_DIR/../server_client" \
      "$SCRIPT_DIR/../build/bin/server_client" \
      "$SCRIPT_DIR/../build/server_client"; do
    if [ -x "$cand" ]; then BIN="$cand"; break; fi
  done
fi
if [ -z "$BIN" ] || [ ! -x "$BIN" ]; then
  echo "ERROR: server_client binary not found. Set BIN=/path/to/server_client" >&2
  exit 1
fi
echo "Using binary: $BIN"

SIZES=(32 64 256 1024 4096)
BASE_PORT="${BASE_PORT:-5000}"
fail=0

for i in "${!SIZES[@]}"; do
  size="${SIZES[$i]}"
  port=$((BASE_PORT + i))
  in_dir="$TESTS_DIR/$size"
  out_dir="$OUTPUT_DIR/$size"

  if [ ! -f "$in_dir/client.txt" ] || [ ! -f "$in_dir/server.txt" ]; then
    echo "=== size $size: SKIP (missing $in_dir/client.txt or server.txt)"
    fail=1
    continue
  fi

  mkdir -p "$out_dir"
  echo "=== size $size (port $port) ==="

  # Server: receives the client's payload (-> server_received.txt), then sends server.txt
  timeout 15s "$BIN" -t loopback -p "$port" -s \
      -i "$in_dir/server.txt" \
      -o "$out_dir/server_received.txt" \
      -d "$out_dir/server_debug.log" &
  server_pid=$!

  # Give the server time to bind/listen before the client connects
  sleep 0.5

  # Client: sends client.txt, receives the server's payload (-> client_received.txt)
  timeout 15s "$BIN" -t loopback -p "$port" -c \
      -i "$in_dir/client.txt" \
      -o "$out_dir/client_received.txt" \
      -d "$out_dir/client_debug.log"

  wait "$server_pid" 2>/dev/null

  # Round-trip check: what each side received should equal what the other sent
  if cmp -s "$in_dir/client.txt" "$out_dir/server_received.txt"; then
    echo "  client -> server: OK"
  else
    echo "  client -> server: MISMATCH"; fail=1
  fi
  if cmp -s "$in_dir/server.txt" "$out_dir/client_received.txt"; then
    echo "  server -> client: OK"
  else
    echo "  server -> client: MISMATCH"; fail=1
  fi
  echo "  outputs -> $out_dir"
done

echo
if [ "$fail" -eq 0 ]; then
  echo "RESULT: ALL SIZES PASSED"
else
  echo "RESULT: FAILURES DETECTED"
fi
exit "$fail"
