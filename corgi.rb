
# Handle command-line arguments immediately.
if ARGV.size != 1
  puts "Usage: #{__FILE__} path/to/file.cg"
  exit
else
  path = ARGV.first
end
