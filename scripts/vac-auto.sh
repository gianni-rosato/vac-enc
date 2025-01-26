#!/bin/bash

if [ $# -ne 2 ]; then
    echo "Usage: ./vac-auto.sh [input] [bitrate]"
    exit 1
fi

# Set the input and output filenames
in=$1
extension=${1##*.}
fname="${1%.*}"
bitrate=$2

echo -n "Encoding "

lt=$(openssl rand --hex 4)
fnme="$lt"."$extension"
cp "$in" "/dev/shm/$fnme"

# Convert input to FLAC
at=$(/usr/bin/time -f "%e" ffmpeg -hide_banner -loglevel panic -y -i "/dev/shm/$fnme" "/dev/shm/$lt.flac" 2>&1 | tr -d '\r')
echo -n "."

# Convert FLAC to Opus via vac-enc
bt=$(/usr/bin/time -f "%e" vac-enc "-b$bitrate" "/dev/shm/$lt.flac" "/dev/shm/$lt-temp.opus" 2>&1 | tr -d '\r')
[ -f "/dev/shm/$lt.flac" ] && rm "/dev/shm/$lt.flac"
echo -n "."

# Copy metadata from input -> Opus
ct=$(/usr/bin/time -f "%e" ffmpeg -hide_banner -loglevel panic -y -i "/dev/shm/$fnme" -i "/dev/shm/$lt-temp.opus" -map 1 -c copy -map_metadata 0 "/dev/shm/$lt.opus" 2>&1 | tr -d '\r')

start_size=$(stat --printf=%s "/dev/shm/$fnme" 2>/dev/null || echo "0")
fin_size=$(stat --printf=%s "/dev/shm/$lt.opus" 2>/dev/null || echo "0")

[ -f "/dev/shm/$lt-temp.opus" ] && rm "/dev/shm/$lt-temp.opus"
[ -f "/dev/shm/$fnme" ] && rm "/dev/shm/$fnme"

if [ -f "/dev/shm/$lt.opus" ]; then
    mv "/dev/shm/$lt.opus" "$fname.opus"
else
    echo "Error: Output file not created. Check encoding steps."
    exit 1
fi
echo "."

# Calculate timings and size reduction
if [[ "$start_size" -gt 0 && "$fin_size" -gt 0 ]]; then
    tim=$(echo "$at + $bt + $ct" | bc 2>/dev/null || echo "0")
    dif=$(echo "$start_size - $fin_size" | bc 2>/dev/null || echo "0")
    perc=$(echo "scale=4; ($dif / $start_size) * 100" | bc 2>/dev/null || echo "0")
    perc="${perc%00}"
else
    echo "Error: Invalid file sizes."
    exit 1
fi

echo "Encoded \"$in\" to \"$fname.opus\" in $tim seconds ($perc% smaller)"
