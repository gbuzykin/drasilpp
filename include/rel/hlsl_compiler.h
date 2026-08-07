#pragma once

#include "data_blob.h"

#include <uxs/db/value.h>

namespace drasilpp::rel {

class DRASILPP_REL_EXPORT HlslCompiler {
 public:
    HlslCompiler();
    ~HlslCompiler();

    DataBlob compileShader(const DataBlob& source_text, const uxs::db::value& args, DataBlob& compiler_output);
    void setPlatformArgs(const uxs::db::value& platform_args);

 private:
    class Implementation;
    std::unique_ptr<Implementation> impl_;
};

}  // namespace drasilpp::rel
