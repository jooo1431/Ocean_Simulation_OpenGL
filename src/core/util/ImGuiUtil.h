#pragma once

#include <cstddef>
#include <iostream>
#include <regex>
#include <string>

#include <imgui.h>

namespace OGL4Core2::Core {
    class ImGuiUtil {
    public:
        template<class E>
        static bool EnumCombo(const char* label, E& item, std::vector<std::pair<E, std::string>> values) {
            static_assert(std::is_enum<E>::value);
            std::string imguiNames;
            int currentItem = 0;
            for (std::size_t i = 0; i < values.size(); ++i) {
                imguiNames += values[i].second + '\0';
                if (item == values[i].first) {
                    currentItem = static_cast<int>(i);
                }
            }
            imguiNames += '\0';
            bool result = ImGui::Combo(label, &currentItem, imguiNames.c_str());
            item = values[currentItem].first;
            return result;
        }
    };
} // namespace OGL4Core2::Core
