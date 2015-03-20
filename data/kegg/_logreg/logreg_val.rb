#This script computes the edge probabilities of a network based on logistic distribution
#with the given parameters (free, evidence, coexpression, clustering coefficient)
param_file = ARGV.first
net_files = ARGV.last(ARGV.size-1)

def logprob values, avg_coexp
	values[1] = avg_coexp if values[1] == "NA"
	1.0 / ( 1.0 + Math.exp( [@params, [-1.0] + values.map{|v| v.to_f * -1.0}].transpose.map{|p| p.reduce(:*)}.reduce(:+) ) )
end

@params = open(param_file){|f| f.read}.strip.split.map{|p| p.to_f}
net_files.each do |file|
	net = open(file){|f| f.read}.strip.split("\n").map{|l| l.split}
	valid_coexp = net.map{|e| e[3]}.reject{|c| c == "NA"}.map{|c| c.to_f}
	avg_coexp = valid_coexp.reduce(:+)/valid_coexp.size
	open("#{file}.prob", "w"){|f| f.puts net.map{|e| "#{e.first(2).join(" ")} #{logprob e.last(3), avg_coexp}"}.join("\n")}
end