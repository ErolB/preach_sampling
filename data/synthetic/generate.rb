#This script generates synthetic networks using Barabasi-Albert model.
#It also generates one file per node just for the sake of trying all possible s-t pairs 
edges_per_node, min_size, steps, step_size, versions = ARGV.map{|a| a.to_i}

@connected = {}
@edges = []
@chances = []
@reachable = []

def connect node1, node2
	@connected["#{node1}-#{node2}"] = true
	@connected["#{node2}-#{node1}"] = true
	@chances << node1 << node2
	if rand > 0.5
		@edges << [node1, node2, rand]
		@reachable = @reachable + @reachable.select{|p| p.last == node1}.map{|p| [p.first, node2]}
		@reachable << [node1, node2]
	else
		@edges << [node2, node1, rand]
		@reachable = @reachable + @reachable.select{|p| p.last == node2}.map{|p| [p.first, node1]}
		@reachable << [node2, node1]
	end
	@reachable.uniq!
end

def output filename
	open "#{filename}.txt", "w" do |f|
		@edges.each{|e| f.puts e.join(" ")}
	end
	open "#{filename}.terminals", "w" do |f|
		@reachable.each{|r| f.puts r.join(" ")}
	end
end

1.upto versions do |version|
	@connected = {}
	@edges = []
	@chances = []
	@reachable = []
	steps.times do |step|
		1.upto step_size do |i|
			node1 = step * step_size + i
			if node1 <= edges_per_node + 1
				1.upto node1-1 do |node2|
					connect node1, node2
				end
			else
				edges_per_node.times do
					node2 = @chances[(rand*@chances.size).floor]
					while node2 == node1 or @connected["#{node1}#{node2}"]
						node2 = @chances[(rand*@chances.size).floor]
					end
					connect node1, node2
				end
			end
		end
		size = (step+1)*step_size
		output "BA_#{edges_per_node}_#{size}_#{version}" if size >= min_size
	end
end

1.upto(min_size + steps*step_size - step_size){|i| open("node_#{i}.txt", "w"){|f| f.puts i}}