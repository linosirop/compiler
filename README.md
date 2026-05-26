\# MiniCompiler



MiniCompiler — это учебный компилятор на C++17 для упрощённого C-like языка.



Проект реализован по спринтам и постепенно проходит все основные этапы компиляции:



1\. \*\*Sprint 1\*\* — лексический анализатор, Lexer

2\. \*\*Sprint 2\*\* — парсер и AST

3\. \*\*Sprint 3\*\* — семантический анализ

4\. \*\*Sprint 4\*\* — промежуточное представление, IR

5\. \*\*Sprint 5\*\* — генерация x86-64 assembly

6\. \*\*Sprint 6\*\* — сложный control flow и short-circuit

7\. \*\*Sprint 7\*\* — массивы, external calls и оптимизации

8\. \*\*Sprint 8\*\* — финальный CLI, тесты, документация и demo



\---



\# 1. Требования к окружению



Проект можно редактировать на Windows 11, но backend компилятора генерирует Linux x86-64 assembly.



Для проверки Sprint 5–8 используется:



\- WSL2 Ubuntu

\- CMake

\- g++

\- NASM

\- GNU ld

\- gcc

\- bash



Установка инструментов в WSL Ubuntu:



```bash

sudo apt update

sudo apt install -y build-essential cmake nasm git graphviz

```



Проверка версий:



```bash

g++ --version

cmake --version

nasm -v

ld --version

gcc --version

```



\---



\# 2. Где хранить проект



Рекомендуется хранить проект внутри файловой системы WSL:



```bash

\~/projects/compiler

```



Не рекомендуется собирать проект прямо из Windows-папки:



```bash

/mnt/c/Users/...

```



Потому что могут возникнуть проблемы с правами, CMake cache и путями.



Перейти в проект:



```bash

cd \~/projects/compiler

```



\---



\# 3. Сборка проекта



Из корня проекта:



```bash

cmake -S . -B build -DBUILD\_TESTS=ON -G "Unix Makefiles"

cmake --build build

```



Запуск стандартных CTest-тестов:



```bash

ctest --test-dir build --output-on-failure

```



Ожидаемый результат:



```text

100% tests passed, 0 tests failed out of 5

```



\---



\# 4. Финальная проверка всего проекта



Для финальной проверки Sprint 8 используется скрипт:



```bash

bash scripts/run\_final\_tests.sh

```



Ожидаемый результат:



```text

\[final-tests] all final tests passed

```



Проверить код возврата:



```bash

echo $?

```



Ожидаемо:



```text

0

```



\---



\# 5. Два интерфейса компилятора



В проекте собираются два бинарных файла:



```bash

./build/compiler

./build/mycc

```



\## Старый интерфейс `compiler`



Используется для отдельных стадий компиляции:



```bash

./build/compiler lex --input examples/hello.src

./build/compiler parse --input examples/hello.src --ast-format text

./build/compiler check --input examples/hello.src --show-types

./build/compiler ir --input examples/factorial.src --stats --validate

./build/compiler compile --input demo/final\_showcase.src --output program.asm --target x86\_64

```



\## Новый финальный интерфейс `mycc`



Добавлен в Sprint 8. Он ближе к обычным Unix-компиляторам:



```bash

./build/mycc --help

./build/mycc --version

./build/mycc -S demo/final\_showcase.src -o final\_showcase.asm

./build/mycc -c demo/final\_showcase.src -o final\_showcase.o

./build/mycc demo/final\_showcase.src -o final\_showcase --optimize

./build/mycc --ast demo/final\_showcase.src

./build/mycc --ir demo/final\_showcase.src --optimize --optimization-report

```



\---



\# Sprint 1 — Lexer



\## Что реализовано



В Sprint 1 был реализован лексический анализатор.



Lexer читает исходный `.src` файл и разбивает его на токены.



Пример исходного кода:



```c

fn main() -> int {

&#x20;   int x = 42;

&#x20;   return x;

}

```



Lexer превращает его в токены:



```text

KW\_FN

IDENTIFIER

LPAREN

RPAREN

ARROW

KW\_INT

LBRACE

KW\_INT

IDENTIFIER

ASSIGN

INT\_LITERAL

SEMICOLON

KW\_RETURN

IDENTIFIER

SEMICOLON

RBRACE

END\_OF\_FILE

```



