#include "lib.hpp"
#include <memory>

template <typename Ret, typename... Args>
inline static Ret external(long addr, Args... args) {
    return reinterpret_cast<Ret(*)(Args...)>(exl::util::modules::GetTargetOffset(addr))(args...);
}

u16 rand_move() {
    u16* move_table = reinterpret_cast<u16*>(exl::util::modules::GetTargetOffset(0x3156fd8));
    return move_table[exl::util::GetRandomU64() % 247];
}

HOOK_DEFINE_INLINE(MoveRandomizer) {
    static void Callback(exl::hook::nx64::InlineCtx* ctx) {
        static float last_timers[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        u64 pkm = *reinterpret_cast<u64*>(*reinterpret_cast<u64*>(ctx->X[19] + 0x50) + 0x70);
        float* timers = reinterpret_cast<float*>(ctx->X[19] + 0x5c);
        for (int i = 0; i < 4; i++) {
            // move was just used so its timer is reset from 0
            if (last_timers[i] == 0.0f && timers[i] != 0.0f) {
                // SetMove(pkm, index, move_id)
                external<void>(0xd11e60, pkm, i, rand_move());
            }
            last_timers[i] = timers[i];
        }
        
    }
};

extern "C" void exl_main(void* x0, void* x1) {
    exl::hook::Initialize();

    MoveRandomizer::InstallAtOffset(0xbfe318);
}

extern "C" NORETURN void exl_exception_entry() {
    EXL_ABORT("Default exception handler called!");
}