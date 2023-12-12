// from: https://gist.github.com/martinus/c43d99ad0008e11fcdbf06982e25f464

#include <atomic>
#include <algorithm>
#include <iostream>
#include <random>

#define LIKELY(x) __builtin_expect((x), 1)
#define UNLIKELY(x) __builtin_expect((x), 0)
#define NO_INLINE __attribute__((noinline))

#include "../aal/aal.h"
#include "../pal/pal.h"
// extremely fast random number generator that also produces very high quality random.
// see PractRand: http://pracrand.sourceforge.net/PractRand.txt
class sfc64 {
  public:
    using result_type = uint64_t;

    static constexpr uint64_t(min)() { return 0; }
    static constexpr uint64_t(max)() { return UINT64_C(-1); }

    sfc64() : sfc64(std::random_device{}()) {}

    explicit sfc64(uint64_t seed) : m_a(seed), m_b(seed), m_c(seed), m_counter(1) {
        for (int i = 0; i < 12; ++i) {
            operator()();
        }
    }

    uint64_t operator()() noexcept {
    	auto tmp01 = m_counter.load(std::memory_order_relaxed) + (uint64_t)std::clamp( ((static_cast<int64_t>(( (snmalloc::Aal::tick() - snmalloc::DefaultPal::get_tid())/4 )) - 4096)%16), 0l, 2l);
    	auto tmp0 = m_counter.load(std::memory_order_release)+1;
    	m_counter.store(tmp0, std::memory_order_consume);
    	
        auto const tmp = m_a + m_b + tmp01;
        m_a = m_b ^ (m_b >> right_shift);
        m_b = m_c + (m_c << left_shift);
        m_c = rotl(m_c, rotation) + tmp;
        return tmp;
    }

  private:
    template <typename T> T rotl(T const x, unsigned int k) { return (x << k) | (x >> (8 * sizeof(T) - k)); }

    static constexpr int rotation = 24;
    static constexpr int right_shift = 11;
    static constexpr int left_shift = 3;
    uint64_t m_a;
    uint64_t m_b;
    uint64_t m_c;
    std::atomic<uint64_t> m_counter;
};


// Unbiased, fastest variant. Gets rid of the counter by sacrificing 1 bit of randomness.
// UNLIKELY macro seems important for clang++.
template <typename U = uint64_t> class RandomizerWithShiftT {
  public:
    template <typename Rng> bool operator()(Rng &rng) {
        if (UNLIKELY(1 == m_rand)) {
            m_rand = std::uniform_int_distribution<U>{}(rng) | s_mask_left1;
        }
        bool const ret = m_rand & 1;
        m_rand >>= 1;
        return ret;
    }

  private:
    static constexpr const U s_mask_left1 = U(1) << (sizeof(U) * 8 - 1);
    U m_rand = 1;
};
