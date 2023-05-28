function main() returns (int) {
    int i = 0;
    int a = 0;
    do {
        ++i;
        a = a + i;
    } while ( i < 10 );
    printf("%d\n", a);

    i = 0;
    a = 0;*/
    while (i < 10) {
        ++i;
        a = a + i;
    }
    printf("%d\n", a);

    a = 0;
    for (i = 0; i < 10; ++i) {
        a += i;
    }
    printf("%d\n", a);

    return 0;
}
