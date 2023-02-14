#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "PluginDescriptor.h"

#define REGISTERPLUGIN(classname, idx) \
    [[maybe_unused]] static inline bool is_registered_ = Core::PluginRegister::add<classname>(PLUGIN_DIR, idx);

namespace OGL4Core2::Core {
    class AbstractPluginDescriptor;

    class PluginRegister {
    public:
        PluginRegister() = delete;
        ~PluginRegister() = delete;
        PluginRegister(const PluginRegister&) = delete;
        PluginRegister(PluginRegister&&) = delete;
        PluginRegister& operator=(const PluginRegister) = delete;
        PluginRegister& operator=(PluginRegister&&) = delete;

        template<class C>
        static bool add(const std::string& path, int idx = 1000) {
            // The path string must be the directory path of the plugin directory relative to the `src/plugins`
            // directory with Linux style path separation `/`. It will later be used to load resources from the plugin
            // directory. Each plugin source file has PLUGIN_DIR defined from CMake to get this path. It is also
            // automatically used when using the REGISTERPLUGIN macro above.
            plugins_.emplace_back(std::make_shared<PluginDescriptor<C>>(C::name(), path, idx));
            std::sort(plugins_.begin(), plugins_.end(),
                [](const auto& a, const auto& b) -> bool { return a->idx() < b->idx(); });
            return true;
        }

        [[nodiscard]] static std::shared_ptr<AbstractPluginDescriptor> get(std::size_t i) {
            if (i >= plugins_.size()) {
                throw std::out_of_range("Invalid plugin index!");
            }
            return plugins_[i];
        }

        [[nodiscard]] static inline auto empty() {
            return plugins_.empty();
        }
        [[nodiscard]] static inline auto size() {
            return plugins_.size();
        }
        [[nodiscard]] static inline const auto& getAll() {
            return plugins_;
        }

    private:
        static inline std::vector<std::shared_ptr<AbstractPluginDescriptor>> plugins_;
    };
} // namespace OGL4Core2::Core
