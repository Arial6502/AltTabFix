#pragma once
#include "SimpleIni.h"

namespace Config {

    struct _AltTabFix {
        bool bEnable = true;
    };

    struct _FocusTheft {
        bool bEnable = false;
    };

    struct _Misc {
        bool bEnableSnippingToolForward = false;
    };

    class ConfigManager final {

        public:
        static void Initialize();

        static inline _AltTabFix AltTabFix = {};
        static inline _FocusTheft FocusTheft = {};
        static inline _Misc Misc = {};

        private:
        static inline CSimpleIniA INI = {};

    };

}