#include "sampler.h"

#include "device.h"
#include "tables.h"

using namespace drasilpp;
using namespace drasilpp::rel;
using namespace drasilpp::rel::d3d12;

// --------------------------------------------------------
// Sampler class implementation

Sampler::Sampler(std::shared_ptr<Device> device) : device_(std::move(device)) {}

Sampler::~Sampler() {}

bool Sampler::create(const SamplerDesc& desc) {
    desc_ = {
        .Filter = TBL_D3D12_FILTER[unsigned(desc.filter)],
        .AddressU = TBL_D3D12_ADDRESS_MODE[unsigned(desc.address_mode_u)],
        .AddressV = TBL_D3D12_ADDRESS_MODE[unsigned(desc.address_mode_u)],
        .AddressW = TBL_D3D12_ADDRESS_MODE[unsigned(desc.address_mode_u)],
        .MipLODBias = FLOAT(desc.mip_lod_bias),
        .MaxAnisotropy = desc.filter >= SamplerFilter::ANISOTROPIC ? 1U : UINT(desc.max_anisotropy),
        .ComparisonFunc = D3D12_COMPARISON_FUNC_NONE,
        .MinLOD = FLOAT(desc.min_lod),
        .MaxLOD = FLOAT(desc.max_lod),
    };

    return true;
}

//@{ ISampler

//@}
