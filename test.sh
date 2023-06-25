#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./compiler "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0  'int main(){0;}'
assert 42 'int main(){42;}'

assert 41 'int main(){ 12 + 34 - 5 ; }'

assert 47 'int main(){5+6*7;}'
assert 15 'int main(){5*(9-6);}'
assert 4  'int main(){(3+5)/2;}'

assert 10 'int main(){-10+20;}'
assert 8  'int main(){-(-(3+5));}'
assert 15 'int main(){-3*-5;}'

assert 1 'int main(){1-1<1;}'
assert 0 'int main(){1+0<1;}'
assert 0 'int main(){1+1<1;}'

assert 1 'int main(){1-1<=1;}'
assert 1 'int main(){1+0<=1;}'
assert 0 'int main(){1+1<=1;}'

assert 0 'int main(){-1-1>-1;}'
assert 0 'int main(){-1+0>-1;}'
assert 1 'int main(){-1+1>-1;}'

assert 0 'int main(){-1-1>=-1;}'
assert 1 'int main(){-1+0>=-1;}'
assert 1 'int main(){-1+1>=-1;}'

assert 1 'int main(){35==35;}'
assert 0 'int main(){35==-35;}'

assert 1 'int main(){+24!=-24;}'
assert 0 'int main(){+24!=+24;}'

assert 12 'int main() {
    int a;
    a = 5;

    int ab;
    ab = 2;
    ab = ab + 1;

    int b;
    b = 2;

    int c;
    c = ab + b + 3;
    return c + 4;

    c;
}'

assert 3 'int main() {
    int a;
    a = 0;

    int b;
    b = 0;

    int c;
    c = 0;

    if (a == 0) {
        b = b + 3;
    }

    if (a != 0) {
        c = c + 4;
    }

    return b + c;
}'

assert 8 'int main() {
    int a;
    a = 1;

    int b;
    b = 3;

    if (a) {
        int a;
        a = 4;
        b = b + a;
    }

    return b + a;
}'

assert 9 'int sub() {
    int a;
    a = 0;
    return 9;
}

int main() {
    return sub();
}'

assert 10 'int add(int a, int b) {
    return a + b;
}

int main() {
    add(7, 3);
}'

assert 3 'int f(int x) {
    if (x) {
        return 1;
    } else {
        return 0;
    }
}

int main() {
    int a;
    a = 0;
    if (f(1)) a = a + 3;
    if (f(0)) a = a + 4;
    return a;
}'

assert 55 'int fibonacci(int n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int main() {
    return fibonacci(10);
}'

echo OK
