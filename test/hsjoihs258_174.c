int main(){int x = 86;int *y = &x;int **z = &y;return (*y) + (**z) + 2;}
