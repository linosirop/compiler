@echo off
setlocal
cl /std:c++17 /EHsc /I src ^
  src\lexer\token.cpp ^
  src\lexer\lexer.cpp ^
  src\parser\ast.cpp ^
  src\parser\parser.cpp ^
  src\semantic\type_system.cpp ^
  src\semantic\symbol_table.cpp ^
  src\semantic\errors.cpp ^
  src\semantic\analyzer.cpp ^
  src\ir\ir_instructions.cpp ^
  src\ir\basic_block.cpp ^
  src\ir\control_flow.cpp ^
  src\ir\ir_generator.cpp ^
  tests\ir_tests.cpp ^
  /Fe:ir_tests.exe
if errorlevel 1 exit /b 1
ir_tests.exe
