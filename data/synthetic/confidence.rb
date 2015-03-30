#This script runs BA_2_20_1 1000 times, computes confidence intervals,
#and counts the empirical percent of time the true result lies inside the interval
#parameters: 0.7, 1000, fixwrand, 5, 10

size, sampling_prob = ARGV
t = 1.96234147 #t 0.05,999
outfile = "confidence_2_#{size}_#{sampling_prob}.out"
logfile = "confidence_2_#{size}_#{sampling_prob}.log"

open(outfile, "w"){}
net = "BA_2_#{size}_1"
open("#{net}.terminals"){|f| f.read}.strip.split("\n").map{|l| l.strip.split}.each do |pair|
	cmd = "../../preach1 #{net}.txt node_#{pair.first}.txt node_#{pair.last}.txt pmc pre"
	puts cmd
	output = `#{cmd} 2>&1`
	if output =~ /EXCEPTION|bad_alloc/
		puts "EXCEPTION"
		next
	else
		ref = output.split(">>").last.strip.to_f
		open(logfile, "a"){|f| f.puts "========================================\n#{pair.first}\t#{pair.last}\t#{ref}\n--------------"}
		cmd = "../../preach #{net}.txt node_#{pair.first}.txt node_#{pair.last}.txt #{sampling_prob} 1000 fixwrand 5 10"
		success = 0.0
		puts cmd
		1000.times do
			output = `#{cmd} 2>&1`
			sample = output.split("\n").map{|l| l.strip}.reject{|l| l =~ /^#/}.map{|l| l.split.map{|p| p.strip}[2].to_f}
			avg = sample.reduce(:+)/sample.size
			std = Math.sqrt(sample.map{|x| (x-avg)*(x-avg)}.reduce(:+)/(sample.size-1))
			err = t*std/Math.sqrt(sample.size)
			ci = [avg-err, avg+err]
			success += 1 if ref > ci.first and ref < ci.last
			print "."
			open(logfile, "a"){|f| f.puts ci.to_s}
		end
		open(outfile, "a"){|f| f.puts "#{pair.first}\t#{pair.last}\t#{success/1000}"}
		puts success/1000
	end
end
