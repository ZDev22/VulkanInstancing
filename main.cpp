#include "app.hpp"
#include "compileShader.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace std;

short scene = 1;

uint32_t state = 123456789;
float normalized;

uint32_t xorshift32() {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

float randomNumber(float min, float max) {
    normalized = xorshift32() / static_cast<float>(UINT32_MAX);
    return min + (max - min) * normalized;
}

int main() {
    if (compile()) {
        vulkan::App app{};

        try {
            app.run();
        }
        catch (const exception& e) {
            cerr << e.what() << '\n';
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else { return 0; }
}
