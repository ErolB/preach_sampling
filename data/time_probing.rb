#This script times a given probing method for several probe sizes against the single time consumed by the rand method
dataset, output, sample_prob, sample_size, method, probe_start, probe_end, probe_step, repeat = ARGV
probe_start = probe_start.to_i
probe_end = probe_end.to_i
probe_step = probe_step.to_i
repeat = repeat.to_i

open("#{output}/times_#{method}_#{sample_prob}_#{sample_size}.out", "w"){|f| f.puts "#probe_size\ttime"}
probe_start.step probe_end, probe_step do |probe_size|
	time = 0.0
	repeat.times do
		puts "../preach #{dataset}/network.txt #{dataset}/sources.txt #{dataset}/targets.txt #{sample_prob} #{sample_size} #{method} #{probe_size}"
		time = time + `(time -p ../preach #{dataset}/network.txt #{dataset}/sources.txt #{dataset}/targets.txt #{sample_prob} #{sample_size} #{method} #{probe_size}) 2>&1 | grep real | tr -d '\n' | cut -d" " -f2`.strip
	end
	
	open("#{output}/times_#{method}_#{sample_prob}_#{sample_size}.out", "a"){|f| f.puts "#{probe_size}\t\t#{time/repeat}"}
end

time = 0.0
repeat.times do
	puts "../preach #{dataset}/network.txt #{dataset}/sources.txt #{dataset}/targets.txt #{sample_prob} #{sample_size} rand 10"
	time = time + `(time -p ../preach #{dataset}/network.txt #{dataset}/sources.txt #{dataset}/targets.txt #{sample_prob} #{sample_size} rand 10) 2>&1 | grep real | tr -d '\n' | cut -d" " -f2`.strip	
end
open("#{output}/times_rand_#{sample_prob}_#{sample_size}.out", "w"){|f| f.puts "#{probe_start}\t\t#{time}\n#{probe_end}\t\t#{time/repeat}"}