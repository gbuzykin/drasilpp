#include "surface.h"

#include "rendering_driver.h"

using namespace drasilpp;
using namespace drasilpp::rel;
using namespace drasilpp::rel::d3d12;

// --------------------------------------------------------
// Surface class implementation

Surface::Surface(std::shared_ptr<RenderingDriver> instance) : instance_(std::move(instance)) {}

Surface::~Surface() {}

bool Surface::create(const WindowDescriptor& win_desc) {
    win_desc_ = win_desc;
    return true;
}

//@{ ISurface

//@}
