#pragma once

namespace foray::irradiance_cache {

    enum class IrradianceCacheMode {
        INDIRECT_TRACE_DIRECT_TRACE,
        INDIRECT_IC_DIRECT_TRACE,
        INDIRECT_IC,
        DIRECT_TRACE,
        DIRECT_IC,
        DEBUG_PATTERN,
        MAX_ENUM,
        DEFAULT = INDIRECT_IC_DIRECT_TRACE,
    };

    inline bool skipIC(IrradianceCacheMode mode) {
        return mode == IrradianceCacheMode::INDIRECT_TRACE_DIRECT_TRACE || mode == IrradianceCacheMode::DIRECT_TRACE;
    }

}
