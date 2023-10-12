# Keszeg language
## How to build
You can use the provided build script which uses *gcc* to build the sources.

`./build.sh`

It is recommended to use the *-O3* build flag on *gcc* when compiling *keszegr*.

## Compiling and running the code
When you are ready with the keszeg source file, you should first compile it with *keszegc*.

`./keszegc source.kszg program`

This will grab the "source.kszg" file and compile it to a keszeg binary named "program".

You can run this by using *keszegr*.

`./keszegr program`

## Writing code
You write keszeg code by giving instructions to the vm. A keszeg source file should only contain these, nothing else (besides comments).

### Hello world
This hello world program uses the print instruction with the string parameter. This prints everything after the space after the string parameter.

`print string Hello World!\n`

The "\n" at the end prints a new line. There are only 3 escaped characters in keszeg, one is the newline character and the other two are the numbersign (#) and the space.

- newline: "\n"
- numbersign: "\h"
- space: "\s"

### Print numbers from 0 to 100
This program uses a while loop to print numbers from 0 to 100 each to a new line.

```
rout main
    set i 0
    while i <= 100
        print num i
        print string \n
        inc i
    end
return
```

### Subroutines
Each program execution starts at the main subroutine (basically a main flag) and runs until its return instruction is reached.
One can call another subroutine using the call instruction.

The following program prints the line "monke" 20 times.

```
rout monke
    set j 0
    while j < 10
        print string monke\n
        inc j
    end
return

rout main
    set i 0
    while i < 2
        call monke
        inc i
    end
return
```

Notice that the iterator variables differ. That is because in keszeg they are not specific to scope.

### Comments
You can document your code using comments. After a '#' in the line, nothing will be compiled.

Example:

```
rout main
    set i 0                    #set the value of i to 0
    while i <= 100             #repeat while i <= 100
        print num i            #print value of i
        print string \n        #print newline character
        inc i                  #increase i by one
    end                        #end of while loop
return
```

# All default instructions
Here is a list of all instructions with a description of what they do. It is important to note that they are case insensitive.

## Arithmetic operations  

### set X Y
Sets value of Y to X variable, Y can be either a constant or a variable.

### add X Y Z
Stores the sum of Y and Z in the X variable.

### sub X Y Z
Stores the difference of Y and Z in the X variable.

### mul X Y Z
Stores the product of Y and Z in the X variable.

### div X Y Z
Stores the ratio of Y and Z in the X variable.

### mod X Y Z
Stores Y mod Z value in the X variable.

### inc X (Y)
Increases the X variable by Y if Y is specified, if not then by 1.

### dec X (Y)
Decreases the X variable by Y if Y is specified, if not then by 1.

## Control flow instructions

### if X C Y
The if instruction has 6 different conditions:
- =
- <
- \>
- !=
- <=
- \>=

The value of X is compared to the value of Y the way the condition describes.

If the result is true the code between this *if* instruction and the corresponding *end* instruction will be executed.

### end
Indicates end of *if* conditions and *while* loops.

### flag X
Flags a position in the execution position.

### jump X
Continues execution at X flag.

### exec X
Executes X command in the default shell.

### while X C Y
Works the same as *if*, except it repeats execution until condition is false.

### call X
Program execution jumps to X subroutine and when its finished, returns here.

### return
End of subroutine, execution continues from where the subroutine was called.

### rout X
Declares X subroutine.

## Input/Output operations

### print P X
The print instruction has 3 different options:
- num
- ascii
- string

The *num* option prints the numerical value of the constant or the variable.

The *ascii* option print the corresponding ascii character to the numerical value of a constant or a variable.

The *string* option prints everything written after the instruction's parameter.
### input P X
This instruction has 3 different options:
- num
- ascii
- string

The *num* option receives a number input and places the value in X variable.

The *ascii* option receives a character as input and places its ascii value in the X variable.

The *string* option receives a string input and loads it into the X array.

### save X Y
Saves contents of X array to Y file (path is relative to the vm).
### load X Y
Loads contents from Y file to X array (path is relative to the vm).

## Arrays

### arrset X Y Z
Sets the (Y - 1)th value of the X array to Z. Resizes the array if it is not large enough.

### arrget X Y Z
Sets the X variable to the (Z - 1)th value of the Y array, returns 0 if Z is out of bounds.

### arrsize X Y
Sets the X variable to the current size of the Y array.

### free X
Empties an array, clears its content.
