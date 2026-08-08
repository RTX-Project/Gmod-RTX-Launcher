// main_test.cpp
// Simple test file for main.cpp functionality

#include <string>
#include <vector>
#include <cassert>

// Forward declarations of functions to test
std::string WStringToUTF8(const std::wstring& wstr);
std::wstring UTF8ToWString(const std::string& str);

void test_string_conversion() {
    std::wstring original = L"Hello, World! Привет, Мир!";
    std::string utf8 = WStringToUTF8(original);
    std::wstring back = UTF8ToWString(utf8);
    assert(original == back);
}

int main() {
    // Run tests
    // test_string_conversion();
    return 0;
}
