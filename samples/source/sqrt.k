rout MAIN
    input $ number
    number = number * 10000
    sqrt = number
    sqrt = sqrt / 2
    while sqrt != temp
        temp = sqrt
        tmp = number / temp
        tmp = tmp + temp
        tmp = tmp / 2
        sqrt = tmp
    end
    disp = sqrt
    sqrt = sqrt / 100
    print $ sqrt
    sqrt = sqrt * 100
    disp = disp - sqrt
    print const .
    print $ disp
    print const \n
return
