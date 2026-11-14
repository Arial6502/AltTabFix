#pragma once
#include "Util/HookUtil.hpp"

namespace Hooks::SnippingTool {

	//The previous WNDPROC method worked unreliably due mods (this one included) being able to mess with the flags given to windows.
	//Just grab the keystate from the the keyboard device directly instead.
	struct Keyboard_Poll {

		static constexpr std::size_t funcIndex = 0x2;

		static void thunk(RE::BSWin32KeyboardDevice* a_this, float a_timeDelta) {

			func(a_this, a_timeDelta);

			const auto& data = a_this->GetRuntimeData();

			const auto down = [&](REX::W32::DIK a_key) {
				return (data.curState[a_key] & 0x80) != 0;
			};

			const bool sPressed = down(REX::W32::DIK_S) && !(data.prevState[REX::W32::DIK_S] & 0x80);

			if (sPressed && (down(REX::W32::DIK_LWIN) || down(REX::W32::DIK_RWIN)) && (down(REX::W32::DIK_LSHIFT) || down(REX::W32::DIK_RSHIFT))) {
				//logger::info("Launch Snipping Tool");
				ShellExecuteW(nullptr, L"open", L"ms-screenclip:", nullptr, nullptr, SW_SHOWNORMAL);
			}
		}

		FUNCTYPE_VFUNC func;

	};

	inline void Install() {

		logger::info("Installing \"SnippingTool Passthrough Fix\" Hooks...");
		Hooks::stl::write_vfunc<RE::BSWin32KeyboardDevice, Keyboard_Poll>();

	}

}
