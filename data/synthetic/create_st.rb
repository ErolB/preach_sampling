Dir.glob("BA_*.txt").each do |net|
	edges = open(net){|f| f.read}.strip.split("\n").map{|l| l.strip.split.first(2).map{|n| n.strip}}
	sources, targets = edges.transpose.map{|l| l.uniq}
	open(net.gsub(".txt", ".sources"), "w"){|f| f.puts (sources - targets).join("\n")}
	open(net.gsub(".txt", ".targets"), "w"){|f| f.puts (targets - sources).join("\n")}
end