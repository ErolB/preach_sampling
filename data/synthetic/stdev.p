reset 
set term png size 1280,960 transparent truecolor linewidth 3 28
set xlabel "Sampling rate"
set ylabel "Standard deviation/Confidence interval"
set logscale y

do for [file in system("ls stdev_*.summary")] {
	set output system("echo ".file." | sed s=.summary=.png=g")
	plot file u 1:2 w lp t "Std. Deviation", file u 1:3 w lp t "Confidence interval"
	set output
}
