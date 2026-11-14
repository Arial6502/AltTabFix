#pragma once
#include "Util/HookUtil.hpp"

namespace Hooks::SnippingTool {

	struct WndProcHandler {

		static LRESULT thunk(HWND a_hwnd, UINT a_msg, WPARAM a_wParam, LPARAM a_lParam) {

			switch (a_msg) {

				// "Fixes" The game preventing usage of the snipping tool shortcut if it currently has kb focus.
				case WM_SYSKEYDOWN:
				{
					const bool winDown = (GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000);
					const bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

					if (winDown && shiftDown && a_wParam == 'S') {

						// Directly launch Windows Snip & Sketch
						ShellExecuteW(0, L"open", L"ms-screenclip:", NULL, NULL, SW_SHOWNORMAL);

						return 0;
					}

					break;

				}
			}

			return func(a_hwnd, a_msg, a_wParam, a_lParam);
		}

		FUNCTYPE_CALL func;
	};

	struct Win32_RegisterClassA {

		static WORD thunk(WNDCLASSA* a_wndClass) {

			WndProcHandler::func = reinterpret_cast<uintptr_t>(a_wndClass->lpfnWndProc);
			a_wndClass->lpfnWndProc = &WndProcHandler::thunk;

			return func(a_wndClass);
		}

		FUNCTYPE_CALL func;
	};

	inline void Install() {

		logger::info("Installing \"SnippingTool Passthrough Fix\" Hooks...");

		Hooks::stl::write_call<Win32_RegisterClassA, 6>(REL::RelocationID(75591, 77226, NULL), REL::VariantOffset(0x8E, 0x15C, NULL));

		//BSWin32KeyboardDevice::Initialize: SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY).
		//NOWINKEY swallows the Win key so GetAsyncKeyState never sees it. This used to only work because SetCooperativeLevel
		//failed with FocusTheft on (GetActiveWindow returned null). Drop NOWINKEY: mov r8d, 0x15 -> 0x05.
		REL::safe_write(REL::RelocationID(67471, 68781, NULL).address() + 0x57, static_cast<std::uint8_t>(0x05));

	}

}