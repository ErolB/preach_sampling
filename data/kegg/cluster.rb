#This script uses rowlabels_clustered.out and columnlabels_clustered.out along with matrix.out, rowlables.out and columnlabels.out
#To produce matrix_clustered.out for easy plotting with gnuplot
dataset = ARGV.first

row_labels = open("#{dataset}/rowlabels.out"){|f| f.read}.gsub(/\{|\}|'/, "").strip.split
column_labels = open("#{dataset}/columnlabels.out"){|f| f.read}.gsub(/\{|\}|'/, "").strip.split
row_order = open("#{dataset}/rowlabels_clustered.out"){|f| f.read}.strip.split.map{|i| i.strip.to_i}
column_order = open("#{dataset}/columnlabels_clustered.out"){|f| f.read}.strip.split.map{|i| i.strip.to_i}
matrix = open("#{dataset}/matrix.out"){|f| f.read}.gsub(/\[|\]/, "").strip.split("\n").map{|l| l.strip.split}

matrix = row_order.map{|i| matrix[i-1]}.transpose #1 based
matrix = column_order.map{|i| matrix[i-1]}.transpose #1 based

open("#{dataset}/columnlabels_ordered.out", "w"){|f| f.puts column_order.map{|i| column_labels[i-1]}.join " "}
open("#{dataset}/matrix_clustered.out", "w") do |f|
	f.puts row_order.map.with_index{|i, j| "#{row_labels[i-1]}, #{matrix[j].join ", "}"}
end
