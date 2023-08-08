int *foo(int *p){*p = 4;return p;} int main(){int x;int y; int **z; *foo(&x) += 170;return x;}
