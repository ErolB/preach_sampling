#This script works on the profiles.out file, filters aout all-zero st pairs, and outputs the filtered matrix, row labels and column labels
dataset = ARGV[0]

lines = open("#{dataset}/profiles.out"){|f| f.read}.strip.split("\n")
pairs = lines.first.strip.split
rows = lines.last(lines.size-1).map{|l| l.strip.split}
types = rows.map{|r| r.first}
values = rows.map{|r| r.last(r.size-1).map{|v| v.to_f}}

values = values.transpose
filtered_pairs = []
filtered_values = []
pairs.each_with_index do |pair, i|
	unless values[i].map{|v| v.zero?}.all?
		filtered_pairs << pair
		filtered_values << values[i]
	end
end
filtered_values = filtered_values.transpose

open("#{dataset}/matrix.out", "w") do |f|
	f.puts "[ #{filtered_values.map{|row| row.join " "}.join "\n"} ]"
end
open("#{dataset}/rowlabels.out", "w") do |f|
	f.puts "{ #{types.map{|t| "'#{t}'"}.join " "} }"
end
open("#{dataset}/columnlabels.out", "w") do |f|
	f.puts "{ #{filtered_pairs.map{|p| "'#{p}'"}.join " "} }"
end