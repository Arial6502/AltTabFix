#include "Config/Config.hpp"
#include "Hooks/Hooks.hpp"
#include "Version.hpp"

#include "Util/Logger/Logger.hpp"

SKSEPluginLoad(const SKSE::LoadInterface * a_skse) {

	//You never know...
	REL::Module::reset();

	SKSE::Init(a_skse);
	logger::Initialize();
	logger::SetLevel(spdlog::level::info);
	Config::ConfigManager::Initialize();
	Hooks::Install();

	return true;
}

SKSEPluginInfo(
	.Version = Plugin::ModVersion,
	.Name = Plugin::ModName,
	.Author = "Arial6502",
	.StructCompatibility = SKSE::StructCompatibility::Independent,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
);


