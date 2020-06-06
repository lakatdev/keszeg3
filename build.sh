#export PATH="/opt/kotlin/bin:$PATH"
#kotlinc src/compiler_kotlin/*.kt -o keszegc
gcc src/compiler/compile.c -o keszegc
gcc src/vm/keszegvm.c -o keszegr
#mv keszegc.kexe keszegc
