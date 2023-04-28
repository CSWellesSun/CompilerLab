contract Test {

    uint256 a;
    uint128 b = 0xDEADBEEF; /* Annotation Test. */
    string c = "hello world"; // Annotation Test.
    bool d = true;

    function main(int x, uint y) public pure bool returns (uint) {
        return 100 ether;
    }
}