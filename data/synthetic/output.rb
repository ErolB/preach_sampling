#This script runs preach for BA 2 with specific ranges of size and prob, and writes the outputs in files of matching names
versions, size_start, size_end, size_step = ARGV[0..3].map{|i| i.to_i}
prob_start, prob_end, prob_step = ARGV[4..6].map{|f| f.to_f}

size_start.step(size_end, size_step) do |size|
	prob_start.step(prob_end, prob_step) do |prob|
		1.upto(versions) do |version|
			net = "BA_2_#{size}_#{version}.txt"
			cmd = "../../preach #{net}.txt #{net}.sources #{net}.targets #{prob} 1000 fixwrand 5 10"
			puts cmd
			output = `#{cmd} 2>&1`
			open("output_2_#{size}_#{prob}_#{version}.out", "w"){|f| f.write output}
		end
	end
end