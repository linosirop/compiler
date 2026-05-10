cl /std:c++17 /EHsc /I src `
  src\lexer\token.cpp `
  src\lexer\lexer.cpp `
  src\parser\ast.cpp `
  src\parser\parser.cpp `
  tests\core_tests_extended.cpp `
  /Fe:core_tests_extended.exe

if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
.\core_tests_extended.exe
