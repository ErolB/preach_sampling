#This script works on output_epn_size_*_*.out and produces summary files for stdev and CI
epn, size = ARGV
t = 1.96234147 #t 0.05,999

values = {}
Dir.glob("output_#{epn}_#{size}_*_*.out").each do |file|
	prob = file.split("_")[3].to_f
	values[prob] ||= []
	values[prob] << open(file){|f| f.read}.strip.split("\n").map{|l| l.strip.split[2].to_f}
end

open "stdev_#{epn}_#{size}.summary", "w" do |f|
	f.puts "#{sprintf "%-8s", "#prob"}#{sprintf "%-12s", "stdev"}#{sprintf "%-12s", "CI"}"
	values.keys.sort.each do |prob|
		avgs = values[prob].map{|list| list.reduce(:+)/list.size}
		stdevs = values[prob].map.with_index{|list, i| Math.sqrt(list.map{|x| (x-avgs[i])*(x-avgs[i])}.reduce(:+)/(list.size-1))}
		cis = values[prob].map.with_index{|list, i| 2.0 * t*stdevs[i]/Math.sqrt(list.size)}
		f.puts "#{sprintf "%-8s", prob.to_s}#{sprintf "%-12f", (stdevs.reduce(:+)/stdevs.size).round(6)}#{sprintf "%-12f", (cis.reduce(:+)/cis.size).round(6)}"
	end
end