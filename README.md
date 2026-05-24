\# MiniCompiler



MiniCompiler — это учебный компилятор на C++17 для упрощённого C-like языка.



Проект реализует основные этапы компиляции:



1\. \*\*Sprint 1\*\* — лексический анализатор, Lexer

2\. \*\*Sprint 2\*\* — парсер и AST

3\. \*\*Sprint 3\*\* — семантический анализ

4\. \*\*Sprint 4\*\* — промежуточное представление, IR

5\. \*\*Sprint 5\*\* — генерация x86-64 assembly



\---



\## 1. Что нужно установить



Проект можно писать на Windows 11, но \*\*Sprint 5 проверяется через WSL Ubuntu\*\*, потому что генерация assembly сделана под:



\- Linux

\- NASM

\- ELF64

\- System V AMD64 ABI

\- `ld`

\- Linux syscalls



То есть на Windows проект редактируем, а компиляцию и запуск backend-а делаем через WSL.



\### Установка инструментов в WSL Ubuntu



```bash

sudo apt update

sudo apt install -y build-essential cmake nasm git



Проверка:



g++ --version

cmake --version

nasm -v

ld --version



3\. Сборка проекта



Из корня проекта:



cmake -S . -B build -DBUILD\_TESTS=ON -G "Unix Makefiles"

cmake --build build



Запуск всех тестов:



ctest --test-dir build --output-on-failure



Ожидаемый результат:



100% tests passed



Sprint 1 — Lexer / Лексический анализатор



На этом этапе исходный код разбивается на токены.



Пример:



fn main() {

&#x20;   int x = 42;

}



Lexer превращает это в набор токенов:



KW\_FN

IDENTIFIER

LPAREN

RPAREN

LBRACE

KW\_INT

IDENTIFIER

ASSIGN

INT\_LITERAL

SEMICOLON

RBRACE

Запустить lexer

./build/compiler lex --input examples/hello.src

Сохранить токены в файл

./build/compiler lex --input examples/hello.src --output tokens.txt

Посмотреть результат

cat tokens.txt

Что демонстрирует Sprint 1

распознавание ключевых слов;

распознавание идентификаторов;

распознавание чисел, строк, boolean-значений;

распознавание операторов;

отслеживание line/column;

обработку ошибок lexer-а.

Sprint 2 — Parser / AST



На этом этапе токены превращаются в синтаксическое дерево — AST.



AST показывает структуру программы: функции, блоки, переменные, выражения, return и т.д.



Распарсить файл и вывести AST в консоль

./build/compiler parse --input examples/hello.src --ast-format text

Сохранить AST в файл

./build/compiler parse --input examples/hello.src --ast-format text --output ast.txt



Посмотреть:



cat ast.txt

Сгенерировать AST в формате Graphviz DOT

./build/compiler parse --input examples/hello.src --ast-format dot --output ast.dot

Превратить DOT в PNG



Для этого нужен Graphviz:



sudo apt install -y graphviz



Команда:



dot -Tpng ast.dot -o ast.png



После этого появится картинка:



ast.png

Что демонстрирует Sprint 2

recursive descent parser;

построение AST;

обработку функций;

обработку блоков;

обработку if/else, while, for;

обработку выражений с приоритетами операторов;

вывод AST в text и DOT.

Sprint 3 — Semantic Analysis / Семантический анализ



На этом этапе компилятор проверяет смысл программы.



Например:



объявлена ли переменная;

нет ли повторного объявления;

совпадают ли типы;

правильно ли вызываются функции;

правильный ли тип возвращает return;

boolean ли условие в if / while.

Запустить semantic check

./build/compiler check --input examples/hello.src

Запустить semantic check с выводом типов

./build/compiler check --input examples/hello.src --show-types

Вывести symbol table

./build/compiler symbols --input examples/hello.src

Сохранить symbol table в файл

./build/compiler symbols --input examples/hello.src --output symbols.txt



Посмотреть:



cat symbols.txt

Что демонстрирует Sprint 3

symbol table;

области видимости;

проверку типов;

проверку функций;

semantic errors;

decorated/type-annotated AST.

Sprint 4 — IR / Intermediate Representation



На этом этапе программа переводится в промежуточное представление — IR.



IR — это форма между AST и assembly.

Она удобна для оптимизаций и дальнейшей генерации машинного кода.



Пример IR-операций:



ADD

SUB

MUL

DIV

CMP\_GT

JUMP

JUMP\_IF

CALL

RETURN

Сгенерировать IR

./build/compiler ir --input examples/factorial.src

Сохранить IR в файл

./build/compiler ir --input examples/factorial.src --output factorial.ir



Посмотреть:



cat factorial.ir

Сгенерировать IR со статистикой

./build/compiler ir --input examples/factorial.src --stats

Сгенерировать IR и проверить control flow

./build/compiler ir --input examples/factorial.src --validate

Полная команда для IR

./build/compiler ir --input examples/factorial.src --stats --validate

Сгенерировать CFG в DOT

./build/compiler ir --input examples/factorial.src --format dot --output cfg.dot

Превратить CFG в PNG

dot -Tpng cfg.dot -o cfg.png

Что демонстрирует Sprint 4

генерацию IR;

basic blocks;

control flow graph;

временные переменные t1, t2, t3;

инструкции ADD, SUB, MUL, CALL, RETURN;

проверку корректности переходов.

Sprint 5 — x86-64 Code Generation / Генерация Assembly



На этом этапе IR переводится в x86-64 assembly.



Backend генерирует:



NASM syntax;

ELF64 object files;

System V AMD64 ABI;

stack frame;

function prologue/epilogue;

вызовы функций;

return values.

Сгенерировать assembly

./build/compiler compile --input tests/codegen/valid/function\_calls/add\_call.src --output add.asm --target x86\_64

Сгенерировать assembly с картой stack frame

./build/compiler compile --input tests/codegen/valid/function\_calls/add\_call.src --output add.asm --target x86\_64 --emit-stack-map

Посмотреть assembly

cat add.asm

Собрать generated assembly в object file

nasm -f elf64 -o add.o add.asm

Собрать runtime library

nasm -f elf64 -o runtime.o src/runtime/runtime.asm

Слинковать executable

ld -o add\_program runtime.o add.o

Запустить программу

./add\_program

echo $?



Для файла:



tests/codegen/valid/function\_calls/add\_call.src



ожидаемый результат:



5



Это значит, что программа реально выполнилась и вернула результат:



2 + 3 = 5

Полная демонстрация Sprint 5



Эти команды можно использовать при защите/проверке:



cd \~/projects/compiler



cmake -S . -B build -DBUILD\_TESTS=ON -G "Unix Makefiles"

cmake --build build

ctest --test-dir build --output-on-failure



./build/compiler compile --input tests/codegen/valid/function\_calls/add\_call.src --output add.asm --target x86\_64 --emit-stack-map



nasm -f elf64 -o add.o add.asm

nasm -f elf64 -o runtime.o src/runtime/runtime.asm

ld -o add\_program runtime.o add.o



./add\_program

echo $?



Ожидаемый вывод:



5

Запуск всех тестов



Все тесты проекта:



ctest --test-dir build --output-on-failure



Ожидаемо:



100% tests passed



В проекте есть тесты для:



lexer;

parser;

semantic analysis;

IR;

codegen.

Полезные быстрые команды

Lexer

./build/compiler lex --input examples/hello.src

Parser

./build/compiler parse --input examples/hello.src --ast-format text

Semantic

./build/compiler check --input examples/hello.src --show-types

IR

./build/compiler ir --input examples/factorial.src --stats --validate

Codegen

./build/compiler compile --input tests/codegen/valid/function\_calls/add\_call.src --output add.asm --target x86\_64 --emit-stack-map

Runtime library



Для Sprint 5 нужен файл:



src/runtime/runtime.asm



Он содержит минимальную runtime library:



\_start;

exit;

print\_int;

print\_string;

read\_int.



Если в .gitignore есть правило:



\*.asm



то файл runtime может не добавиться автоматически.



Добавлять его надо так:



git add -f src/runtime/runtime.asm

