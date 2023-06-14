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

assert 0  'main(){0;}'
assert 42 'main(){42;}'

assert 41 'main(){ 12 + 34 - 5 ; }'

assert 47 'main(){5+6*7;}'
assert 15 'main(){5*(9-6);}'
assert 4  'main(){(3+5)/2;}'

assert 10 'main(){-10+20;}'
assert 8  'main(){-(-(3+5));}'
assert 15 'main(){-3*-5;}'

assert 1 'main(){1-1<1;}'
assert 0 'main(){1+0<1;}'
assert 0 'main(){1+1<1;}'

assert 1 'main(){1-1<=1;}'
assert 1 'main(){1+0<=1;}'
assert 0 'main(){1+1<=1;}'

assert 0 'main(){-1-1>-1;}'
assert 0 'main(){-1+0>-1;}'
assert 1 'main(){-1+1>-1;}'

assert 0 'main(){-1-1>=-1;}'
assert 1 'main(){-1+0>=-1;}'
assert 1 'main(){-1+1>=-1;}'

assert 1 'main(){35==35;}'
assert 0 'main(){35==-35;}'

assert 1 'main(){{-12!=12;+24==24;}{-12!=12;+24==24;}}su(){}'
#assert 0 '+24!=+24;'

# assert 10 'a = 0;
# ab = ab + 1;
# b = 2;
# c = ab + b + 3;
# return c + 4;
# c;'

echo OK
