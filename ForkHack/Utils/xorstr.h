#ifndef OBF_XORSTR_HPP
#define OBF_XORSTR_HPP

#if defined(_M_ARM64) || defined(__aarch64__) || defined(_M_ARM) || defined(__arm__)
#   include <arm_neon.h>
#elif defined(_M_X64) || defined(__amd64__) || defined(_M_IX86) || defined(__i386__)
#   include <immintrin.h>
#else
#   error Unsupported platform
#endif

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <utility>
#include <type_traits>

#ifdef _MSC_VER
#   define XS_FORCEINLINE __forceinline
#   define XS_NOINLINE    __declspec(noinline)
#else
#   define XS_FORCEINLINE __attribute__((always_inline)) inline
#   define XS_NOINLINE    __attribute__((noinline))
#endif



#define xorstr(str) \
    ::xs::make_xor_string([]() { return str; }, \
        std::integral_constant<std::size_t, sizeof(str)/sizeof(*str)>{}, \
        std::make_index_sequence<::xs::detail::buf_chunks<sizeof(str)>()>{}, \
        std::integral_constant<std::uint32_t, __LINE__>{})

#define xorstr_(str)   xorstr(str).crypt_get()

namespace xs {

    namespace detail {


        XS_FORCEINLINE constexpr std::uint32_t mm3_fmix(std::uint32_t h) noexcept {
            h ^= h >> 16;
            h *= 0x85EBCA6Bu;
            h ^= h >> 13;
            h *= 0xC2B2AE35u;
            h ^= h >> 16;
            return h;
        }

        XS_FORCEINLINE constexpr std::uint64_t mm3_fmix64(std::uint64_t k) noexcept {
            k ^= k >> 33;
            k *= 0xFF51AFD7ED558CCDull;
            k ^= k >> 33;
            k *= 0xC4CEB9FE1A85EC53ull;
            k ^= k >> 33;
            return k;
        }


        XS_FORCEINLINE constexpr std::uint32_t ct_hash_str(const char* s) noexcept {
            std::uint32_t h = 0x811C9DC5u;
            while (*s) {
                h ^= static_cast<uint8_t>(*s++);
                h *= 0x1000193u;
            }
            return h;
        }


        template<std::size_t ChunkIdx, std::uint32_t Counter>
        XS_FORCEINLINE constexpr std::uint64_t make_key_xor() noexcept {
            constexpr std::uint32_t base = ct_hash_str(__TIME__ __DATE__);
            constexpr std::uint32_t lo = mm3_fmix(base ^ (static_cast<std::uint32_t>(ChunkIdx) * 0xCC9E2D51u) ^ Counter);
            constexpr std::uint32_t hi = mm3_fmix(lo ^ (static_cast<std::uint32_t>(ChunkIdx) * 0x1B873593u) ^ ~Counter);
            return (static_cast<std::uint64_t>(hi) << 32) | lo;
        }

        template<std::size_t ChunkIdx, std::uint32_t Counter>
        XS_FORCEINLINE constexpr std::uint64_t make_key_add() noexcept {
            constexpr std::uint32_t base = ct_hash_str(__DATE__ __TIME__);
            constexpr std::uint32_t lo = mm3_fmix(base ^ (static_cast<std::uint32_t>(ChunkIdx + 1) * 0x9E3779B9u) ^ Counter);
            constexpr std::uint32_t hi = mm3_fmix(lo ^ (static_cast<std::uint32_t>(ChunkIdx + 3) * 0x6C62272Eu) ^ Counter);
            return (static_cast<std::uint64_t>(hi) << 32) | lo;
        }

        template<std::size_t ChunkIdx, std::uint32_t Counter>
        XS_FORCEINLINE constexpr std::uint8_t make_key_rol() noexcept {
            constexpr std::uint32_t v = mm3_fmix(
                ct_hash_str(__TIME__) ^ (static_cast<std::uint32_t>(ChunkIdx) * 0xDEADC0DEu) ^ Counter);
            return static_cast<std::uint8_t>((v % 63) + 1);
        }


        XS_FORCEINLINE constexpr std::uint64_t rol64(std::uint64_t v, std::uint8_t n) noexcept {
            return (v << n) | (v >> (64u - n));
        }
        XS_FORCEINLINE constexpr std::uint64_t ror64(std::uint64_t v, std::uint8_t n) noexcept {
            return (v >> n) | (v << (64u - n));
        }


        XS_FORCEINLINE constexpr std::uint64_t encrypt_chunk(
            std::uint64_t plain,
            std::uint64_t key_xor,
            std::uint64_t key_add,
            std::uint8_t  key_rol) noexcept
        {
            return rol64((plain + key_add) ^ key_xor, key_rol);
        }

