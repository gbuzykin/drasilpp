#pragma once

#include "d3d12_api.h"

#include "interfaces/i_rendering_driver.h"

namespace drasilpp::rel::d3d12 {

class Surface final : public ISurface {
 public:
    explicit Surface(std::shared_ptr<RenderingDriver> instance);
    ~Surface() override;

    const WindowDescriptor& getWindowDescriptor() const { return win_desc_; }

    bool create(const WindowDescriptor& win_desc);

    RenderingDriver& getInstance() { return *instance_; }

    //@{ ISurface
    //@}

 private:
    std::shared_ptr<RenderingDriver> instance_;
    WindowDescriptor win_desc_;
};

}  // namespace drasilpp::rel::d3d12
