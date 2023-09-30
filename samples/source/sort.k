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

    set nm length
    dec nm
    set i 0
    while i < nm
        set secit nm
        sub secit nm i
        set j 0
        while j < secit
            arrget this numbers j
            inc j
            arrget next numbers j
            dec j
            if this > next
                arrset numbers j next
                inc j
                arrset numbers j this
                dec j
            end
            inc j
        end
        inc i
    end

    set i 0
    while i < length
        arrget disp numbers i
        print num disp
        print ascii 32
        inc i
    end

    print string \n
return
