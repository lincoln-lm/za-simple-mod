#include "lib.hpp"
#include <random>
#include <unordered_set>

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

const std::unordered_set<u64> blacklisted = {
    150,
    719,
    720,
    720 | (1<<16),
    721,
    3 | (1<<16),
    6 | (1<<16),
    6 | (2<<16),
    9 | (1<<16),
    15 | (1<<16),
    18 | (1<<16),
    36 | (1<<16),
    65 | (1<<16),
    71 | (1<<16),
    80 | (1<<16),
    94 | (1<<16),
    115 | (1<<16),
    121 | (1<<16),
    127 | (1<<16),
    130 | (1<<16),
    142 | (1<<16),
    149 | (1<<16),
    150 | (1<<16),
    150 | (2<<16),
    154 | (1<<16),
    160 | (1<<16),
    181 | (1<<16),
    208 | (1<<16),
    212 | (1<<16),
    214 | (1<<16),
    227 | (1<<16),
    229 | (1<<16),
    248 | (1<<16),
    282 | (1<<16),
    302 | (1<<16),
    303 | (1<<16),
    306 | (1<<16),
    308 | (1<<16),
    310 | (1<<16),
    319 | (1<<16),
    323 | (1<<16),
    334 | (1<<16),
    354 | (1<<16),
    359 | (1<<16),
    362 | (1<<16),
    373 | (1<<16),
    376 | (1<<16),
    428 | (1<<16),
    445 | (1<<16),
    448 | (1<<16),
    460 | (1<<16),
    475 | (1<<16),
    478 | (1<<16),
    500 | (1<<16),
    530 | (1<<16),
    531 | (1<<16),
    545 | (1<<16),
    560 | (1<<16),
    604 | (1<<16),
    609 | (1<<16),
    652 | (1<<16),
    655 | (1<<16),
    658 | (1<<16),
    668 | (1<<16),
    670 | (5<<16),
    670 | (6<<16),
    687 | (1<<16),
    689 | (1<<16),
    691 | (1<<16),
    701 | (1<<16),
    718 | (5<<16),
    719 | (1<<16),
    780 | (1<<16),
    870 | (1<<16),
};

bool is_blacklisted(u64 species, u64 form) {
    return blacklisted.find(species | (form << 16)) != blacklisted.end();
}

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
            while (!external<bool>(0x2aa84c, init_spec->species, init_spec->form) || is_blacklisted(init_spec->species, init_spec->form));
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