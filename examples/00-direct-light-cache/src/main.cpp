#include <osi/foray_env.hpp>
#include <vector>
#include "IrradianceCacheApp.h"

namespace irradiance_cache {
    int example(std::vector<std::string> &args) {
        foray::osi::OverrideCurrentWorkingDirectory(CWD_OVERRIDE);
        IrradianceCacheApp app;
        return app.Run();
    }
}

int main(int argv, char **args) {
    std::vector<std::string> argvec(argv);
    for (int i = 0; i < argv; i++) {
        argvec[i] = args[i];
    }
    return irradiance_cache::example(argvec);
}
