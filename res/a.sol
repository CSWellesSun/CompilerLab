struct person {
    string name;
    int age;
    double height;
};
function main() returns (int) {
    struct person p1;
    p1.name = "Suzumiya Haruhi";
    p1.age = 16;
    p1.height = 158.0;
    struct person copy = p1;
    printf("name = %s\nage = %d\nheight = %.2f\n", copy.name, copy.age, copy.height);
    return 0;
}
