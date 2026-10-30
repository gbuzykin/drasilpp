#pragma once

#include "common/logger.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#define LOG_VK "Vulkan: "

namespace drasilpp::rel::vulkan {
std::string_view getVkResultText(VkResult result);
}

template<typename CharT>
struct uxs::formatter<VkResult, CharT> {
 public:
    template<typename ParseCtx>
    constexpr typename ParseCtx::iterator parse(ParseCtx& ctx) {
        auto it = ctx.begin();
        return it == ctx.end() || *it != ':' ? it : it + 1;
    }
    template<typename FmtCtx>
    void format(FmtCtx& ctx, VkResult val) const {
        const auto text = drasilpp::rel::vulkan::getVkResultText(val);
        if (!text.empty()) {
            ctx.out() += text;
        } else {
            to_string_append(ctx.out(), int(val));
        }
    }
};
