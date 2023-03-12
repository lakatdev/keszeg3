fun main
    print string n:
    input num length
    array numbers length

    set i 0
    while less i length
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
    while less i length
        arrget tmp numbers i
        if more tmp max
            set max tmp
        end
        inc i
    end

    print string max:
    print num max
    print string \n

    free numbers
return
