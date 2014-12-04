#This file plots one figure for one combination of dataset, sample_size, sampling_prob, and binwidth
reset
#set term postscript eps enhanced
set term png size 1280,960 transparent truecolor linewidth 3 24
set title sprintf("Sampling %d%%, %d samples", sampling_prob*100, sample_size)
set xlabel "Sample Reachability (Rs)"
set ylabel "P(Rs)"

#binwidth=0.005
bin(x,width)=width*floor(x/width) #- width/2.0
set boxwidth binwidth

#set xrange [-0.1:1.1]
set xrange [minx-5*binwidth:maxx+5*binwidth]
set yrange [0:1.1]
set output sprintf("%s/histogram_%.1f_%d.png", dataset, sampling_prob, sample_size)
plot sprintf("%s/results_%.1f_%d.out", dataset, sampling_prob, sample_size) using (bin($1,binwidth)):(1.0/sample_size) smooth freq with boxes t "", \
	 sprintf("%s/mean_%.1f_%d.out", dataset, sampling_prob, sample_size) u 1:2 w l lw 0.5 t "",\
	 sprintf("%s/dev_%.1f_%d.out", dataset, sampling_prob, sample_size) u 1:2:3:4 with xerrorbars t ""
set output