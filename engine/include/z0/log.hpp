#pragma once

#include <concepts>
#include <string_view>
#include <iostream>
#include <sstream>

namespace z0 {

    void log(std::convertible_to<std::string_view> auto&& ...s) {
        for (auto v : std::initializer_list<std::string_view>{ s... }) {
            std::cout << v << " ";
        }
        std::cout << std::endl;
    }

    void die(std::convertible_to<std::string_view> auto&& ...s) {
        std::stringstream stringstream;
        for (auto v : std::initializer_list<std::string_view>{ s... }) {
            stringstream << v << " ";
        }
        throw std::runtime_error(stringstream.str());
    }

}