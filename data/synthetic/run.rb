#This script runs time experiments for synthetic data for any of the three versions of preach
require 'benchmark'
preach, sampling_prob, edges_per_node, start_size, end_size, step, version_count = ARGV
sampling_prob = sampling_prob.to_f
edges_per_node = edges_per_node.to_i
start_size = start_size.to_i
end_size = end_size.to_i
step = step.to_i
version_count = version_count.to_i

outfile = "times_#{preach}_#{edges_per_node}_#{sampling_prob}.out"
open(outfile, "w"){}
start_size.step(end_size, step) do |size|
	1.upto(version_count) do |version|
		if preach == "preach1"
			tail = " pmc pre"
		elsif preach == "preach2"
			tail = " "
		else
			tail = " #{sampling_prob} 1000 fixwrand 10 10"
		end
		filename = "BA_#{edges_per_node}_#{size}_#{version}"
		open("#{filename}.terminals"){|f| f.read}.strip.split("\n").map{|l| l.strip.split}.each do |pair|
			cmd = "../../#{preach} #{filename}.txt node_#{pair.first}.txt node_#{pair.last}.txt #{tail}"
			puts cmd
			output = ""
			times = Benchmark.measure{output = `#{cmd} 2>&1`}
			result = (output =~ /EXCEPTION|bad_alloc/) ? "NA" : times.total
			open(outfile, "a"){|f| f.puts "#{size}\t#{version}\t#{pair.first}\t#{pair.last}\t#{result}"}
		end
	end
end
