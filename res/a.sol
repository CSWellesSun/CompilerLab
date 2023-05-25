function main() returns () {
    double a = 3.0;
    a = a++ + 3.0;
    a = ++a + 3.0;

    int i = 0;
    do {
        ++i;
        a = a + i;
    } while ( i < 10 );
}