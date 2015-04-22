#Uses parameter: input, confidence
reset
set term png size 1280,960 transparent truecolor linewidth 3 28

stats input

set xlabel "Source - Target pairs"
set ylabel "Empirical confidence"
set yrange [confidence-0.4:confidence+0.1]
set xrange [0:STATS_records]
#unset xtics

set output input.".png"
plot input u 3 w p t "", confidence t sprintf("%.2f", confidence)
set output