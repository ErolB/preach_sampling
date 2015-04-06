#Uses parameter: dataset
reset
set term png size 1280,960 transparent truecolor linewidth 3 28
set datafile separator comma
set xlabel "Source-Target pairs"
set cblabel "Reachability probability"
set palette model RGB defined (0 "green", 1 "dark-green", 1 "yellow", 2 "dark-yellow", 2 "red", 3 "dark-red" )
unset xtics
unset title
unset key

set output dataset."/clustergram.png"
plot dataset.'/matrix_clustered.out' matrix rowheaders with image
set output
