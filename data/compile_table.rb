#This script compiles a results table in output_file from the given input_files
output_file = ARGV[0]
input_files = ARGV[1..ARGV.size-1]

open output_file, "w" do |fout|
	input_files.each do |file|
		open file do |f|
			f.read.strip.split("\n").each_with_index do |line, i|
				fout.puts "#{i}\t#{file.split("results_").last.split("_").first}\t#{line}"
			end
		end
	end
end