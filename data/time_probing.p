#This script takes method, method_file, rand_file, output_file as inputs
reset
#set term postscript eps enhanced
set term png size 1280,960 transparent truecolor linewidth 3 24
set title sprintf("Running time for %s approach, against rand approach", method)
set xlabel "Number of probes"
set ylabel "Time (s)"

set output output_file
plot method_file using 1:2 with linespoints title method,\
	 rand_file using 1:2 with lines title "rand"
set output