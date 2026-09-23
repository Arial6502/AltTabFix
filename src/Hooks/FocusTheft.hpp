#pragma once
#include "Util/HookUtil.hpp"

namespace Hooks::FocusTheft {

	static inline HWND Window = nullptr;

	struct Win32_SetForeGroundWindow {

		static bool __stdcall thunk(HWND a_hwnd) {
			logger::info("SetForeGroundWindow Proxy Call");
			return true;
		}

		FUNCTYPE_CALL func;
	};

	struct Win32_ShowWindow {

		static bool __stdcall thunk(HWND a_hwnd, int a_cmd) {
			//Replace ShowWindow (a_cmd = 5) with (a_cmd = 4)
			logger::info("ShowWindow Proxy Call");
			Window = a_hwnd;
			return func(a_hwnd, SW_SHOWNOACTIVATE);
		}

		FUNCTYPE_CALL func;
	};

	struct Win32_SetFocus {

		static HWND __stdcall thunk(HWND a_hwnd) {
			//Stub
			logger::info("SetFocus Proxy Call");
			return a_hwnd;
		}

		FUNCTYPE_CALL func;
	};

	//Keyboard and mouse Initialize call SetCooperativeLevel(GetActiveWindow(), ...). The hooks above stop the window
	//from ever activating so GetActiveWindow returns null, SetCooperativeLevel fails and DInput leaves the devices in
	//background mode. Which is why i had to detour the dinput device state funcs due to it reading input in backround somehow. Just give it the game window hwmd from the renderer.
	template <int ID>
	struct Win32_GetActiveWindow {

		static HWND __stdcall thunk() {
			return Window ? Window : func();
		}

		FUNCTYPE_CALL func;
	};

	inline void Install() {

		logger::info("Installing Anti-Focus Steal Hooks");

		//Steal Focus on window create fix
		Hooks::stl::write_call<Win32_SetForeGroundWindow, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x195, 0x25e, NULL));
		Hooks::stl::write_call<Win32_ShowWindow,          6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x184, 0x24d, NULL));
		Hooks::stl::write_call<Win32_SetFocus,            6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x1a6, 0x26f, NULL));

		//BSWin32KeyboardDevice::Initialize, BSWin32MouseDevice::Initialize
		Hooks::stl::write_call<Win32_GetActiveWindow<0>, 6>(REL::RelocationID(67471, 68781, NULL), REL::VariantOffset(0x4B, 0x4B, NULL));
		Hooks::stl::write_call<Win32_GetActiveWindow<1>, 6>(REL::RelocationID(67490, 68801, NULL), REL::VariantOffset(0x65, 0x65, NULL));
		Hooks::stl::write_call<Win32_GetActiveWindow<2>, 6>(REL::RelocationID(67490, 68801, NULL), REL::VariantOffset(0x73, 0x73, NULL));

	}

}
