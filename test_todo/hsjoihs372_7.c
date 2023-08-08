void foo(int*p) {*p=3;} int main() { int a; if (0) { foo(&a); } else { a = 7; } return a; }
