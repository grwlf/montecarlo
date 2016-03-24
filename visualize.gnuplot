set key right center
set multiplot layout 2,1
plot 'game_log.txt' using 8 with lines title "first player wins percent"
plot 'game_log.txt' using 2 with lines title 'Q size', 'game_log.txt' using 4 with lines title 'policy size'
unset multiplot