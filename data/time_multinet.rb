#This script times a given probing method for several probe sizes against the single time consumed by the rand method
dataset, output, sample_prob, sample_size, method, probe_start, probe_end, probe_step, probe_repeats, run_repeats = ARGV
probe_start = probe_start.to_i
probe_end = probe_end.to_i
probe_step = probe_step.to_i
run_repeats = run_repeats.to_i

open("#{output}/times_#{method}_#{sample_prob}_#{sample_size}.out", "w"){|f| f.puts "#probe_size\ttime"}
probe_start.step probe_end, probe_step do |probe_size|
	time = 0.0
	Dir.glob("#{dataset}/*.net").each do |net|
		run_repeats.times do
			puts "../preach #{net} #{dataset}/sources.txt #{dataset}/targets.txt #{sample_prob} #{sample_size} #{method} #{probe_size} #{probe_repeats}"
			time = `(time -p ../preach #{net} #{dataset}/sources.txt #{dataset}/targets.txt #{sample_prob} #{sample_size} #{method} #{probe_size} #{probe_repeats}) 2>&1 | grep real | tr -d '\n' | cut -d" " -f2`.strip.to_f
			open("#{output}/times_#{method}_#{sample_prob}_#{sample_size}.out", "a"){|f| f.puts "#{probe_size}\t\t#{time}"}
		end
	end
end

open("#{output}/times_rand_#{sample_prob}_#{sample_size}.out", "w"){|f| f.puts "#probe_size\ttime"}
time = 0.0
Dir.glob("#{dataset}/*.net").each do |net|
	run_repeats.times do
		puts "../preach #{net} #{dataset}/sources.txt #{dataset}/targets.txt #{sample_prob} #{sample_size} rand 10 10"
		time = `(time -p ../preach #{net} #{dataset}/sources.txt #{dataset}/targets.txt #{sample_prob} #{sample_size} rand 10 10) 2>&1 | grep real | tr -d '\n' | cut -d" " -f2`.strip.to_f
		open("#{output}/times_rand_#{sample_prob}_#{sample_size}.out", "a"){|f| f.puts "#{probe_start}\t\t#{time}\n#{probe_end}\t\t#{time}"}
	end
end