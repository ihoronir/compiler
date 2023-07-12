char a;char foo(char *p){*p = 5; a = 3; return a;} int main(){char q; char r; r = foo(&q); return 172-r+q;}