Lexer поддерживает:



\- ключевые слова;

\- идентификаторы;

\- integer literals;

\- float literals;

\- string literals;

\- boolean literals;

\- операторы;

\- разделители;

\- комментарии;

\- line/column tracking;

\- сообщения об ошибках.



\## Команды проверки



Запустить lexer:



```bash

./build/compiler lex --input examples/hello.src

```



Сохранить токены в файл:



```bash

./build/compiler lex --input examples/hello.src --output tokens.txt

```



Посмотреть результат:



```bash

cat tokens.txt

```



\## Что показать преподавателю



Команда:



```bash

./build/compiler lex --input examples/hello.src

```



Пояснение:



```text

На этом этапе исходный текст программы разбивается на поток токенов.

Каждый токен содержит тип, лексему, строку и колонку.

```



\---



\# Sprint 2 — Parser и AST



\## Что реализовано



В Sprint 2 был реализован парсер.



Parser получает токены от lexer-а и строит AST — Abstract Syntax Tree.



AST показывает структуру программы:



\- функции;

\- параметры;

\- блоки;

\- переменные;

\- выражения;

\- if/else;

\- while;

\- for;

\- return;

\- function calls.



Пример:



```c

fn main() -> int {

&#x20;   int x = 10;

&#x20;   return x;

}

```



AST будет описывать:



```text

Program

&#x20; FunctionDecl main -> int

&#x20;   Block

&#x20;     VarDecl int x = 10

&#x20;     Return x

```



\## Команды проверки



Вывести AST в консоль:



```bash

./build/compiler parse --input examples/hello.src --ast-format text

```



Сохранить AST в файл:



```bash

./build/compiler parse --input examples/hello.src --ast-format text --output ast.txt

cat ast.txt

```



Сгенерировать AST в Graphviz DOT:



```bash

./build/compiler parse --input examples/hello.src --ast-format dot --output ast.dot

```



Сделать картинку AST:



```bash

dot -Tpng ast.dot -o ast.png

```



\## Что показать преподавателю



Команда:



```bash

./build/compiler parse --input examples/hello.src --ast-format text

```



Пояснение:



```text

Parser строит AST, то есть дерево синтаксической структуры программы.

В отличие от lexer-а, parser уже понимает вложенность блоков, функций и выражений.

```



\---



\# Sprint 3 — Semantic Analysis



\## Что реализовано



В Sprint 3 был реализован семантический анализ.



Semantic analyzer проверяет смысл программы:



\- объявлена ли переменная перед использованием;

\- нет ли повторных объявлений в одной области видимости;

\- совпадают ли типы;

\- правильно ли вызываются функции;

\- соответствует ли return типу функции;

\- является ли условие в `if` / `while` boolean-выражением;

\- корректны ли scope rules.



Также реализованы:



\- symbol table;

\- type system;

\- semantic error reporting;

\- type annotations.



\## Команды проверки



Запустить semantic check:



```bash

./build/compiler check --input examples/hello.src

```



Запустить semantic check с выводом типов:



```bash

./build/compiler check --input examples/hello.src --show-types

```



Вывести symbol table:



```bash

./build/compiler symbols --input examples/hello.src

```



Сохранить symbol table:



```bash

./build/compiler symbols --input examples/hello.src --output symbols.txt

cat symbols.txt

```



\## Что показать преподавателю



Команда:



```bash

./build/compiler check --input examples/hello.src --show-types

```



Пояснение:



```text

На этом этапе компилятор проверяет программу не только синтаксически,

но и семантически: типы, области видимости, объявления и return-выражения.

```



\---



\# Sprint 4 — Intermediate Representation, IR



\## Что реализовано



В Sprint 4 был реализован IR — промежуточное представление.



IR находится между AST и assembly.



Он нужен для:



\- более простой генерации машинного кода;

\- оптимизаций;

\- анализа control flow;

\- представления программы в виде инструкций.



Примеры IR-инструкций:



```text

ADD

SUB

MUL

DIV

CMP\_GT

CMP\_LE

JUMP

JUMP\_IF

CALL

RETURN

LOAD

STORE

ALLOCA

```



