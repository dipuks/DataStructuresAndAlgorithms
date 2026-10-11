#include <array>
#include <iostream>

#include "dsa/arrays_hashing.hpp"

int main()
{
    using dsa::arrays_hashing::ArraysHashing;

    constexpr std::array numbers{2, 7, 11, 15};
    if (const auto indices = ArraysHashing::twoSum(numbers, 9)) {
        std::cout << "Two Sum indices: [" << indices->first << ", "
                  << indices->second << "]\n";
    }

    constexpr std::array values{1, 2, 3, 1};
    std::cout << std::boolalpha
              << "Contains duplicate: "
              << ArraysHashing::containsDuplicate(values) << '\n'
              << "Is anagram: "
              << ArraysHashing::isAnagram("anagram", "nagaram") << '\n';
}