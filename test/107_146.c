int f(int a, int b) {
    int n;
    n = 0;
    if (a == 1) {
        n = n + 1;
    } else if (a == 2) {
        n = n + 3;
    } else if (a == 3) {
        n = n + 5;
    } else {
        n = n + 7;
    }

    int m;
    m = 0;
    if (b == 1) {
        m = m + 10;
    } else {
        if (b == 2) {
            m = m + 30;
        } else if (b == 3) {
            m = m + 50;
        }
    }
    m = m + 10;

    return n + m;
}

int main() { return f(1, 1) + f(2, 2) + f(3, 3) + f(4, 4); }
