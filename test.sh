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

assert 1 'int main(){{-12!=12;+24==24;}{-12!=12;+24==24;}}int su(){}'
#assert 0 '+24!=+24;'

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

echo OK
