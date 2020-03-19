if ARGV.length != 3
    print "Bad arguments\n"
    print ARGV
    exit
end

filter_name = ARGV[0]
size = ARGV[1].to_i
float = ARGV[2]

filter = File.open(filter_name, "w+")

filter.write(size.to_s)
filter.write(" \n")
i = 0
while i < size
    j = 0
    while j < size
        filter.write(float)
        if j != size-1 || i != size-1 
            filter.write(" ")
        end
        j += 1
    end
    if (i != size-1) 
        filter.write("\n")
    end
    i += 1
end

print "Filter generated\n"