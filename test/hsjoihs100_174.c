int foo(int* p){*p = 172; return *p+2;} int main(){int x; return foo(&x);}
