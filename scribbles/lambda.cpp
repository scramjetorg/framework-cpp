#include <iostream>
#include <string>

template<typename T, typename S>
using L = S (*)(T);

void abssort() {
    L<int,std::string> glambda = [](int a) { return std::to_string(a); };
}

int main(int argc, char const *argv[])
{
    abssort();
    
    return 0;
}
