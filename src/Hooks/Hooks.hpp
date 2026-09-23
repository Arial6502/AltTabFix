#pragma once

#include "AltTabFix.hpp"
#include "FocusTheft.hpp"
#include "SnippingTool.hpp"

namespace Hooks {

	inline void Install() {

		auto& Trampoline = SKSE::GetTrampoline();
		Trampoline.create(56);

		if (Config::ConfigManager::AltTabFix.bEnable)
			AltTabFix::Install();

		if (Config::ConfigManager::FocusTheft.bEnable)
			FocusTheft::Install();

		if (Config::ConfigManager::Misc.bEnableSnippingToolForward)
			SnippingTool::Install();

	}
}