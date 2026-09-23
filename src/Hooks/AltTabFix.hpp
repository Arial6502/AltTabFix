#pragma once
#include "Util/HookUtil.hpp"

namespace Hooks::AltTabFix {

	constexpr std::uint32_t gDInputEventSize = sizeof(REX::W32::DIDEVICEOBJECTDATA);

	//Main::Update Has an async key check that early bails out of the loop if both alt and tab are pressed.
	static void RemoveAsyncKeyCheck() {

		logger::trace("Removing Main::Update Key check");
		//1.7.99 = 0x140658610 - 0x140658671 = 0x61, Unchanged luckily
		REL::Relocation<std::uintptr_t> jmp{ REL::VariantID(35565, 36564, NULL), REL::VariantOffset(0x46, 0x61, NULL) };

		// disable jmp.
		REL::safe_fill(jmp.address(), 0x90, 6);
	}

	//The keyboard uses buffered DInput so the game only ever gets key events instead of the actual key state.
	//If the device got unacquired (Acquire doesn't return S_FALSE) or the buffer overflows this will result in missed events
	//causing curState to be wrong. So we clear it and let SyncHeldButtons send the key ups.
	static void DropLostKeyboardState(RE::BSWin32KeyboardDevice* a_keyboard) {

		const auto device = reinterpret_cast<REX::W32::IDirectInputDevice8A*>(a_keyboard->GetRuntimeData().dInputDevice);
		if (!device) {
			return;
		}

		constexpr REX::W32::HRESULT kAlreadyAcquired = 1;  //S_FALSE
		constexpr REX::W32::HRESULT kBufferOverflow = 1;   //DI_BUFFEROVERFLOW

		const auto acquired = device->Acquire();

		static bool unacquired = false;
		if (acquired < 0) {
			if (std::exchange(unacquired, true)) {
				return;
			}
		}
		else {
			unacquired = false;
			if (acquired == kAlreadyAcquired) {
				//Null buffer with a count of 0 only checks for an overflow
				std::uint32_t count = 0;
				if (device->GetDeviceData(gDInputEventSize, nullptr, &count, 0) != kBufferOverflow) {
					return;
				}
			}
		}

		logger::debug("Keyboard lost DInput events, Acquire 0x{:X}", static_cast<std::uint32_t>(acquired));
		a_keyboard->ClearInputState();
	}
	
	//Vanilla calls this on WM_ACTIVATE through ResetInputDevices. At that point the keyboard is usually still acquired
	//and the tab from the alt+tab is still sitting in the DInput buffer. The reset clears alt from curState, so on the
	//next poll the tab gets past the alt+tab check in Poll and opens the tween menu.
	//Anything still in the buffer is from before the reset anyway, so just flush it.
	//This fixes the tween menu still poping up in an unmodded game. 
	//Which itself only happens due to a race because the unmodded game ran at like 2000fps when testing.
	struct Keyboard_ClearInputState {

		static void thunk(RE::BSWin32KeyboardDevice* a_this) {

			if (const auto device = reinterpret_cast<REX::W32::IDirectInputDevice8A*>(a_this->GetRuntimeData().dInputDevice)) {
				//Null buffer with an INFINITE count -> flushes everything according to ms docs.
				std::uint32_t count = std::numeric_limits<std::uint32_t>::max();
				device->GetDeviceData(gDInputEventSize, nullptr, &count, 0);
			}

			func(a_this);
		}

		FUNCTYPE_VFUNC func;
		static constexpr std::size_t funcIndex = 0x8;
	};

