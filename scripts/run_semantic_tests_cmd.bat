@echo off
setlocal
cl /std:c++17 /EHsc /I src src\lexer\token.cpp src\lexer\lexer.cpp src\parser\ast.cpp src\parser\parser.cpp src\semantic\type_system.cpp src\semantic\symbol_table.cpp src\semantic\errors.cpp src\semantic\analyzer.cpp tests\semantic_tests.cpp /Fe:semantic_tests.exe
if errorlevel 1 exit /b 1
semantic_tests.exe
endlocal
