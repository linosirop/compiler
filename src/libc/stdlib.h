// MiniCompiler libc binding notes for Sprint 7.
// Supported external declarations use MiniCompiler syntax, for example:
//   extern int printf(string format, int value);
// These names are emitted as external NASM symbols and linked with gcc -no-pie.
extern int printf(string format, int value);
extern int puts(string text);
extern int getchar();
extern int strlen(string text);
