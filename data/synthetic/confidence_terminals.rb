#This script runs BA_2_20_1 10000 times, computes confidence intervals,
#and counts the empirical percent of time the true result lies inside the interval
#It does if for a specific given s-t pair (one result point)

size, sampling_prob, source, target = ARGV
t = 1.96234147 #t 0.05,999
outfile = "confidence_2_#{size}_#{sampling_prob}_#{source}_#{target}.out"
logfile = "confidence_2_#{size}_#{sampling_prob}_#{source}_#{target}.log"
open(logfile, "w"){}

net = "BA_2_#{size}_1"
cmd = "../../preach1 #{net}.txt node_#{source}.txt node_#{target}.txt pmc pre"
puts cmd
output = `#{cmd} 2>&1`
if output =~ /EXCEPTION|bad_alloc/
	puts "EXCEPTION"
	next
else
	ref = output.split(">>").last.strip.to_f
	open(logfile, "a"){|f| f.puts "========================================\n#{source}\t#{target}\t#{ref}\n--------------"}
	cmd = "../../preach #{net}.txt node_#{source}.txt node_#{target}.txt #{sampling_prob} 1000 fixwrand 5 10"
	success = 0.0
	puts cmd
	10000.times do
		output = `#{cmd} 2>&1`
		sample = output.split("\n").map{|l| l.strip}.reject{|l| l =~ /^#/}.map{|l| l.split.map{|p| p.strip}[2].to_f}
		#avg = sample.reduce(:+)/sample.size
		avg = output.split("#>>result =").last.strip.to_f
		std = Math.sqrt(sample.map{|x| (x-avg)*(x-avg)}.reduce(:+)/(sample.size-1))
		err = t*std/Math.sqrt(sample.size)
		ci = [avg-err, avg+err]
		success += 1 if ref >= ci.first and ref <= ci.last
		print "."
		open(logfile, "a"){|f| f.puts ci.to_s}
	end
	open(outfile, "w"){|f| f.puts "#{source}\t#{target}\t#{success/10000}"}
	puts success/1000
end
