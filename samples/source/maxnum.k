rout main
    print string n:
    input num length

    set i 0
    while i < length
        set disp i
        inc disp
        print num disp
        print string .:
        input num tmp
        arrset numbers i tmp
        inc i
    end

    arrget max numbers 0
    set i 1
    while i < length
        arrget tmp numbers i
        if tmp > max
            set max tmp
        end
        inc i
    end

    print string max:
    print num max
    print string \n
return
