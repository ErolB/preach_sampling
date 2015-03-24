#This script process the times files produced by time_probing.rb, produces files for avg, median and plots the results
dir, method, prob, size = ARGV
randfile = "#{dir}/times_rand_#{prob}_#{size}.out"
methodfile = "#{dir}/times_#{method}_#{prob}_#{size}.out"

def get_values(filename)
	results = {}
	open filename do |f|
		until (l = f.gets).nil?
			next if l =~ /^#/
			k, v = l.strip.split
			v = v.to_f
			results[k] ||= []
			results[k] << v
		end
	end
	results
end

rand_values = get_values randfile
method_values = get_values methodfile
open(randfile.gsub("times_", "avg_"), "w"){|f| f.puts rand_values.map{|k,v| "#{k}\t#{v.reduce(:+)/v.size}"}}
open(randfile.gsub("times_", "median_"), "w"){|f| f.puts rand_values.map{|k,v| "#{k}\t#{(v.sort[v.size/2]+v.sort[(v.size-1)/2])/2.0}"}}
open(methodfile.gsub("times_", "avg_"), "w"){|f| f.puts method_values.map{|k,v| "#{k}\t#{v.reduce(:+)/v.size}"}}
open(methodfile.gsub("times_", "median_"), "w"){|f| f.puts method_values.map{|k,v| "#{k}\t#{(v.sort[v.size/2]+v.sort[(v.size-1)/2])/2.0}"}}

puts `gnuplot -e "dir='#{dir}';method='#{method}';prob='#{prob}';size='#{size}'" time_probing.p`