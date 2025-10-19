#include "lib.hpp"
#include <memory>

template <typename Ret, typename... Args>
inline static Ret external(long addr, Args... args) {
    return reinterpret_cast<Ret(*)(Args...)>(exl::util::modules::GetTargetOffset(addr))(args...);
}

static u8 last_selected_mon = 0;
static u8 last_experience_index = 0;

HOOK_DEFINE_INLINE(UpdateLastSentOutMon) {
    static void Callback(exl::hook::nx64::InlineCtx* ctx) {
        last_selected_mon = ctx->W[8];
    }
};

HOOK_DEFINE_INLINE(ResetExperienceIndex) {
    static void Callback(exl::hook::nx64::InlineCtx* ctx) {
        last_experience_index = 0;
    }
};

HOOK_DEFINE_INLINE(GatekeepExperience) {
    static void Callback(exl::hook::nx64::InlineCtx* ctx) {
        // only give exp to the pokemon that is currently out (presumably the one that was used in battle)
        if (last_experience_index++ != last_selected_mon) {
            ctx->W[8] = 0;
        }
    }
};

extern "C" void exl_main(void* x0, void* x1) {
    exl::hook::Initialize();

    // start of function that gives exp to party
    ResetExperienceIndex::InstallAtOffset(0x2420dc);
    // part of the above function that reads the amount of exp to give
    GatekeepExperience::InstallAtOffset(0x242164);
    // called constantly in the overworld and checks the sent out mon's index
    UpdateLastSentOutMon::InstallAtOffset(0x2e7380);
}

extern "C" NORETURN void exl_exception_entry() {
    EXL_ABORT("Default exception handler called!");
}