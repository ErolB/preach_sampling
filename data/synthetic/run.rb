edges_per_node, init_size, steps, step_size, version = ARGV.map{|a| a.to_i}

open("times_BA_#{edges_per_node}_#{version}.out", "w"){}
steps.times do |step|
	size = init_size + step * step_size
	puts "../eparc BA_#{edges_per_node}_#{size}_#{version}.txt BA_#{edges_per_node}_#{size}_#{version}_coexp.txt BA_#{edges_per_node}_#{version}.prob"
	time = `(time -p ../eparc BA_#{edges_per_node}_#{size}_#{version}.txt BA_#{edges_per_node}_#{size}_#{version}_coexp.txt BA_#{edges_per_node}_#{version}.prob) 2>&1 | grep real | tr -d '\n' | cut -d" " -f2`.strip
	open("times_BA_#{edges_per_node}_#{version}.out", "a"){|f| f.puts "#{size}  #{time}"}
end