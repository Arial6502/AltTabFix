#include "Config.hpp"
#include "Version.hpp"

namespace {

    // Helpers
    inline bool ToBool(const char* v, bool def) {
        if (!v) return def;
        std::string s = v;
        if (s == "1" || s == "true" || s == "True" || s == "on") return true;
        if (s == "0" || s == "false" || s == "False" || s == "off") return false;
        return def;
    }

    // Loaders
    void LoadAltTabFix(CSimpleIniA& ini, Config::_AltTabFix& cfg) {
        cfg.bEnable = ToBool(ini.GetValue("AltTabFix", "bEnable", cfg.bEnable ? "True" : "False"), cfg.bEnable);
    }

    void LoadFocusTheft(CSimpleIniA& ini, Config::_FocusTheft& cfg) {
        cfg.bEnable = ToBool(ini.GetValue("FocusTheft", "bEnable", cfg.bEnable ? "True" : "False"), cfg.bEnable);
    }

    void LoadMisc(CSimpleIniA& ini, Config::_Misc& cfg) {
        cfg.bEnableSnippingToolForward = ToBool(
            ini.GetValue("Misc", "bEnableSnippingToolForward", cfg.bEnableSnippingToolForward ? "True" : "False"),
            cfg.bEnableSnippingToolForward
        );
    }
}

namespace Config {

    void ConfigManager::Initialize() {

        const std::string fileName = R"(Data\SKSE\Plugins\)" + std::string(Plugin::ModName) + ".ini";

        INI.SetUnicode();
        SI_Error rc = INI.LoadFile(fileName.c_str());

        if (rc == SI_FILE) {
            logger::warn("Config file not found, creating...");

            // write default values
            INI.SetValue("AltTabFix", "bEnable", AltTabFix.bEnable ? "True" : "False");
            INI.SetValue("FocusTheft", "bEnable", FocusTheft.bEnable ? "True" : "False");
            INI.SetValue("Misc", "bEnableSnippingToolPassthrough", Misc.bEnableSnippingToolForward ? "True" : "False");

            INI.SaveFile(fileName.c_str());
        }

        // always load into structs (either defaults or from file)
        LoadAltTabFix(INI, AltTabFix);
        LoadFocusTheft(INI, FocusTheft);
        LoadMisc(INI, Misc);

        logger::info("Config Loaded");
        logger::info("AltTabFix: bEnable:{}", AltTabFix.bEnable);
        logger::info("FocusTheft: bEnable:{}", FocusTheft.bEnable);
        logger::info("Misc: bEnableSnippingToolPassthrough:{}", Misc.bEnableSnippingToolForward);

    }
}
