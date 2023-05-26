function main() returns (int) {
    struct st {
        int a;
        double b;
    };
    struct st m;
    struct st n;
    m.b = 1.0;
    n = m; // Struct Assignment
    printf("%lf\n", n.b);

    int i = 10;
    i <<= 3; // assign operators
    printf("%d\n", i);
    return 0;
}

// Output:
// 1.000000
// 80