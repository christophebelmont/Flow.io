#pragma once

namespace Calib {

struct Ph {
    static constexpr float InternalC0 = 0.9583f;
    static constexpr float InternalC1 = 4.834f;
    static constexpr float ExternalC0 = -2.50133333f;
    static constexpr float ExternalC1 = 6.9f;
};

struct Orp {
    static constexpr float InternalC0 = 129.2f;
    static constexpr float InternalC1 = 384.1f;
    static constexpr float ExternalC0 = 431.03f;
    static constexpr float ExternalC1 = 0.0f;
};

struct Psi {
    static constexpr float DefaultC0 = 0.377923399f;
    static constexpr float DefaultC1 = -0.17634473f;
};

struct Temperature {
    static constexpr float Ds18MinValidC = -55.0f;
    static constexpr float Ds18MaxValidC = 125.0f;
};

}  // namespace Calib