\## Команды проверки



Сгенерировать IR:



```bash

./build/compiler ir --input examples/factorial.src

```



Сохранить IR в файл:



```bash

./build/compiler ir --input examples/factorial.src --output factorial.ir

cat factorial.ir

```



Показать IR со статистикой:



```bash

./build/compiler ir --input examples/factorial.src --stats

```



Проверить control flow:



```bash

./build/compiler ir --input examples/factorial.src --validate

```



Полная команда:



```bash

./build/compiler ir --input examples/factorial.src --stats --validate

```



Сгенерировать CFG в DOT:



```bash

./build/compiler ir --input examples/factorial.src --format dot --output cfg.dot

dot -Tpng cfg.dot -o cfg.png

```



\## Что показать преподавателю



Команда:



```bash

./build/compiler ir --input examples/factorial.src --stats --validate

```



Пояснение:



```text

IR показывает программу в виде промежуточных инструкций.

Это уже не AST, но ещё не assembly. На этом уровне удобно делать оптимизации и строить CFG.

```



\---



\# Sprint 5 — x86-64 Code Generation



\## Что реализовано



В Sprint 5 был добавлен backend, который переводит IR в x86-64 assembly.



Генератор поддерживает:



\- NASM syntax;

\- ELF64 object files;

\- System V AMD64 ABI;

\- function prologue;

\- function epilogue;

\- stack frame;

\- передачу аргументов через ABI-регистры;

\- return value через `rax`;

\- arithmetic operations;

\- function calls;

\- runtime library.



Добавленные компоненты:



```text

src/codegen/

&#x20; abi.\*

&#x20; stack\_frame.\*

&#x20; register\_allocator.\*

&#x20; x86\_generator.\*



src/runtime/

&#x20; runtime.asm

```



\## System V AMD64 ABI



Первые 6 integer-аргументов передаются через регистры:



```text

1-й аргумент -> rdi

2-й аргумент -> rsi

3-й аргумент -> rdx

4-й аргумент -> rcx

5-й аргумент -> r8

6-й аргумент -> r9

```



Возвращаемое значение передаётся через:



```text

rax

```



\## Stack frame



Каждая функция генерирует prologue:



```asm

push rbp

mov rbp, rsp

sub rsp, N

```



И epilogue:



```asm

mov rsp, rbp

pop rbp

ret

```



Локальные переменные и временные значения хранятся относительно `rbp`:



```asm

\[rbp-8]

\[rbp-16]

\[rbp-24]

```



\## Команды проверки Sprint 5



Сгенерировать assembly:



```bash

./build/compiler compile \\

&#x20; --input tests/codegen/valid/function\_calls/add\_call.src \\

&#x20; --output add.asm \\

&#x20; --target x86\_64 \\

&#x20; --emit-stack-map

```



Посмотреть assembly:



```bash

cat add.asm

```



Собрать object file:



```bash

nasm -f elf64 -o add.o add.asm

```



Собрать runtime:



```bash

nasm -f elf64 -o runtime.o src/runtime/runtime.asm

```



Слинковать программу:



```bash

ld -o add\_program runtime.o add.o

```



Запустить:



```bash

./add\_program

echo $?

```



Ожидаемый результат:



```text

5

```



\## Что это доказывает



```text

source code

→ compiler

→ x86-64 assembly

→ object file

→ executable

→ program run

```



Если `echo $?` показывает `5`, значит программа реально выполнилась и вернула результат `2 + 3 = 5`.



\---



\# Sprint 6 — Control Flow и Short-Circuit



\## Что реализовано



Sprint 6 расширил backend для сложного control flow.



Добавлена поддержка:



\- `if`;

\- `if-else`;

\- nested conditionals;

\- `while`;

\- `for`;

\- relational jumps;

\- `\&\&`;

\- `||`;

\- `!`;

\- short-circuit evaluation.



Добавленные компоненты:



```text

src/codegen/

&#x20; label\_manager.\*

&#x20; control\_flow\_generator.\*

&#x20; expression\_generator.\*

```



\## Что такое short-circuit



Short-circuit — это способ вычисления логических выражений, при котором правая часть не вычисляется, если результат уже известен по левой части.



