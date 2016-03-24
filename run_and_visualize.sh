#!/usr/bin/env bash
set -e
./montecarlo $1 $2 > game_log.txt
gnuplot -p visualize.gnuplot
