#!/usr/bin/env ruby

# Handle command-line arguments immediately.
if ARGV.size != 1
  puts "Usage: #{__FILE__} path/to/file.cg"
  exit
else
  path = ARGV.first
  raise "File doesn't exist!" unless File.file?(path)
end

# Reading the entire file into memory is not a good idea,
# so we'll read each character from a stream instead.
file = File.open(path)
chars = file.each_char

def consume(char, stream)
  if stream.peek == char
    stream.next
  else
    raise "expected a #{char}, got a #{stream.peek}"
  end
end

SYMBOLS = %w( ()[]{} +-*/% ., ~!|& ^ ).join

def token(type, data = '')
  [type, data]
end

def read_word(first, enum)
  string = first
  until enum.peek =~ /\s/ || SYMBOLS.include?(enum.peek)
    string += enum.next
  end
  string
end

def read_token(enum)
  char = enum.next
  if char == '/'
    consume('/', enum)
    token(:comment)
  elsif SYMBOLS.include? char
    token(:symbol, char)
  elsif char == '"'

    # Stop only when we find a matching ", not
    # preceded by a backslash.
    # Strings may not contain newlines either.
    string = '' + char

    while (string_char = enum.next)
      if string_char == "\\"
        raise 'String escape sequences are currenty unsupported.'
      elsif string_char == '"'
        break
      else
        string += string_char
      end
    end

    token(:string, string)
  elsif char =~ /[0-9]/
    string = read_word(char, enum)
    token(:number, string)
  elsif char =~ /[A-Za-z_]/
    string = read_word(char, enum)
    if %w(function).include? string

      # Instead of making a token of type "keyword"
      # with the appropriate keyword as data, this is
      # a simpler approach. It also makes the parser
      # code simpler, since in most cases we're
      # looking for a specific keyword, rather than any
      # token of this type.
      token(string.to_sym)
    else
      token(:id, string)
    end
  elsif char == "\n"
    token(:newline)
  else
    read_token(enum)
  end
end

begin
  tokens = []
  loop { tokens << read_token(chars) }
  puts tokens.inspect
rescue
end
