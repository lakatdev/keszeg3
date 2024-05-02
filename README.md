# Keszeg language version 3

## About Keszeg
Keszeg is a programming language with simplicity and clarity in mind. It is designed to be clean and simple, easy to get to know. By its design it has many limitations, but it provides functionality that can be a base even for more complex solutions, which can be more of an entertaining challange than an actual tool.

## Contents
- [Keszeg language version 3](#keszeg-language-version-3)
  - [About Keszeg](#about-keszeg)
  - [Contents](#contents)
  - [How to build](#how-to-build)
  - [Compiling and running the code](#compiling-and-running-the-code)
  - [Writing code](#writing-code)
    - [Subroutines](#subroutines)
    - [Comments](#comments)
    - [Escaped characters](#escaped-characters)
    - [Variables, arrays and strings](#variables-arrays-and-strings)
  - [Instructions](#instructions)
    - [Arithmetic operations](#arithmetic-operations)
      - [Setting variables](#setting-variables)
      - [Addition](#addition)
      - [Subtraction](#subtraction)
      - [Multiplication](#multiplication)
      - [Division](#division)
      - [Modulo](#modulo)
      - [Increment](#increment)
      - [Decrement](#decrement)
    - [Control flow instructions](#control-flow-instructions)
      - [if conditions](#if-conditions)
      - [end](#end)
      - [Flags](#flags)
      - [Executing system commands](#executing-system-commands)
      - [while loops](#while-loops)
      - [Calling and returning from a subroutine](#calling-and-returning-from-a-subroutine)
    - [Input/Output operations](#inputoutput-operations)
      - [Printing to the console](#printing-to-the-console)
      - [Receiving input from the console](#receiving-input-from-the-console)
      - [File I/O](#file-io)
    - [Array and string operations](#array-and-string-operations)
      - [Setting values of arrays and strings](#setting-values-of-arrays-and-strings)
  - [Example program](#example-program)


## How to build
Keszeg code can compiled and run using the following software:
- *keszegc* is what compiles the code into binary form
- *keszegr* is what runs the compiled binary.

These can be built with the following commands using gcc:

`gcc src/compiler/compile.c -o keszegc`

and

`gcc -O3 src/vm/keszegvm.c -o keszegr`.

The provided build.sh script contains exactly these two lines.

The O3 flag is to make running keszeg code faster. The difference is minimal, so if it is causing problems it can be avoided. Using gcc 13.2.0 and the provided sorting algorithm ran on an array of 30 thousand numbers, the runtime of the program without the O3 flag was 57.553 seconds, using the flag it was 57.093 seconds on the reference computer. The test included creating the array itself.

## Compiling and running the code
When the source file is ready, it can be compiled using *keszegc* in the following manner:

`./keszegc source.k binary`.

This will take the "source.k" file and compile it to a keszeg binary named "binary".

This binary can be executed using *keszegr*.

`./keszegr binary`

## Writing code
Code is written by giving the executor program instructions line by line. It is important to note that the instructions in the language are case insensitive while subroutine and variable names are not.

### Subroutines

The program enters in the "main" subroutine and runs until it is returned. From there can other subroutines be called. These are not functions, they do not return any value. For example:

```
rout main
    call other
return

rout other
    print const Hello World\n
return
```

this prints "Hello World".

### Comments
In a line after the # character all text is ignored during compilation. These are comments.

```
rout main
    i = 0
    # initialize loop
    while i < 100
        print $ i        # print the number
        print const \n   # print new line character
        i +=             # increase the iterator variable
    end
return
```

### Escaped characters
In strings there are 3 escaped characters, one is the new line character and the other two are the number sign (#) and the space.

- new line: "\n"
- number sign: "\h"
- space: "\s"

### Variables, arrays and strings
These are the three data types that can be used. They are not scope specific, keep that in mind while coding. Variables are 32bit signed integers, arrays are an array of these, while strings are like arrays, except they are 8bit (also signed).

## Instructions
There are four main groups of instructions

- Arithmetic
- Control flow
- I/O
- Arrays and strings

**It is *very* important to type spaces between each symbol described below, otherwise the code will not compile correctly.**

This will work:
`n = m + 4`

this will not:
`n=m+4`.

As a positive side effect, this makes the code readable.

### Arithmetic operations

#### Setting variables

`X = Y`

Sets value of Y to X variable, Y can be either a constant or a variable.

#### Addition

`X = Y + Z`

Stores the sum of Y and Z in the X variable.

#### Subtraction

`X = Y - Z`

Stores the difference of Y and Z in the X variable.

#### Multiplication

`X = Y * Z`

Stores the product of Y and Z in the X variable.

#### Division

`X = Y / Z`

Stores the ratio of Y and Z in the X variable.

#### Modulo

`X = Y % Z`

Stores Y mod Z value in the X variable.

#### Increment

`X += Y` or `X +=`

Increases the X variable by Y if Y is specified, if not then by 1.

#### Decrement

`X -= Y` or `X -=`

Decreases the X variable by Y if Y is specified, if not then by 1.

### Control flow instructions

#### if conditions

`if X condition Y`

The if instruction has 6 possible conditions:
- =
- <
- \>
- !=
- <=
- \>=

The value of X is compared to the value of Y the way the condition describes.

If the result is true the code between this *if* instruction and the corresponding *end* instruction will be executed.

#### end
Indicates the end of *if* conditions and *while* loops.

#### Flags
A flag flags a position in the execution position. It is usually not needed.

`flag beginning`

Using `jump` the execution continues at the specified flag:

`jump beginning`
 

#### Executing system commands
`exec X` executes X command in the default shell of the operating system.

#### while loops
`while X condition Y`

While loops work the same as *if* conditions, except they repeat execution until the condition is false.

#### Calling and returning from a subroutine

`rout example` declares a subroutine called "example". The instructions between this, and the corresponding `return` are the body of the subroutine.

When using `call example` the program execution continues at the specified subroutine (in this case "example") until that finishes, and then returns to where it was originally called from.

`return` describes the end of a subroutine, after that the execution returns to where it was called from. If *main* is returned the program finishes.

### Input/Output operations

Keszeg is capable of communicating with the outer world in mainly two ways. It can read and write standard console input and output and it can write and read strings and arrays to and from the file system. It is done using the following instructions:

#### Printing to the console

`print mode X`

The print instruction has 4 different modes:
- $
- ascii
- string
- const

The *$* (numerical) option prints the numerical value of the constant or the variable to the console. The *ascii* option does something similar, except it doesn't print it as number, but its corresponding ascii character.

The *string* option prints the data stored in the referenced string, while the *const* option prints everything written after the instruction's parameter.

#### Receiving input from the console

Similarly to printing, the `input mode X` instruction also has different modes.

- $
- ascii
- string

These work the same. *$* receives a number input to a variable, *ascii* a single character input to a variable, while *string* takes an entire string of text and places it in a string.

#### File I/O

Strings and arrays can be written to and read from files. The `save mode X Y` and `load mode X Y` instructions take care of that. The modes can be *array* and *string* depending on the type of the data.

When using `save array X Y` X is the array to be saved and Y is the path to the file which it should be saved to. Similarly with strings.

`load array X Y` works the same way, X is the destination array or string while Y is a path.

### Array and string operations

#### Setting values of arrays and strings
One can set the Y - 1th value of an X array to Z by using the following operators:

`X : Y <= Z`.

This loads Z to the array. If the array is not at least Y + 1 long, it will be resized.

With strings the operator is `<-`:

`X : Y <- Z`.

To get a previously set value back from arrays or strings, the following should be done:

`Z <= X : Y` when working with arrays and `Z <- X : Y` when working with strings.

To get the size of these types, there are two functions to be used.

`arrsize X Y` loads the length of the Y array to X, while
`strsize X Y` loads the length of the Y string to X.

To reset the sizes and empty the values, the `free mode X` instruction is to be used. Again, mode can be *string* or *array*.

The `cat X Y` instruction concatenates a constant Y string to theee string X. Y is everything written after the space character after the X value, just like previously with paths and constant string prints.

## Example program
Below is an example program to determine the maximum value of an array provided by the user using the console.

```
rout main
    call receive_input
    call determine_max
    call output_result
return

rout receive_input
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
return

rout determine_max
	max <= numbers : 0
	i = 1
	while i < length
		tmp <= numbers : i
		if tmp > max
			max = tmp
		end
		i +=
	end
return

rout output_result
    print const max:
    print $ max
    print const \n
return
```

The console will look like this (including the user input):
```
n:5
1.:45
2.:-23
3.:163
4.:-89
5.:0
max:163
```

*29th April 2024*
