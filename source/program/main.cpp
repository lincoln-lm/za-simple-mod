#include "lib.hpp"
#include <memory>

template <typename Ret, typename... Args>
inline static Ret external(long addr, Args... args) {
    return reinterpret_cast<Ret(*)(Args...)>(exl::util::modules::GetTargetOffset(addr))(args...);
}

struct encounter_info_t {
    u8 unk_0[0x48];
    u16 species;
    u8 unk_1[0x6];
    f32 alpha_rate;
    u16 alpha_level_boost;
    u64 can_be_alpha;
    u8 unk_2[0x6];
    u16 form;
    // ...
} PACKED;


HOOK_DEFINE_INLINE(AllAlphaRandomzer) {
    static void Callback(exl::hook::nx64::InlineCtx* ctx) {
        auto encounter_info = reinterpret_cast<encounter_info_t*>(ctx->X[0]);
        encounter_info->alpha_rate = 101.0f;
        do {
            // TODO: this is INCREDIBLY naive
            encounter_info->species = (exl::util::GetRandomU64() % 1025) + 1;
            encounter_info->form = exl::util::GetRandomU64() % 31;
            // PersonalInfo::ExistsInGame
        }
        while (!external<bool>(0x2aa84c, encounter_info->species, encounter_info->form));
    }
};

extern "C" void exl_main(void* x0, void* x1) {
    exl::hook::Initialize();

    AllAlphaRandomzer::InstallAtOffset(0x42eb34);
}

extern "C" NORETURN void exl_exception_entry() {
    EXL_ABORT("Default exception handler called!");
}