        XS_FORCEINLINE std::uint64_t decrypt_chunk(
            std::uint64_t cipher,
            std::uint64_t key_xor,
            std::uint64_t key_add,
            std::uint8_t  key_rol) noexcept
        {
            return (ror64(cipher, key_rol) ^ key_xor) - key_add;
        }


        template<std::size_t ByteSize>
        XS_FORCEINLINE constexpr std::size_t buf_chunks() noexcept {
            return (ByteSize + 7u) / 8u;
        }


        template<std::size_t N, class CharT>
        XS_FORCEINLINE constexpr std::uint64_t load_str8(std::size_t chunk, const CharT* str) noexcept {
            using U = typename std::make_unsigned<CharT>::type;
            constexpr std::size_t chars_per_chunk = 8u / sizeof(CharT);
            std::uint64_t val = 0;
            for (std::size_t i = 0; i < chars_per_chunk; ++i) {
                std::size_t pos = chunk * chars_per_chunk + i;
                if (pos < N)
                    val |= (std::uint64_t{ static_cast<U>(str[pos]) } << (i * 8u * sizeof(CharT)));
            }
            return val;
        }


        XS_FORCEINLINE std::uint64_t reg(std::uint64_t v) noexcept {
#if defined(__clang__) || defined(__GNUC__)
            asm("" : "=r"(v) : "0"(v) : );
            return v;
#else
            volatile std::uint64_t r = v;
            return r;
#endif
        }

    } // namespace detail


    template<
        class CharT,
        std::size_t Size,
        std::uint32_t Counter,
        std::size_t... Chunks>
    class xor_string {
        static constexpr std::size_t N = sizeof...(Chunks);

        std::uint64_t _storage[N];

        static constexpr std::uint64_t _kxor[N] = {
            detail::make_key_xor<Chunks, Counter>()... };
        static constexpr std::uint64_t _kadd[N] = {
            detail::make_key_add<Chunks, Counter>()... };
        static constexpr std::uint8_t  _krol[N] = {
            detail::make_key_rol<Chunks, Counter>()... };

    public:
        using value_type = CharT;
        using size_type = std::size_t;
        using pointer = CharT*;
        using const_pointer = const CharT*;

        template<class L>
        XS_FORCEINLINE xor_string(L l, std::index_sequence<Chunks...>) noexcept
            : _storage{
                detail::reg(
                    std::integral_constant<std::uint64_t,
                        detail::encrypt_chunk(
                            detail::load_str8<Size>(Chunks, l()),
                            detail::make_key_xor<Chunks, Counter>(),
                            detail::make_key_add<Chunks, Counter>(),
                            detail::make_key_rol<Chunks, Counter>()
                        )>::value
                )...
            }
        {
        }

        XS_FORCEINLINE constexpr size_type size() const noexcept { return Size - 1; }

        XS_FORCEINLINE void crypt() noexcept {
            for (std::size_t i = 0; i < N; ++i)
                _storage[i] = detail::decrypt_chunk(_storage[i], _kxor[i], _kadd[i], _krol[i]);
        }

        XS_FORCEINLINE void encrypt() noexcept {
            for (std::size_t i = 0; i < N; ++i)
                _storage[i] = detail::encrypt_chunk(_storage[i], _kxor[i], _kadd[i], _krol[i]);
        }

        XS_FORCEINLINE pointer get() noexcept {
            crypt();
            return reinterpret_cast<pointer>(_storage);
        }

        XS_FORCEINLINE pointer crypt_get() noexcept {
            return get();
        }

        XS_NOINLINE pointer crypt_get_wipe() noexcept {
            crypt();
            volatile std::uint64_t noise =
                reinterpret_cast<std::uint64_t>(this) ^ 0xDEADBEEFCAFEBABEull;
            for (std::size_t i = 0; i < N; ++i) {
                volatile std::uint64_t tmp = _storage[i]; (void)tmp;
                _storage[i] = noise ^ (noise >> (i & 7));
                noise = detail::mm3_fmix64(noise ^ _storage[i]);
            }
            return reinterpret_cast<pointer>(
                const_cast<std::uint64_t*>(
                    reinterpret_cast<volatile std::uint64_t*>(_storage)));
        }
    };


    template<class L, std::size_t Size, std::uint32_t Counter, std::size_t... Chunks>
    XS_FORCEINLINE auto make_xor_string(
        L l,
        std::integral_constant<std::size_t, Size>,
        std::index_sequence<Chunks...>,
        std::integral_constant<std::uint32_t, Counter>) noexcept
    {
        using CharT = std::remove_const_t<
            std::remove_reference_t<decltype(l()[0])>>;
        return xor_string<CharT, Size, Counter, Chunks...>(l, std::index_sequence<Chunks...>{});
    }

} // namespace xs

#endif 