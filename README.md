# Keszeg language
## How to build
You can use the provided build script which uses *gcc* to build the sources

`./build.sh`

It is recommended to use the *-O3* build flag on *gcc* when compiling *keszegr*

## Compiling and running the code
When you are ready with the keszeg source file, you should first compile it with *keszegc*

`./keszegc source.kszg program`

This will grab the "source.kszg" file and compile it a keszeg binary named "program"

You can run this by using *keszegr*

`./keszegr program`

## Writing code
You write keszeg code by giving instructions to the vm. A keszeg source file should only contain these, nothing else (besides comments)

### Hello world
This hello world program uses the print instruction with the string parameter. This print everything after the space after the string paramter

`print string Hello World!\n`

The "\n" at the end print a new line. There are only 2 escaped characters in keszeg, one is the newline character the other is the numbersign (#)

- newline: "\n"
- numbersign: "\h"

### Print numbers from 0 to 100
This program uses a while loop to print numbers from 0 to 100 each to a new line

```
set i 0
while lessequals i 100
    print num i
    print string \n
    inc i
end
```

### Comments
You can document your code using comments. After a '#' in the line, nothing will be compiled.

Example:

```
set i 0                    #set the value of i to 0
while lessequals i 100     #repeat while i <= 100
    print num i            #print value of i
    print string \n        #print newline character
    inc i                  #increase i by one
end                        #end of while loop
```

## All default instructions
Here is a list of all instructions with a description of what they do

### set X Y
Sets value of Y to X variable, Y can be either a constant or a variable
### if P X Y
The if instruction has 6 different options
- equals
- less
- more
- different
- lessequals
- moreequals

The value of X is compared to the value of Y the way the parameter describes

If the result is true the code between this *if* instruction and the corresponding *end* instruction will be executed
### add X Y Z
Stores the sum of Y and Z in the X variable
### array X Y
Defines X array with size of Y
### sub X Y Z
Stores the difference of Y and Z in the X variable
### mul X Y Z
Stores the product of Y and Z in the X variable
### div X Y Z
Stores the ratio of Y and Z in the X variable
### print P X
The print instruction has 3 different options
- num
- ascii
- string

The *num* option prints the numerical value of the constant or the variable

The *ascii* option print the corresponding ascii character to the numerical value of a constant or a variable

The *string* option prints everything written after the instruction's parameter
### input P X
This instruction has 2 different options
- num
- ascii

The *num* option receives a number input and places the value in X variable

The *ascii* option receives a character as input and places its ascii value in the X variable
### mod X Y Z
Stores Y mod Z value in the X variable
### end
Indicates end of *if* conditions and *while* loops
### arrset X Y Z
Sets the (Y - 1)th value of the X array to Z
### arrget X Y Z
Sets the X variable to the (Z - 1)th value of the Y array
### inc X (Y)
Increases the X variable by Y if Y is specified, if not then by 1
### dec X (Y)
Decreases the X variable by Y if Y is specified, if not then by 1
### flag X
Flags a position in the execution position
### jump X
Continues execution at X flag
### free X
Frees X array from memory
### exec X
Executes X command in the default shell
### while P X Y
Works the same as *if*, except it repeats execution until condition is false