struct A{int a;}; struct A f(void) {struct A u; u.a = 174; return u;} int main(void){struct A u = f(); return u.a;}
