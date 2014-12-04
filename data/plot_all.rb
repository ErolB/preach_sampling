dataset = ARGV[0]

values = Dir.glob("#{dataset}/results_*.out").map{|s| open(s){|f| f.read}.split("\n").map{|v| v.to_f}}.flatten
Dir.glob("#{dataset}/results_*.out").map{|s| s.split("results_").last.split(".out").first.split("_")}.each do |pair|
	puts pair.join(", ")
	#values = open("#{dataset}/results_#{pair[0]}_#{pair[1]}.out"){|f| f.read}.split("\n").map{|v| v.to_f}
	`gnuplot -e "dataset='#{dataset}';sample_size=#{pair[1]};sampling_prob=#{pair[0]};binwidth=#{(values.max-values.min)/50.0};minx=#{values.min};maxx=#{values.max}" plot.p`
end