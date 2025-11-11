# Compiler

Compiler for a C-like language written in C++. Input code is compiled into x86-64 assembly (intel syntax).

## Usage

The compiler can be built using the provided Makefile by simply running `make`. To run the compiler, provide the name of the input executable, i.e. `./compiler test.c`. There are also the following optional command-line arguments:

- `-o outfile` provide a name for the compiled assembly file
- `-v` display verbose information of the compiler's workings. This prints the parse path and a visual representation of the generated abstract syntax tree

On Linux machines with `nasm` installed, the Makefile can also be used to assemble any generated assembly into an executable. To do this, compile the code into a file with file extension `.asm`. Then run `make a.out` to make the executable. This can then be run with `./a.out`. The `make asm-clean` command can be used to remove any files built by the compiler or `nasm`.

## Example

Given the following input Fibonacci program:
```c
int fib(int x) {
  if (x == 0) return 0;
  if (x == 1) return 1;

  /* Exponential time complexity... just for testing! */
  return fib(x - 1) + fib(x - 2);
}

int main(void) {
  int i;

  i = 0;
  while (i < 10) {
    write(fib(i));
    i = i + 1;
  }

  return 0;
}
```
The result is the following assembly:

```nasm
global main

extern printf
extern scanf

section .data
  read_int_fmt: db "%lld", 0x0
  read_float_fmt: db "%lf", 0x0
  write_int_fmt: db "%lld", 0xA, 0x0
  write_flt_fmt: db "%lf", 0xA, 0x0

section .bss

section .text
fib:
  push rbp
  mov rbp, rsp
  push rdi

  mov r10, [rbp - 8]
  push r10
  mov r10, 0
  pop r11
  cmp r11, r10
  mov r10, 0
  sete r10b
  cmp r10, 0
  jne .if_true0
  jmp .if_end0

.if_true0:
  mov r10, 0
  mov rax, r10
  jmp .func_end
.if_end0:
  mov r10, [rbp - 8]
  push r10
  mov r10, 1
  pop r11
  cmp r11, r10
  mov r10, 0
  sete r10b
  cmp r10, 0
  jne .if_true1
  jmp .if_end1

.if_true1:
  mov r10, 1
  mov rax, r10
  jmp .func_end
.if_end1:
  mov r10, [rbp - 8]
  push r10
  mov r10, 1
  pop r11
  sub r11, r10
  mov r10, r11
  mov rdi, r10
  call fib
  mov r10, rax
  push r10
  mov r10, [rbp - 8]
  push r10
  mov r10, 2
  pop r11
  sub r11, r10
  mov r10, r11
  mov rdi, r10
  call fib
  mov r10, rax
  pop r11
  add r10, r11
  mov rax, r10
  jmp .func_end

  mov rax, 0
.func_end:
  mov rsp, rbp
  pop rbp
  ret

main:
  push rbp
  mov rbp, rsp
  sub rsp, 8

  mov r10, 0
  mov qword [rbp - 8], r10

.while_start0:
  mov r10, [rbp - 8]
  push r10
  mov r10, 10
  pop r11
  cmp r11, r10
  mov r10, 0
  setl r10b
  cmp r10, 0
  je .while_end0

  mov r10, [rbp - 8]
  mov rdi, r10
  call fib
  mov r10, rax
  mov rdi, write_int_fmt
  mov rsi, r10
  mov rax, 0
  call printf

  mov r10, [rbp - 8]
  push r10
  mov r10, 1
  pop r11
  add r10, r11
  mov qword [rbp - 8], r10

  jmp .while_start0
.while_end0:
  mov r10, 0
  mov rax, r10
  jmp .func_end

  mov rax, 0
.func_end:
  mov rsp, rbp
  pop rbp
  ret
```
We can assemble this into an executable using `nasm`. Running this yields the output:
```console
0
1
1
2
3
5
8
13
21
34
```
These are the elements $F_0$ to $F_9$ of the Fibonacci sequence.

# Resources

I found the following resources helpful for understanding how to design and implement a compiler, as well as for determining the grammar of my language:
- These [pdfs](https://github.com/dillondaudert/C-minus-minus/tree/master/docs) that appear to be from a university assignment on building a compiler
- This [language specification](https://www2.cs.arizona.edu/~debray/Teaching/CSc453/DOCS/cminusminusspec.html) for C--. My language is mostly a subset of this, though I found varying specifications for C--
- This [guide](https://austinhenley.com/blog/teenytinycompiler1.html) on building a compiler for a simpler language
- This [repo](https://github.com/luamfb/intro_x86-64) for learning x86-64 assembly
- These [blogposts](https://norasandler.com/2017/11/29/Write-a-Compiler.html) on implementing a compiler for C
