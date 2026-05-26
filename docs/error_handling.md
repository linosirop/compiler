# Error Handling

Sprint 8 adds a small unified diagnostic helper in `src/utils/diagnostics.*`.

The target diagnostic format is:

```text
file.src:12:5: error E042: undefined variable 'x'
   |
 12 |     return x;
   |            ^
note: declare the variable before use
1 error, 0 warnings generated.
```

Existing compiler stages still keep their original reports, while the final CLI uses the diagnostic helper for command-line and top-level compiler errors.
