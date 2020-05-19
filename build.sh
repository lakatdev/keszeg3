export PATH="/opt/kotlin/bin:$PATH"
kotlinc compiler/*.kt -o keszegc
gcc vm/keszegvm.c -o keszegr
mv keszegc.kexe keszegc
