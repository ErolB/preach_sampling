#This script takes dir, method, prob, size as inputs
reset
#set term postscript eps enhanced
set term png size 1280,960 transparent truecolor linewidth 3 24
set title sprintf("Running time for %s approach, against rand approach", method)
set xlabel "Number of probes"
set ylabel "Time (s)"

set output sprintf("%s/avg_%s_%s_%s.png", dir, method, prob, size)
plot sprintf("%s/times_%s_%s_%s.out", dir, method, prob, size) using 1:2 with points title method,\
	 sprintf("%s/avg_%s_%s_%s.out", dir, method, prob, size) using 1:2 with linespoints title sprintf("%s average", method),\
	 sprintf("%s/avg_rand_%s_%s.out", dir, prob, size) using 1:2 with lines title "rand average"
set output


set output sprintf("%s/median_%s_%s_%s.png", dir, method, prob, size)
plot sprintf("%s/times_%s_%s_%s.out", dir, method, prob, size) using 1:2 with points title method,\
	 sprintf("%s/median_%s_%s_%s.out", dir, method, prob, size) using 1:2 with linespoints title sprintf("%s median", method),\
	 sprintf("%s/median_rand_%s_%s.out", dir, prob, size) using 1:2 with lines title "rand median"
set output