Для `\&\&`:



```text

false \&\& X = false

```



Поэтому если левая часть false, правая часть не вычисляется.



Для `||`:



```text

true || X = true

```



Поэтому если левая часть true, правая часть не вычисляется.



\## Главный тест Sprint 6



Файл:



```bash

cat tests/control\_flow/valid/logical\_ops/short\_circuit\_and.src

```



Содержит:



```c

fn main() -> int {

&#x20;   int a = 0;

&#x20;   int b = 10;

&#x20;   if (a != 0 \&\& b / a > 2) {

&#x20;       return 99;

&#x20;   }

&#x20;   return 5;

}

```



Если short-circuit не работает, программа выполнит:



```text

b / a

```



то есть:



```text

10 / 0

```



и упадёт.



Если short-circuit работает, правая часть не вычислится, и программа вернёт `5`.



\## Команды проверки



Сгенерировать assembly:



```bash

./build/compiler compile \\

&#x20; --input tests/control\_flow/valid/logical\_ops/short\_circuit\_and.src \\

&#x20; --output sc\_and.asm \\

&#x20; --target x86\_64 \\

&#x20; --emit-stack-map

```



Показать важные строки assembly:



```bash

grep -n "L\_logic\_false\\|L\_and\_rhs\\|idiv\\|je\\|jne\\|jmp" sc\_and.asm

```



Ожидаемо будет видно что-то похожее:



```text

je .main\_L\_logic\_false3

jmp .main\_L\_and\_rhs1

.main\_L\_and\_rhs1:

idiv r10

.main\_L\_logic\_false3:

```



Это значит:



```text

Если левая часть false, программа прыгает на L\_logic\_false.

Инструкция idiv находится в правой ветке L\_and\_rhs и не выполняется.

```



Собрать и запустить:



```bash

nasm -f elf64 -o sc\_and.o sc\_and.asm

nasm -f elf64 -o runtime.o src/runtime/runtime.asm

ld -o sc\_and\_program runtime.o sc\_and.o



./sc\_and\_program

echo $?

```



Ожидаемый результат:



```text

5

```



\## Что показать преподавателю



Главная демонстрация:



```bash

cat tests/control\_flow/valid/logical\_ops/short\_circuit\_and.src

grep -n "L\_logic\_false\\|L\_and\_rhs\\|idiv\\|je\\|jmp" sc\_and.asm

./sc\_and\_program

echo $?

```



Пояснение:



```text

В исходнике есть потенциальное деление на ноль.

Если short-circuit не работает, программа падает.

Но она возвращает 5, а в assembly видно, что idiv находится в правой ветке,

которая пропускается при false left operand.

```



\---



\# Sprint 7 — Arrays, External Calls, Optimizations



\## Что реализовано



Sprint 7 добавил три больших направления:



1\. массивы;

2\. внешние вызовы функций из C library;

3\. оптимизации IR.



Добавленные файлы:



```text

src/ir/

&#x20; optimizer.\*



src/codegen/

&#x20; array\_generator.\*

&#x20; external\_calls.\*

&#x20; optimization\_passes.\*



src/libc/

&#x20; stdlib.h

```



\---



\## 7.1. Оптимизации



Реализованы:



\- constant folding;

\- constant propagation;

\- branch simplification;

\- dead code elimination;

\- optimization report.



\### Constant Folding



Пример:



```c

int x = 10 + 20;

```



После оптимизации:



```c

int x = 30;

```



\### Constant Propagation



Пример:



```c

int x = 30;

int y = x \* 2;

```



Компилятор понимает, что `x = 30`, и может заменить использование `x`.



\### Branch Simplification



Пример:



```c

if (60 > 50) {

&#x20;   return 1;

} else {

&#x20;   return 0;

}

```



Условие известно на этапе компиляции, поэтому можно заменить на прямой переход в true branch.



\### Dead Code Elimination



Если после оптимизаций какая-то ветка стала недостижимой, её можно удалить.



\## Команды проверки оптимизаций



```bash

./build/compiler ir \\

&#x20; --input tests/optimization/constant\_folding.src \\

&#x20; --optimize \\

&#x20; --optimization-report \\

&#x20; --stats

```



