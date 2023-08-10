int foo(void) { return 3; }
int bar(void) { return 5; }
int main(void) { return (1 ? foo : bar)(); }
