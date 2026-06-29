#include "global_mouse_hook.h"
#include "cs_logger.h"

namespace cs {

GlobalMouseHook& GlobalMouseHook::instance() {
    static GlobalMouseHook inst;
    return inst;
}

GlobalMouseHook::~GlobalMouseHook() {
    stop();
}

void GlobalMouseHook::start(MouseClickCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (hook_) return;

    callback_ = callback;
    // Note: WH_MOUSE_LL doesn't require a DLL injection.
    // It must be installed on a thread with a message loop (Flutter UI thread qualifies).
    hook_ = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(nullptr), 0);

    if (hook_) {
        CS_LOG_INFO("Global Mouse Hook installed successfully");
    } else {
        CS_LOG_ERR("Failed to install Global Mouse Hook: " + std::to_string(GetLastError()));
    }
}

void GlobalMouseHook::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (hook_) {
        UnhookWindowsHookEx(hook_);
        hook_ = nullptr;
        CS_LOG_INFO("Global Mouse Hook uninstalled");
    }
}

LRESULT CALLBACK GlobalMouseHook::MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN) {
            MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;
            auto& inst = GlobalMouseHook::instance();

            // We use a try_lock or just lock. Since this is a low-level hook,
            // we must be very fast.
            if (inst.mutex_.try_lock()) {
                if (inst.callback_) {
                    inst.callback_((float)mouseStruct->pt.x, (float)mouseStruct->pt.y);
                }
                inst.mutex_.unlock();
            }
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

} // namespace cs
