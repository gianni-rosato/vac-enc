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

# Convert input to 16-bit WAV
at=$(/usr/bin/time -f "%e" ffmpeg -hide_banner -loglevel panic -y -i "/dev/shm/$fnme" -c:a pcm_s16le "/dev/shm/$lt.wav" 2>&1)
echo -n "."

# Convert WAV to Opus via vac
bt=$(/usr/bin/time -f "%e" vac-enc "/dev/shm/$lt.wav" "/dev/shm/$lt-temp.opus" "$bitrate" 2>&1)
rm "/dev/shm/$lt.wav"
echo -n "."

# Copy metadata from input -> Opus
ct=$(/usr/bin/time -f "%e" ffmpeg -hide_banner -loglevel panic -y -i "/dev/shm/$fnme" -i "/dev/shm/$lt-temp.opus" -map 1 -c copy -map_metadata 0 "/dev/shm/$lt.opus" 2>&1)

start_size=$(stat --printf=%s "/dev/shm/$fnme" 2>&1)
fin_size=$(stat --printf=%s "/dev/shm/$lt.opus" 2>&1)

rm "/dev/shm/$lt-temp.opus"
rm "/dev/shm/$fnme"
mv "/dev/shm/$lt.opus" "$fname.opus"
echo "."

tim=$(echo "$at + $bt + $ct" | bc 2>&1)

dif=$(echo "$start_size - $fin_size" | bc)
perc=$(echo "scale=4; ($dif / $start_size) * 100" | bc)
perc="${perc%00}"

echo "Encoded \""$in"\" to \""$fname.opus"\" in "$tim" seconds ("$perc"% smaller)"
