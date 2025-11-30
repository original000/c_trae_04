#include <ucontext.h>
#include <iostream>

using namespace std;

int main() {
    ucontext_t ctx;
    getcontext(&ctx);
    cout << "ucontext is available!" << endl;
    return 0;
}
