$compiler = ".\out\build\x64-Debug\compiler.exe"

Write-Host "=== VALID TESTS ==="

Get-ChildItem "tests/parser/valid" -Filter *.src | ForEach-Object {
    Write-Host "Running $($_.Name)..."

    $result = & $compiler parse --input $_.FullName --ast-format text 2>&1

    if ($result -match "Syntax error") {
        Write-Host "❌ FAIL (unexpected error)"
    } else {
        Write-Host "✅ PASS"
    }
}

Write-Host "`n=== INVALID TESTS ==="

Get-ChildItem "tests/parser/invalid" -Filter *.src | ForEach-Object {
    Write-Host "Running $($_.Name)..."

    $result = & $compiler parse --input $_.FullName --ast-format text 2>&1

    if ($result -match "Syntax error") {
        Write-Host "✅ PASS (error detected)"
    } else {
        Write-Host "❌ FAIL (no error)"
    }
}