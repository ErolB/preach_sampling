#This script reads all times_*.out and outputs a corresponding times_*.summary with average times for every size
Dir.glob("times_preach*.out").each do |file|
	times = {}
	records = open(file){|f| f.read}.strip.split("\n").map{|l| l.strip.split}
	records.each do |r|
		times[r.first.to_i] ||= []
		times[r.first.to_i] << r.last
	end
	open(file.gsub(".out", ".summary"), "w"){|f| f.puts times.reject{|k,v| v.include? "NA"}.map{|k,v| "#{k} #{v.map{|f| f.to_f}.reduce(:+)/v.size}"}.join "\n"}
end