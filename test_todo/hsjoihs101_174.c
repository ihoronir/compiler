int *foo(int *p){*p = 4;return p;} int main(){int x;int *y;y = foo(&x); *y+= 170;return x;}
