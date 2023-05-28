function foo(int day) returns () {
    printf("%d\n", day);
}

function main() returns (int) {
    int day = 5;
    foo(day);

    return 0;
}
