#This script produces a matlab file for estimating the logistic distribution parameters
#for edge probabilities in multiple networks
#Input: evidence file and gold standard file (both in gene names),
#		gene expression file (gene names as rows, samples as columns),
#		gene id to name translation file (or you can pass "none",
#		The network file(s)
NEGATIVE_RATIO = 0.05 #defines how many negative samples to generate - ratio to the number of present edges
evidence_file, gold_file, ge_file, @dictionary_file = ARGV[0..3]
net_files = ARGV[4..ARGV.length-1]

def translate gene
	if @dictionary_file == "none"
		return gene
	else
		return @dictionary[gene] || []
	end
end

def permute n, k
	return 1 if k.zero?
	k.times.map{|i| n-i}.reduce :*
end

def choose n, k
	permute(n, k) / permute(k, k)
end

#mutual clustering coefficient (hypergeometric) of two nodes using their neighborhood
def mcc u_hood, v_hood, net_size
	-1.0 * Math.log( (u_hood & v_hood).size.upto([u_hood.size, v_hood.size].min).map{|i| 1.0*choose(u_hood.size, i)*choose(net_size-u_hood.size, v_hood.size-i)/choose(net_size, v_hood.size)}.reduce(:+) )
end

#pearson correlation coefficient
def pcc x, y
	xbar, ybar = x.reduce(:+)/x.size, y.reduce(:+)/y.size
	numerator = [x.map{|i| i-xbar}, y.map{|i| i-ybar}].transpose.map{|pair| pair.reduce(:*)}.reduce(:+)
	denomerator = Math.sqrt(x.map{|i| (i-xbar)*(i-xbar)}.reduce(:+)) * Math.sqrt(y.map{|i| (i-ybar)*(i-ybar)}.reduce(:+))
	corr = (numerator/denomerator)
	raise "pearson value out of bounds: #{corr}" if corr > 1.0000001 or corr < -1.0000001
	corr
end

#translates an edge using @dictionary
def translate_edge edge
	edge.map{|v| v.map{|g| translate g}.reduce(:+)}
end

#average evidence of an edge, using the evidence file
def evidence edge
	edge = translate_edge edge
	confidences = @evidence.select{|v| ( (v[0] & edge[0]).any? and (v[1] & edge[1]).any? ) or ( (v[0] & edge[1]).any? and (v[1] & edge[0]).any? )}.map{|v| v.last}
	return 0.0 if confidences.empty? 
	confidences.flatten.reduce(:+).to_f / confidences.size
end

#gene coexpression using the expression file
def coexpression edge
	edge = translate_edge edge
	s_expr, t_expr = edge.map{|v| v.map{|g| @expression[g]}.reject{|g| g.nil?}.transpose.map{|col| col.reduce(:+)/col.size}}
	return "NA" if s_expr.empty? or t_expr.empty?
	pcc s_expr, t_expr
end

#checks if an edge is present in the gold standard
def positive? edge
	edge = translate_edge edge
	@gold.each do |e|
		return true if ((edge.first & e.first).any? and (edge.last & e.last).any?) or ((edge.first & e.last).any? and (edge.last & e.first).any?)
	end
	false
end

#small world clustering coefficient of an edge
def small_world edge, net
	s_hood, t_hood = edge.map{|v| net.select{|e| e.first == v}.map{|e| e.last} + net.select{|e| e.last == v}.map{|e| e.first}}
	mcc s_hood, t_hood, net.reduce(:+).uniq.size
end

#select random negative samples
def random_edges net
	samples = []
	nodes = net.reduce(:+).uniq
	(NEGATIVE_RATIO*net.size).to_i.times do
		s = nodes[(rand * nodes.size).floor]
		t = nodes[(rand * nodes.size).floor]
		while (s == t or net.include? [s, t] or samples.include? [s, t] or coexpression([s, t]) == "NA")
			s = nodes[(rand * nodes.size).floor]
			t = nodes[(rand * nodes.size).floor]
		end
		samples << [s, t]
	end
	samples
end

#writes an output metrics file
def write_metrics edges, filename
	open filename, "w" do |f|
		edges.each do |edge|
			f.puts "#{edge.first(2).map{|v| v.join("|")}.join(" ")} #{edge.last.join(" ")}"
		end
	end
end

#loading input files
puts "loading input files"
print "."
@gold = open(gold_file){|f| f.read}.split("\n").map{|l| l.split(/\s/).map{|p| p.strip.split("|")}}
print "."
@dictionary = open(@dictionary_file){|f| f.read}.strip.split("\n").map{|l| {l.strip.split.first => l.strip.split.last.split("|")}}.reduce(:merge) unless @dictionary_file == "none"
print "."
nets = net_files.map{|nf| open(nf){|f| f.read}.strip.split("\n").map{|l| l.strip.split.map{|g| g.split("|")}}}
print "."
@evidence = open(evidence_file){|f| f.read}.strip.split("\n").map{|l| l.strip.split.map{|g| g.split("|")}}
print "."
@expression = open(ge_file){|f| f.read}.strip.split("\n").map{|l|
	parts = l.strip.split
	{parts.first => parts.last(parts.size-1).map{|i| i.to_f}}
}.reduce(:merge)
puts "done"

#calculating the 3 metrics for every edge in every network
#also collecting positive and negative samples
positive = []
negative = []
nets.each_with_index do |net, i|
	print net_files[i]
	neo_net = Marshal.load(Marshal.dump net)
	net.each do |edge|
		print "."
		positive_edge = positive? edge
		edge << [evidence(edge), coexpression(edge), small_world(edge, neo_net)]
		positive << edge if positive_edge and !edge.last.include? "NA"
	end
	net_negative = random_edges neo_net
	net_negative.each do |edge|
		print "-"
		edge << [evidence(edge), coexpression(edge), small_world(edge, neo_net)]
	end
	negative = negative + net_negative
	puts
end

#outputting intermediate data files
write_metrics positive, "#{ge_file.split(/\/|\\/).last}.pos"
write_metrics negative, "#{ge_file.split(/\/|\\/).last}.neg"
nets.size.times do |i|
	write_metrics nets[i], "#{net_files[i]}.#{ge_file.split(/\/|\\/).last}.obs"
end

#outputting matlab file
open ge_file.split(/\/|\\/).last.gsub(/(\.\w+)+/, ".m").gsub("-", "_"), "w" do |f|
	f.puts "Obs = [#{(positive + negative).map{|e| e.last.join " "}.join "\n"}];"
	f.puts "Cat = [#{positive.map{1}.join " "} #{negative.map{2}.join " "}]';"
	f.puts "B = mnrfit(Obs, Cat)"
	f.puts "fid = fopen('#{ge_file.split(/\/|\\/).last}.param', 'w');"
	f.puts "fprintf(fid, '%f\\n', B);"
	f.puts "fclose(fid);"
end










