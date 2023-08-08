int foo(void) { return 3; } int bar(void) { return 5;} int main(void) { int (*foo1)(void) = foo; int (*bar1)(void) = bar; return (1? foo1 : bar1)(); }
