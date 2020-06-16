#Keszeg language
##How to build
You can use the provided build script which uses *gcc* to build the sources

`./build.sh`

It is recommended to use the *-O3* build flag on *gcc* when compiling *keszegr*

##Compiling and running the code
When you are ready with the keszeg source file, you should first compile it with *keszegc*

`./keszegc source.kszg program`

This will grab the "source.kszg" file and compile it a keszeg binary named "program"

You can run this by using *keszegr*

`./keszegr program`

##Writing code
You write keszeg code by giving instructions to the vm. A keszeg source file should only contain these nothing else (besides comments)

###Hello world
This hello world program uses the print instruction with the string parameter. This print everything after the space after the string paramter

`print string Hello World!\n`

The "\n" at the end print a new line. There are only 2 escaped characters in keszeg, one is the newline character the other is the numbersign (#)

- newline: "\n"
- numbersign: "\h"

###Print numbers from 0 to 100
This program uses a while loop to print numbers from 0 to 100 each to a new line

<code><pre>
set i 0
while lessequals i 100
    print num i
    print string \n
    inc i
end
</pre></code>

###Comments
You can document your code using comments. After a '#' in the line, nothing will be compiled.

Example:

<code><pre>
set i 0 #set value of i to 0
while lessequals i 100 #repeats until i is 100
    #prints value of i
    print num i
    print string \n
    inc i
end
</pre></code>

