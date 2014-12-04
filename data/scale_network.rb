require 'fileutils'
dataset, factor = ARGV
factor = factor.to_f

FileUtils.mkdir "#{dataset}/#{dataset}_#{factor}"
open("#{dataset}/#{dataset}_#{factor}/network.txt", "w"){ |fout| fout.puts open("#{dataset}/network.txt"){|f| f.read}.strip.split("\n").map{|l| (l.split.first(2) << l.split.last.to_f*factor).join("\t")}.join("\n")}
FileUtils.copy "#{dataset}/sources.txt", "#{dataset}/#{dataset}_#{factor}/sources.txt"
FileUtils.copy "#{dataset}/targets.txt", "#{dataset}/#{dataset}_#{factor}/targets.txt"
