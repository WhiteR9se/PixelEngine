#!/usr/bin/env bash
set -e

# Ensure an executable path was passed
if [ -z "$1" ]; then
    echo "Usage: $0 <path_to_executable> [args...]"
    exit 1
fi

TARGET_BIN="$1"
shift  # Shift arguments so "$@" holds any remaining flags/arguments for the binary

# 1. Setup output directory
PROFILE_DIR="bin/profile"
mkdir -p "$PROFILE_DIR"

PERF_DATA="$PROFILE_DIR/perf.data"
PERF_SCRIPT="$PROFILE_DIR/perf_script.txt"

echo "[PROFILER] Profiling target: $TARGET_BIN"
echo "[PROFILER] Storing artifacts in: $PROFILE_DIR/"

# 2. Record CPU profile
# Using sudo to guarantee PMU hardware event capture
sudo perf record -F 9999 -g -o "$PERF_DATA" -- "$TARGET_BIN" "$@"

# 3. Reclaim ownership so root doesn't lock the files
sudo chown "$USER:$USER" "$PERF_DATA"

# 4. Export the profile data to text format (-f forces past ownership checks)
echo "[PROFILER] Converting perf.data to text..."
perf script -f -i "$PERF_DATA" -F +pid > "$PERF_SCRIPT"

echo "[PROFILER] Profile written to: $PERF_SCRIPT"

# 5. Launch Brave Browser pointing to the profiler
echo "[PROFILER] Launching Brave Browser..."
if command -v brave-browser &> /dev/null; then
    brave-browser "https://profiler.firefox.com" &
elif command -v brave &> /dev/null; then
    brave "https://profiler.firefox.com" &
else
    # Fallback to system default browser if brave binary name differs
    xdg-open "https://profiler.firefox.com" &
fi

echo "[PROFILER] Done! Drag and drop '$PERF_SCRIPT' into the browser tab."
