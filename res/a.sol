function main() returns (int) {
    struct st {
        int a;
        double b;
    };
    struct st m;
    m.b = 1.0;
    printf("%lf\n", m.b);
    return 0;
}