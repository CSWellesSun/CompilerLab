#include <iostream>
#include <iomanip>

using namespace std;

// 自定义setw_n函数，打印n个空格
ostream& setw_n(ostream& os, int n)
{
    for(int i = 0; i < n; i++)
    {
        os << " ";
    }
    return os;
}

int main()
{
    int n = 10;
    cout << setw_n(n) << "Hello, world!" << endl;

    return 0;
}
