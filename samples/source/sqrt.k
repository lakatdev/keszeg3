input num number
mul number number 10000
set sqrt number
div sqrt sqrt 2
flag whileflag
if different sqrt temp
    set temp sqrt
    div tmp number temp
    add tmp tmp temp
    div tmp tmp 2
    set sqrt tmp
    jump whileflag
end
set disp sqrt
div sqrt sqrt 100
print num sqrt
mul sqrt sqrt 100
sub disp disp sqrt
print string .
print num disp
print string \n