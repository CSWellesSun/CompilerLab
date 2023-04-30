contract Test {
#include "test.sol"
    uint256 a = 10;
    uint256 b = 20;
    uint256 c = a + b * 0x1234 + (a - b) & 0xFF ; 
    /* Annotation Test. */ 
    string e = "hello world"; // Annotation Test.
    bool f = !( a > b );
    uint128 g = a-- >> c;

    function main(int x, uint y) public pure returns (uint) {
        return 100 ether;
    }
}