	//a_held is every button this device sent a pressed event for that hasn't been released yet.
	//If the device stops reporting one of them we release it through SetButtonState like a normal key up. That zeroes
	//heldDownSecs and sends the up event, so BSInputDevice::IsPressed and anything hooked into the event dispatch agree.
	//Mostly needed because ResetInputDevices (which the game itself calls called on its wndproc WM_ACTIVATE) wipes every device's raw state without releasing anything.
	static void SyncHeldButtons(std::vector<std::uint32_t>& a_held, RE::INPUT_DEVICE a_type, RE::BSInputDevice* a_device, std::uint32_t a_firstEvent) {

		const auto queue = RE::BSInputEventQueue::GetSingleton();
		const std::uint32_t lastEvent = queue->buttonEventCount;
		const auto contains = [](const std::vector<std::uint32_t>& a_ids, std::uint32_t a_id) {
			return std::ranges::find(a_ids, a_id) != a_ids.end();
		};

		std::vector<std::uint32_t> reported;
		for (auto i = a_firstEvent; i < lastEvent; ++i) {
			const auto& event = queue->GetRuntimeData().buttonEvents[i];
			if (event.GetDevice() != a_type) {
				continue;
			}
			const auto id = event.GetIDCode();
			std::erase(a_held, id);
			if (event.IsPressed()) {
				a_held.push_back(id);
				reported.push_back(id);
			}
		}

		//Queue is full so some events might have been dropped this update. Can't tell what's stale or not so try again at the next update
		if (lastEvent >= RE::BSInputEventQueue::MAX_BUTTON_EVENTS) {
			return;
		}

		std::vector<std::uint32_t> stale;
		for (const auto id : a_held) {
			if (!contains(reported, id)) {
				stale.push_back(id);
			}
		}

		//Also check heldDownSecs in case the pressed event got dropped because the queue was full.
		if (a_device) {
			for (const auto& button : a_device->GetRuntimeData().deviceButtons) {
				if (button.second->heldDownSecs > 0.0f && !contains(reported, button.first) && !contains(stale, button.first)) {
					stale.push_back(button.first);
				}
			}
		}

		for (const uint32_t id : stale) {

			if (queue->buttonEventCount >= RE::BSInputEventQueue::MAX_BUTTON_EVENTS) {
				return;
			}

			logger::debug("Releasing stale button 0x{:X} on device {}", id, static_cast<std::uint32_t>(a_type));
			std::erase(a_held, id);

			if (a_device) {
				a_device->SetButtonState(id, 0.0f, true, false);
			}
			else {
				queue->AddButtonEvent(a_type, id, 0.0f, 0.0f);
			}
		}
	}

	struct Device_Poll {

		static constexpr std::size_t funcIndex = 0x2;

		template <int ID>
		static void thunk(RE::BSIInputDevice* a_this, float a_timeDelta) {

			constexpr auto type = static_cast<RE::INPUT_DEVICE>(ID);

			if constexpr (type == RE::INPUT_DEVICE::kKeyboard) {
				DropLostKeyboardState(skyrim_cast<RE::BSWin32KeyboardDevice*>(a_this));
			}

			const std::uint32_t firstEvent = RE::BSInputEventQueue::GetSingleton()->buttonEventCount;

			func<ID>(a_this, a_timeDelta);

			RE::BSInputDevice* device = nullptr;
			if constexpr (type == RE::INPUT_DEVICE::kGamepad) {
				device = skyrim_cast<RE::BSPCGamepadDeviceHandler*>(a_this)->GetRuntimeData().currentPCGamePadDelegate;
			}
			else {
				device = skyrim_cast<RE::BSInputDevice*>(a_this);
			}

			static std::vector<std::uint32_t> held;
			SyncHeldButtons(held, type, device, firstEvent);
		}

		template <int ID>
		FUNCTYPE_VFUNC_UNIQUE func;

	};

	inline void Install() {

		logger::info("Installing AltTabFix Hooks");

		Hooks::stl::write_vfunc_unique<RE::BSWin32KeyboardDevice,    Device_Poll, RE::INPUT_DEVICE::kKeyboard>();
		Hooks::stl::write_vfunc_unique<RE::BSWin32MouseDevice,       Device_Poll, RE::INPUT_DEVICE::kMouse>();
		Hooks::stl::write_vfunc_unique<RE::BSPCGamepadDeviceHandler, Device_Poll, RE::INPUT_DEVICE::kGamepad>();

		Hooks::stl::write_vfunc<RE::BSWin32KeyboardDevice, Keyboard_ClearInputState>();

		RemoveAsyncKeyCheck();

	}
}
