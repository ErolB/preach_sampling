#This script takes as parameters: infiles (space separated filenames), logscale (0 or 1)

reset 
set term png size 1280,960 transparent truecolor linewidth 3 28
set xlabel "Number of nodes"
set ylabel "Average running time [seconds]"
if (logscale > 0) set logscale y
set output "times.png"
plot for [file in infiles] file u 1:2 w lp t system("echo ".file." | sed s=_=\\\\_=g")