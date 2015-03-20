require 'net/http'
require 'uri'

Dir.glob("_raw/GSM*.cel.gz").map{|f| f.split("/").last.split("_").first}.each do |gsm|
	print "."
	page = Net::HTTP.get(URI.parse("http://www.ncbi.nlm.nih.gov/geo/query/acc.cgi?acc=#{gsm}"))
	type = page.split("<tr valign=\"top\"><td nowrap>Title</td>\n<td style=\"text-align: justify\">").last.strip.split(":").first.strip
	page = Net::HTTP.get(URI.parse("http://www.ncbi.nlm.nih.gov/geo/query/acc.cgi?view=data&acc=#{gsm}"))
	text = page.split("VALUE</strong>").last.strip.split("<br>").first.strip
	open("#{type}/#{gsm}.CEL.tab", "w"){|f| f.puts text}
end