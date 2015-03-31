dataset, sampling_prob = ARGV

outfile = "#{dataset}/profiles.out"
sources = Dir.glob("#{dataset}/source_*.txt").map{|path| path.split("_").last.split(".txt").first.to_i}.sort
targets = Dir.glob("#{dataset}/target_*.txt").map{|path| path.split("_").last.split(".txt").first.to_i}.sort
open(outfile, "w"){|f| f.puts "#{sprintf "%-12s", " "}#{sources.map{|s| targets.map{|t| sprintf "%-12s", "#{s},#{t}"}.join}.join}"}

Dir.glob("#{dataset}/*.net").each do |net|
	open(outfile, "a"){|f| f.print(sprintf "%-12s", net.split("/").last.split(".net").first)}
	sources.each do |s|
		targets.each do |t|
			cmd = "../../preach #{net} #{dataset}/source_#{s}.txt #{dataset}/target_#{t}.txt #{sampling_prob} 1000 fixwrand 5 10"
			puts cmd
			output = `#{cmd} 2>&1`
			if output =~ /EXCEPTION|bad_alloc/
				open(outfile, "a"){|f| f.print(sprintf "%-12s", "EXCEPTION")}
				next
			else
				open(outfile, "a"){|f| f.print(sprintf "%-12f", output.split(">>result =").last.strip.to_f.round(6))}
			end
		end
	end
	open(outfile, "a"){|f| f.puts}
end