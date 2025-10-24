#include "lib.hpp"
#include <random>

template <typename Ret, typename... Args>
inline static Ret external(long addr, Args... args) {
    return reinterpret_cast<Ret(*)(Args...)>(exl::util::modules::GetTargetOffset(addr))(args...);
}

static u64 session_seed = 0;

struct InitSpec {
    u64 seed;
    u64 is_seeded;
    u64 ec;
    u64 tidsid;
    u64 pid;
    u16 shiny_rolls;
    u16 species;
    u16 form;
    // ...
} PACKED;

HOOK_DEFINE_INLINE(RandomizeStarter) {
    static void Callback(exl::hook::nx64::InlineCtx* ctx) {
        auto init_spec = reinterpret_cast<InitSpec*>(ctx->X[0]);
        if (init_spec->species == 152 || init_spec->species == 158 || init_spec->species == 498) {
            std::mt19937_64 mt(session_seed + init_spec->species);
            std::uniform_int_distribution<u64> dist_species(0, 1025);
            std::uniform_int_distribution<u64> dist_form(0, 31);
            do {
                // TODO: this is INCREDIBLY naive
                init_spec->species = dist_species(mt) + 1;
                init_spec->form = dist_form(mt);
                // PersonalInfo::ExistsInGame
            }
            while (!external<bool>(0x2aa84c, init_spec->species, init_spec->form));
        }
    }
};


extern "C" void exl_main(void* x0, void* x1) {
    exl::hook::Initialize();
    session_seed = exl::util::GetRandomU64();

    // FixInitSpec
    RandomizeStarter::InstallAtOffset(0x69b6c0);
}

extern "C" NORETURN void exl_exception_entry() {
    EXL_ABORT("Default exception handler called!");
}