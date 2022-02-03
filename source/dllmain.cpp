#define DEFAULT_ASPECT_RATIO 16.0f / 9.0f
#define DEFAULT_FOV 65.0f

#include "Utils/MemoryMgr.h"
#include "Utils/Trampoline.h"
#include "Utils/Patterns.h"
#include "Avail/Screen.h"

using namespace std;
using namespace hook;
using namespace Memory::VP;
using namespace avail;

intptr_t(__cdecl* hbUpdateRoutine)(intptr_t);
intptr_t UpdateRoutine(intptr_t a) {
    SetAspectRatio();
    SetFOV(DEFAULT_FOV, DEFAULT_ASPECT_RATIO);

	
    return hbUpdateRoutine(a);
}

void FixAspectRatio() {
    Trampoline* trampoline = Trampoline::MakeTrampoline(GetModuleHandle(nullptr));

    auto updateRoutine = get_pattern("E8 ? ? ? ? 48 8D 05 ? ? ? ? C7 83 ? ? ? ? ? ? ? ? 48 89 03 48 8D 0D ? ? ? ?");
    ReadCall(updateRoutine, hbUpdateRoutine);
    InjectHook(updateRoutine, trampoline->Jump(UpdateRoutine), PATCH_CALL);

    // Fiv FOV
    auto setFieldOfView = get_pattern("F3 0F 10 05 ? ? ? ? F3 0F 11 41 ? 89 41 ? C3", 4);
    WriteOffsetValue(setFieldOfView, &GetFieldOfView());

    // Fix aspect ratio.
    void* setAspectRatio[] = {
        get_pattern("C7 87 A8 03 00 00 3B 8E E3 3F", 6),
        get_pattern("C7 80 A0 02 00 00 3B 8E E3 3F", 6),
        get_pattern("C7 87 A4 03 00 00 3B 8E E3 3F", 6),
        get_pattern("C7 83 A0 02 00 00 3B 8E E3 3F", 6),
    };

    for (;;) {
        for (void* pattern : setAspectRatio)
            Patch<float>(pattern, fAspectRatio);
    }
}

void Init() {
    FixAspectRatio();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Init, 0, 0, 0);
    }
    return TRUE;
}
