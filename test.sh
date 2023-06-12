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

assert 0  'a=0;'
assert 42 'a=42;'

assert 41 ' a = 12 + 34 - 5 ; '

assert 47 'a = 5+6*7;'
assert 15 'a = 5*(9-6);'
assert 4  'a = (3+5)/2;'

assert 10 'a = -10+20;'
assert 8  'a = -(-(3+5));'
assert 15 'a = -3*-5;'

assert 1 'a = 1-1<1;'
assert 0 'a = 1+0<1;'
assert 0 'a = 1+1<1;'

assert 1 'a = 1-1<=1;'
assert 1 'a = 1+0<=1;'
assert 0 'a = 1+1<=1;'

assert 0 'a = -1-1>-1;'
assert 0 'a = -1+0>-1;'
assert 1 'a = -1+1>-1;'

assert 0 'a = -1-1>=-1;'
assert 1 'a = -1+0>=-1;'
assert 1 'a = -1+1>=-1;'

assert 1 'a = 35==35;'
assert 0 'a = 35==-35;'

assert 1 'a = -12!=12;'
assert 0 'a = +24!=+24;'

assert 10 '
a = 0;
a = a + 1;
b = @2;
c = a + b + 3;
return c + 4;
c;
'

echo OK
