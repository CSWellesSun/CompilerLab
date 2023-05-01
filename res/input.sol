contract Test {
    /* Annotation Test. */ 
    string e = "hello world"; // Annotation Test.
    bool f = !( a > b );
    uint128 g = a-- >> c;

    function main(int x, uint y) public pure returns (uint) {
        f >>>= 3;
        if (a != 3) {
            a = 3;
        } else {
            a = 4;
        }
        for (int i = 0; i < 3; i++) {
            a += 3;
            break;
        }
        while (a < 3) {
            a += 3;
            continue;
        }
        do {
            a += 3;
        } while (a < 3); 
        uint256 a = 3;
        return 100;
    }
}