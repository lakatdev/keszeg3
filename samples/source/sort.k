(rt MAIN
    print const n:
    input $ length

    i = 0
    while i < length
        disp = i
        disp +=
        print $ disp
        print const .:
        input $ tmp
        numbers : i <= tmp
        i +=
    end

    nm = length
    nm -=
    i = 0
    while i < nm
        secit = nm - i
        j = 0
        while j < secit
            this <= numbers : j
            j +=
            next <= numbers : j
            j -=
            if this > next
                numbers : j <= next
                j +=
                numbers : j <= this
                j -=
            end
            j +=
        end
        i +=
    end

    i = 0
    while i < length
        disp <= numbers : i
        print $ disp
        print ascii 32
        i +=
    end

    print const \n
return)
