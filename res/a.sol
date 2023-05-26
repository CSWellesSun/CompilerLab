function main() returns (int) {
    int a[10];
    a[0] = 1;
    a[1] = 1;
    for (int i = 2; i < 10; ++i) {
        a[i] = a[i-1] + a[i-2];
    }
    for (int i = 0; i < 10; ++i) {
        printf("%d ", a[i]);
    }
    printf("\n");
    return 0;
}