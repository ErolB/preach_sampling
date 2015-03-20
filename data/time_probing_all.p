#This script takes dir, prob, size as inputs and plots comparison of all methods (fixrand, fixwrand)
reset
#set term postscript eps enhanced
set term png size 1280,960 transparent truecolor linewidth 3 24
set title "Running time comparison for probing methods vs rand"
set xlabel "Number of probes"
set ylabel "Time (s)"
set key top left
set logscale y

set output sprintf("%s/avg_all_%s_%s.png", dir, prob, size)
plot for [method in "fixrand fixwrand"] sprintf("%s/times_%s_%s_%s.out", dir, method, prob, size) using 1:2 with points title method,\
	 for [method in "fixrand fixwrand"] sprintf("%s/avg_%s_%s_%s.out", dir, method, prob, size) using 1:2 with linespoints title sprintf("%s average", method),\
	 sprintf("%s/avg_rand_%s_%s.out", dir, prob, size) using 1:2 with lines title "rand average"
set output


set output sprintf("%s/median_all_%s_%s.png", dir, prob, size)
plot for [method in "fixrand fixwrand"] sprintf("%s/times_%s_%s_%s.out", dir, method, prob, size) using 1:2 with points title method,\
	 for [method in "fixrand fixwrand"] sprintf("%s/median_%s_%s_%s.out", dir, method, prob, size) using 1:2 with linespoints title sprintf("%s median", method),\
	 sprintf("%s/median_rand_%s_%s.out", dir, prob, size) using 1:2 with lines title "rand median"
set output