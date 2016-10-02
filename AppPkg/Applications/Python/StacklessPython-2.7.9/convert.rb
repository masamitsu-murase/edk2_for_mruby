
require("zlib")

def get_file_data
  list = []

  Dir.glob("**/*.py").to_a.sort.each do |filename|
    next if filename.start_with?("test/")

    list.push({
      filename: filename,
      data: File.open(filename, "r", &:read).b + "\0".b
    })
  end

  return list
end

def output_list(list)
  File.open("Modules/embeddedimport_data.c", "w") do |file|
    file.puts '#include <stddef.h>'

    file.puts

    file.puts "char embeddedimporter_filename[] = {"
    list.each do |item|
      file.puts "  // " + item[:filename]
      item[:filename].each_byte.each_slice(16) do |bytes|
        file.puts("  " + bytes.map{ |i| "0x" + i.to_s(16).rjust(2, '0') }.join(",") + ",")
      end
      file.puts("  0x00,")
    end
    file.puts("  0x00")
    file.puts("};")

    file.puts

    data = list.map{ |i| i[:data] }.reduce(:+)

    file.puts "const size_t embeddedimporter_raw_data_size = " + data.bytesize.to_s + ";"

    file.puts

    compressed = Zlib.deflate(data, Zlib::BEST_COMPRESSION)
    file.puts "const unsigned char embeddedimporter_raw_data_compressed[] = {"
    compressed.each_byte.each_slice(16) do |bytes|
      file.puts("  " + bytes.map{ |i| "0x" + i.to_s(16).rjust(2, '0') }.join(",") + ",")
    end
    file.puts "};"

    file.puts

    file.puts "const size_t embeddedimporter_raw_data_compressed_size = " + compressed.bytesize.to_s + ";"

    file.puts

    offset = 0
    file.puts "const size_t embeddedimporter_data_offset[] = {"
    list.each do |item|
      file.puts "  #{offset},  // #{item[:filename]}"
      offset += item[:data].bytesize
    end
    file.puts "};"
  end
end

Dir.chdir(__dir__) do
  list = nil

  Dir.chdir("PyMod-2.7.9/Lib") do
    list = get_file_data
  end

  Dir.chdir("Lib") do
    get_file_data.each do |item|
      dup_item = list.find{ |i| i[:filename] == item[:filename] }
      list.push(item) unless dup_item

      if dup_item && dup_item[:data] == File.open(item[:filename], "r", &:read).b + "\0".b
        puts("Duplicated: #{item[:filename]}")
        Dir.chdir("../PyMod-2.7.9/Lib") do
          system("git rm #{item[:filename]}")
        end
      end
    end
  end

  output_list(list)
end