Ожидаемый вывод содержит:



```text

Optimization Report:

&#x20; Constant folding

&#x20; Constant propagation

&#x20; Branch simplification

&#x20; Dead code elimination

```



Пример из проверки:



```text

Constant folding: 3 expression(s) folded

Constant propagation: 9 use(s) replaced

Branch simplification: 1 branch(es) simplified

Dead code elimination: 6 instruction(s) removed

```



\---



\## 7.2. Массивы



Добавлена базовая поддержка static arrays.



Пример:



```c

fn main() -> int {

&#x20;   int arr\[3] = {1, 2, 3};

&#x20;   arr\[1] = arr\[1] + 4;

&#x20;   return arr\[0] + arr\[1] + arr\[2];

}

```



Проверка:



```bash

./build/compiler compile \\

&#x20; --input tests/arrays/valid/static\_array.src \\

&#x20; --output array.asm \\

&#x20; --target x86\_64 \\

&#x20; --emit-stack-map



nasm -f elf64 -o array.o array.asm

nasm -f elf64 -o runtime.o src/runtime/runtime.asm

ld -o array\_program runtime.o array.o



./array\_program

echo $?

```



Ожидаемый результат:



```text

10

```



Это доказывает:



```text

массив создан

элементы записаны

элемент arr\[1] изменён

сумма посчитана правильно

```



\---



\## 7.3. External Calls



Добавлена поддержка вызова внешних функций, например `printf` из libc.



Пример:



```c

extern int printf(string fmt, int value);



fn main() -> int {

&#x20;   printf("answer=%d\\n", 42);

&#x20;   return 0;

}

```



Проверка:



```bash

./build/compiler compile \\

&#x20; --input tests/external\_calls/valid/printf\_demo.src \\

&#x20; --output printf\_demo.asm \\

&#x20; --target x86\_64



nasm -f elf64 -o printf\_demo.o printf\_demo.asm

gcc -no-pie -o printf\_demo printf\_demo.o



./printf\_demo

```



Ожидаемый вывод:



```text

answer=42

```



Важно:



```text

Для printf используется gcc -no-pie, потому что printf приходит из libc.

```



\---



\# Sprint 8 — Final CLI, Tests, Docs, Demo



\## Что реализовано



Sprint 8 — финальная полировка проекта.



Добавлено:



\- новый CLI `mycc`;

\- `--help`;

\- `--version`;

\- режимы `-S`, `-c`, full compilation;

\- `--ast`;

\- `--ir`;

\- `--optimize`;

\- `--optimization-report`;

\- diagnostics module;

\- final tests;

\- final demo;

\- user guide;

\- developer guide;

\- error handling docs.



Добавленные файлы:



```text

src/utils/

&#x20; diagnostics.\*



scripts/

&#x20; run\_final\_tests.sh

&#x20; build\_and\_run.sh



docs/

&#x20; user\_guide.md

&#x20; developer\_guide.md

&#x20; error\_handling.md

&#x20; final\_demo.md



tests/final/

&#x20; good/

&#x20; bad/



demo/

&#x20; final\_showcase.src

```



\## Новый CLI



Показать help:



```bash

./build/mycc --help

```



Показать версию:



```bash

./build/mycc --version

```



Сгенерировать assembly:



```bash

./build/mycc -S demo/final\_showcase.src -o final\_showcase.asm --optimize

```



Сгенерировать object file:



```bash

./build/mycc -c demo/final\_showcase.src -o final\_showcase.o --optimize

```



Полная компиляция:



```bash

./build/mycc demo/final\_showcase.src -o final\_showcase --optimize -v

```



Запуск:



```bash

./final\_showcase

echo $?

```



Ожидаемый вывод:



```text

final result=40

0

```



\## Что демонстрирует final\_showcase



Файл:



```bash

cat demo/final\_showcase.src

```



Демонстрирует:



\- external call `printf`;

\- массив;

\- обращение к элементам массива;

\- рекурсивную функцию `fact`;

\- арифметические выражения;

\- constant folding;

\- успешное завершение программы.



Логика:



```text

arr = {1, 2, 3, 4}

sum = 1 + 2 + 3 + 4 = 10

folded = 10 + 20 = 30

sum + folded = 40

```



