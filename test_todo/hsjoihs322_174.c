struct A{int a; int b; int *q; int *t; int *p;}; struct A f(void) {struct A u; u.a = 100;return u;} int main(void){struct A u = f(); return u.a + 74;}
