struct A {int k[15];}; struct A f(int a, int b){struct A q; q.k[0] = a; q.k[14] = b; return q;} int main(){struct A (*g)(int a, int b) = f; struct A q = g(10, 11); return q.k[0] + q.k[14]; }