Поэтому программа печатает:



```text

final result=40

```



И возвращает:



```text

0

```



\## Финальная проверка Sprint 8



```bash

bash scripts/run\_final\_tests.sh

```



Ожидаемый результат:



```text

\[final-tests] all final tests passed

```



Проверка exit code:



```bash

echo $?

```



Ожидаемо:



```text

0

```



\---



\# Полная команда для защиты



Самая важная последовательность:



```bash

cd \~/projects/compiler



cmake -S . -B build -DBUILD\_TESTS=ON -G "Unix Makefiles"

cmake --build build

ctest --test-dir build --output-on-failure



bash scripts/run\_final\_tests.sh

```



Ожидаемо:



```text

100% tests passed, 0 tests failed out of 5

\[final-tests] all final tests passed

```



Финальная demo-программа:



```bash

./build/mycc demo/final\_showcase.src -o final\_showcase --optimize -v

./final\_showcase

echo $?

```



Ожидаемо:



```text

final result=40

0

```



\---



\# Как посмотреть assembly



Для любого `.src` файла:



```bash

./build/mycc -S demo/final\_showcase.src -o final\_showcase.asm --optimize

cat final\_showcase.asm

```



Или через старый интерфейс:



```bash

./build/compiler compile \\

&#x20; --input demo/final\_showcase.src \\

&#x20; --output final\_showcase.asm \\

&#x20; --target x86\_64 \\

&#x20; --emit-stack-map



cat final\_showcase.asm

```



\---



\# Как посмотреть IR



```bash

./build/mycc --ir demo/final\_showcase.src --optimize --optimization-report

```



Или:



```bash

./build/compiler ir \\

&#x20; --input demo/final\_showcase.src \\

&#x20; --optimize \\

&#x20; --optimization-report \\

&#x20; --stats

```



\---



\# Как посмотреть AST



```bash

./build/mycc --ast demo/final\_showcase.src

```



Или:



```bash

./build/compiler parse --input demo/final\_showcase.src --ast-format text

```



\---



\# Очистка временных файлов



Удалить generated files:



```bash

rm -f \*.o

rm -f \*.asm

rm -f \*\_program

rm -f final\_showcase

rm -f final\_showcase.mycc.asm

rm -f final\_showcase.mycc.o

rm -f add\_program sc\_and\_program array\_program printf\_demo

```



Удалить build:



```bash

rm -rf build

```



\---



\# Git



Перед коммитом желательно удалить generated-файлы:



```bash

rm -f \*.o \*.asm \*\_program

rm -f final\_showcase final\_showcase.mycc.asm final\_showcase.mycc.o

```



Проверить статус:



```bash

git status --short

```



Добавить изменения:



```bash

git add .

git add -f src/runtime/runtime.asm

```



Коммит:



```bash

git commit -m "Final compiler implementation"

```



Push:



```bash

git push

```



Если GitHub просит пароль, нужно вставлять Personal Access Token, а не обычный пароль.



\---



\# Краткое описание архитектуры



```text

source file

&#x20;  ↓

lexer

&#x20;  ↓

tokens

&#x20;  ↓

parser

&#x20;  ↓

AST

&#x20;  ↓

semantic analyzer

&#x20;  ↓

decorated AST

&#x20;  ↓

IR generator

&#x20;  ↓

IR optimizer

&#x20;  ↓

x86-64 code generator

&#x20;  ↓

NASM assembly

&#x20;  ↓

object file

&#x20;  ↓

linked executable

```



\---



\# Короткое объяснение проекта



MiniCompiler — это учебный компилятор, который реализует полный pipeline компиляции:



```text

исходный код → токены → AST → semantic check → IR → optimization → x86-64 assembly → executable

```



К финальному спринту проект умеет:



\- анализировать исходный код;

\- строить AST;

\- проверять типы и области видимости;

\- генерировать IR;

\- оптимизировать IR;

\- генерировать x86-64 assembly;

\- собирать исполняемый файл;

\- поддерживать массивы;

\- вызывать внешние функции вроде `printf`;

\- выполнять short-circuit для `\&\&` и `||`;

\- запускать финальный test suite одной командой.



