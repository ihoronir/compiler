char g;

char f() { return 0; }

int main() {
    char a[3];

    if (sizeof g != 1) return 1;
    if (sizeof f() != 1) return 1;
    return sizeof a;
}
