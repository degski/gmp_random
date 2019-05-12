
// MIT License
//
// Copyright (c) 2019 degski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <array>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <list>
#include <map>
#include <random>
#include <sax/iostream.hpp>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <plf/plf_nanotimer.h>

#include <sax/prng.hpp>
#include <sax/singleton.hpp>
#include <sax/uniform_int_distribution.hpp>

#if defined( _DEBUG )
#    define RANDOM 0
#else
#    define RANDOM 1
#endif



namespace jsf_detail {

template<typename itype, typename rtype, unsigned int p, unsigned int q, unsigned int r>
class jsf {
    protected:
    itype a_, b_, c_, d_;

    static constexpr unsigned int ITYPE_BITS = 8 * sizeof ( itype );
    static constexpr unsigned int RTYPE_BITS = 8 * sizeof ( rtype );

    static itype rotate ( itype x, unsigned int k ) { return ( x << k ) | ( x >> ( ITYPE_BITS - k ) ); }

    public:
    using result_type = rtype;
    using state_type  = itype;

    static constexpr result_type min ( ) { return 0; }
    static constexpr result_type max ( ) { return ~result_type ( 0 ); }

    jsf ( const itype seed = itype ( 0xcafe5eed00000001ULL ) ) : a_ ( 0xf1ea5eed ), b_ ( seed ), c_ ( seed ), d_ ( seed ) {
        for ( unsigned int i = 0; i < 20; ++i )
            advance ( );
    }

    void seed ( const itype seed = itype ( 0xcafe5eed00000001ULL ) ) {
        a_ = 0xf1ea5eed;
        b_ = seed;
        c_ = seed;
        d_ = seed;
        for ( unsigned int i = 0; i < 20; ++i )
            advance ( );
    }

    void advance ( ) {
        itype e = a_ - rotate ( b_, p );
        a_      = b_ ^ rotate ( c_, q );
        b_      = c_ + ( r ? rotate ( d_, r ) : d_ );
        c_      = d_ + e;
        d_      = e + a_;
    }

    rtype operator( ) ( ) {
        advance ( );
        return rtype ( d_ );
    }

    bool operator== ( const jsf & rhs ) { return ( a_ == rhs.a_ ) && ( b_ == rhs.b_ ) && ( c_ == rhs.c_ ) && ( d_ == rhs.d_ ); }

    bool operator!= ( const jsf & rhs ) { return !operator== ( rhs ); }

    // Not (yet) implemented:
    //   - arbitrary jumpahead (doable, but annoying to write).
    //   - I/O
    //   - Seeding from a seed_seq.
};

} // namespace jsf_detail

///// ---- Specific JSF Generators ---- ////
//
// Each size has variations corresponding to different parameter sets.
// Each variant will create a distinct (and hopefully statistically
// independent) sequence.
//

// - 128 state bits, 32-bit output
//
// The constants are all those suggested by Bob Jenkins.  The n variants
// perform only two rotations, the r variants perform three.

using jsf32na = jsf_detail::jsf<uint32_t, uint32_t, 27, 17, 0>;
using jsf32nb = jsf_detail::jsf<uint32_t, uint32_t, 9, 16, 0>;
using jsf32nc = jsf_detail::jsf<uint32_t, uint32_t, 9, 24, 0>;
using jsf32nd = jsf_detail::jsf<uint32_t, uint32_t, 10, 16, 0>;
using jsf32ne = jsf_detail::jsf<uint32_t, uint32_t, 10, 24, 0>;
using jsf32nf = jsf_detail::jsf<uint32_t, uint32_t, 11, 16, 0>;
using jsf32ng = jsf_detail::jsf<uint32_t, uint32_t, 11, 24, 0>;
using jsf32nh = jsf_detail::jsf<uint32_t, uint32_t, 25, 8, 0>;
using jsf32ni = jsf_detail::jsf<uint32_t, uint32_t, 25, 16, 0>;
using jsf32nj = jsf_detail::jsf<uint32_t, uint32_t, 26, 8, 0>;
using jsf32nk = jsf_detail::jsf<uint32_t, uint32_t, 26, 16, 0>;
using jsf32nl = jsf_detail::jsf<uint32_t, uint32_t, 26, 17, 0>;
using jsf32nm = jsf_detail::jsf<uint32_t, uint32_t, 27, 16, 0>;

using jsf32ra = jsf_detail::jsf<uint32_t, uint32_t, 3, 14, 24>;
using jsf32rb = jsf_detail::jsf<uint32_t, uint32_t, 3, 25, 15>;
using jsf32rc = jsf_detail::jsf<uint32_t, uint32_t, 4, 15, 24>;
using jsf32rd = jsf_detail::jsf<uint32_t, uint32_t, 6, 16, 28>;
using jsf32re = jsf_detail::jsf<uint32_t, uint32_t, 7, 16, 27>;
using jsf32rf = jsf_detail::jsf<uint32_t, uint32_t, 8, 14, 3>;
using jsf32rg = jsf_detail::jsf<uint32_t, uint32_t, 11, 16, 23>;
using jsf32rh = jsf_detail::jsf<uint32_t, uint32_t, 12, 16, 22>;
using jsf32ri = jsf_detail::jsf<uint32_t, uint32_t, 12, 17, 23>;
using jsf32rj = jsf_detail::jsf<uint32_t, uint32_t, 13, 16, 22>;
using jsf32rk = jsf_detail::jsf<uint32_t, uint32_t, 15, 25, 3>;
using jsf32rl = jsf_detail::jsf<uint32_t, uint32_t, 16, 9, 3>;
using jsf32rm = jsf_detail::jsf<uint32_t, uint32_t, 17, 9, 3>;
using jsf32rn = jsf_detail::jsf<uint32_t, uint32_t, 17, 27, 7>;
using jsf32ro = jsf_detail::jsf<uint32_t, uint32_t, 19, 7, 3>;
using jsf32rp = jsf_detail::jsf<uint32_t, uint32_t, 23, 15, 11>;
using jsf32rq = jsf_detail::jsf<uint32_t, uint32_t, 23, 16, 11>;
using jsf32rr = jsf_detail::jsf<uint32_t, uint32_t, 23, 17, 11>;
using jsf32rs = jsf_detail::jsf<uint32_t, uint32_t, 24, 3, 16>;
using jsf32rt = jsf_detail::jsf<uint32_t, uint32_t, 24, 4, 16>;
using jsf32ru = jsf_detail::jsf<uint32_t, uint32_t, 25, 14, 3>;
using jsf32rv = jsf_detail::jsf<uint32_t, uint32_t, 27, 16, 6>;
using jsf32rw = jsf_detail::jsf<uint32_t, uint32_t, 27, 16, 7>;

using jsf32n = jsf32na;
using jsf32r = jsf32rq;
using jsf32  = jsf32n;

// - 256 state bits, uint64_t output

using jsf64na = jsf_detail::jsf<uint64_t, uint64_t, 39, 11, 0>;
using jsf64ra = jsf_detail::jsf<uint64_t, uint64_t, 7, 13, 37>;

using jsf64n = jsf64na;
using jsf64r = jsf64ra;
using jsf64  = jsf64r;

// TINY VERSIONS FOR TESTING AND SPECIALIZED USES ONLY
//
// Parameters derived using a variant of rngav.c, originally written by
// Bob Jenkins.

// - 64 state bits, uint16_t output

using jsf16na = jsf_detail::jsf<uint16_t, uint16_t, 13, 8, 0>;

using jsf16 = jsf16na;

// - 32 state bits, uint8_t output

using jsf8na = jsf_detail::jsf<uint8_t, uint8_t, 1, 4, 0>;

using jsf8 = jsf8na;

struct Rng final {

    Rng ( Rng && )      = delete;
    Rng ( const Rng & ) = delete;

    Rng & operator= ( Rng && ) = delete;
    Rng & operator= ( const Rng & ) = delete;

    static void seed ( const std::uint64_t s_ = 0u ) noexcept { Rng::gen ( ).seed ( s_ ? s_ : sax::os_seed ( ) ); }

    [[nodiscard]] static sax::Rng & gen ( ) noexcept {
        static thread_local sax::Rng generator ( RANDOM ? sax::os_seed ( ) : sax::fixed_seed ( ) );
        return generator;
    }
};

#undef RANDOM

#include <gmpxx.h>

template<std::size_t S>
using static_mpz_storage_t = std::array<mp_limb_t, S>;

struct static_mpz_t {
    int _mp_alloc, _mp_size;
    mp_limb_t * _mp_d;

    static_mpz_t ( int a_ = 0, int s_ = 0, mp_limb_t * m_ = nullptr ) noexcept :
        _mp_alloc ( std::move ( a_ ) ), _mp_size ( std::move ( s_ ) ), _mp_d ( std::move ( m_ ) ) {}
    template<std::size_t S>
    static_mpz_t ( static_mpz_storage_t<S> & array_ ) noexcept :
        _mp_alloc ( S ), _mp_d ( array_.data ( ) ){}

                                 [ [maybe_unused] ] static_mpz_t
                             & operator+= ( const static_mpz_t & rhs_ ) noexcept {
        assert ( _mp_d );
        assert ( rhs_._mp_d );
        mpz_add ( reinterpret_cast<mpz_ptr> ( this ), reinterpret_cast<mpz_srcptr> ( this ),
                  reinterpret_cast<mpz_srcptr> ( &rhs_ ) );
        return *this;
    }
    [[maybe_unused]] static_mpz_t & operator-= ( const static_mpz_t & rhs_ ) noexcept {
        assert ( _mp_d );
        assert ( rhs_._mp_d );
        mpz_sub ( reinterpret_cast<mpz_ptr> ( this ), reinterpret_cast<mpz_srcptr> ( this ),
                  reinterpret_cast<mpz_srcptr> ( &rhs_ ) );
        return *this;
    }
    [[maybe_unused]] static_mpz_t & operator*= ( const static_mpz_t & rhs_ ) noexcept {
        assert ( _mp_d );
        assert ( rhs_._mp_d );
        mpz_mul ( reinterpret_cast<mpz_ptr> ( this ), reinterpret_cast<mpz_srcptr> ( this ),
                  reinterpret_cast<mpz_srcptr> ( &rhs_ ) );
        return *this;
    }
    [[maybe_unused]] static_mpz_t & operator/= ( const static_mpz_t & rhs_ ) noexcept {
        assert ( _mp_d );
        assert ( rhs_._mp_d );
        assert ( rhs_._mp_d[ 0 ] );
        mpz_div ( reinterpret_cast<mpz_ptr> ( this ), reinterpret_cast<mpz_srcptr> ( this ),
                  reinterpret_cast<mpz_srcptr> ( &rhs_ ) );
        return *this;
    }

    [[nodiscard]] constexpr int capacity ( ) noexcept { return _mp_alloc; }
    [[nodiscard]] int size ( ) noexcept { return _mp_size; }

    template<typename Generator>
    void randomize ( Generator & gen_, const int size_ = 0 ) noexcept {
        assert ( _mp_d );
        assert ( size_ <= _mp_alloc );
        _mp_size = size_ ? size_ : _mp_alloc;
        std::generate ( std::execution::par_unseq, _mp_d, _mp_d + _mp_size,
                        [&]( ) { return sax::uniform_int_distribution<mp_limb_t> ( ) ( gen_ ); } );
    }

    void make_odd ( ) noexcept {
        assert ( _mp_size );
        *reinterpret_cast<std::uint8_t *> ( _mp_d ) |= 0b0000'0001;
    }
    void make_even ( ) noexcept {
        assert ( _mp_size );
        *reinterpret_cast<std::uint8_t *> ( _mp_d ) &= 0b1111'1110;
    }

    void resize ( const int size_ ) noexcept {
        assert ( _mp_d );
        assert ( size_ <= _mp_alloc );
        _mp_size = size_;
    }

    void shift_high ( ) noexcept {
        assert ( _mp_alloc == _mp_size );
        assert ( _mp_alloc % 2 == 0 );
        _mp_size /= 2;
        std::memcpy ( _mp_d, _mp_d + _mp_size, _mp_size * sizeof ( mp_limb_t ) );
    }

    [[nodiscard]] mpz_ptr get_mpz_t ( ) noexcept { return reinterpret_cast<mpz_ptr> ( this ); }
    [[nodiscard]] mpz_srcptr get_mpz_t ( ) const noexcept { return reinterpret_cast<mpz_srcptr> ( this ); }

    [[nodiscard]] static_mpz_t low_view ( ) noexcept { return { -1, _mp_size / 2, _mp_d }; }
    [[nodiscard]] static_mpz_t high_view ( ) noexcept { return { -1, _mp_size / 2, _mp_d + _mp_size / 2 }; }
};

void mul ( static_mpz_t & d_, static_mpz_t & s1_, static_mpz_t & s2_ ) noexcept {
    assert ( s1_._mp_size == s2_._mp_size );
    assert ( d_._mp_alloc == 2 * s1_._mp_size );
    d_._mp_size = d_._mp_alloc;
    mpn_mul_n ( d_._mp_d, s1_._mp_d, s2_._mp_d, s1_._mp_size );
}

/*

namespace lehmer_detail {

template<typename rtype, typename stype>
class mcg128 {
    stype state_;
    static constexpr __uint128_t MCG_MULT = ( __uint128_t{ 5017888479014934897ULL } << 64 ) +
                                            2747143273072462557ULL; // passing as a template parameter crashes clang frontend.

    static constexpr unsigned int STYPE_BITS = 8 * sizeof ( stype );
    static constexpr unsigned int RTYPE_BITS = 8 * sizeof ( rtype );

    public:
    using result_type = rtype;
    static constexpr result_type min ( ) { return result_type ( 0 ); }
    static constexpr result_type max ( ) { return ~result_type ( 0 ); }

    mcg128 ( stype state = stype ( 0x9f57c403d06c42fcUL ) ) : state_ ( state | 1 ) {
        // Nothing (else) to do.
    }

    void seed ( const rtype s_ ) { state_ = stype{ s_ | 1 }; }

    void advance ( ) { state_ *= MCG_MULT; }

    result_type operator( ) ( ) {
        advance ( );
        return result_type ( state_ >> ( STYPE_BITS - RTYPE_BITS ) );
    }

    bool operator== ( const mcg128 & rhs ) { return ( state_ == rhs.state_ ); }

    bool operator!= ( const mcg128 & rhs ) { return !operator== ( rhs ); }

    // Not (yet) implemented:
    //   - arbitrary jumpahead (see PCG code for an implementation)
    //   - I/O
    //   - Seeding from a seed_seq.
};

template<typename rtype, typename stype>
class mcg128_fast {
    stype state_;
    static constexpr uint64_t MCG_MULT = 0xda942042e4dd58b5ULL;

    static constexpr unsigned int STYPE_BITS = 8 * sizeof ( stype );
    static constexpr unsigned int RTYPE_BITS = 8 * sizeof ( rtype );

    public:
    using result_type = rtype;
    static constexpr result_type min ( ) { return result_type ( 0 ); }
    static constexpr result_type max ( ) { return ~result_type ( 0 ); }

    mcg128_fast ( stype state = stype ( 0x9f57c403d06c42fcUL ) ) : state_ ( state | 1 ) {
        // Nothing (else) to do.
    }

    void seed ( const rtype s_ ) { state_ = stype{ s_ | 1 }; }

    void advance ( ) { state_ *= MCG_MULT; }

    result_type operator( ) ( ) {
        advance ( );
        return result_type ( state_ >> ( STYPE_BITS - RTYPE_BITS ) );
    }

    bool operator== ( const mcg128_fast & rhs ) { return ( state_ == rhs.state_ ); }

    bool operator!= ( const mcg128_fast & rhs ) { return !operator== ( rhs ); }

    // Not (yet) implemented:
    //   - arbitrary jumpahead (see PCG code for an implementation)
    //   - I/O
    //   - Seeding from a seed_seq.
};
} // namespace lehmer_detail

using mcg128      = lehmer_detail::mcg128<uint64_t, __uint128_t>;
using mcg128_fast = lehmer_detail::mcg128_fast<uint64_t, __uint128_t>;

*/

template<std::size_t S>
struct GMPRng {

    static_assert ( S % 2 == 0, "size has to be even" );

    static_mpz_storage_t<2 * S> _state_storage_0, _state_storage_1;
    static_mpz_storage_t<S> _multiplier_storage;
    static_mpz_t _state;
    mp_limb_t * _destination;

    GMPRng ( ) noexcept : _state ( _state_storage_0 ), _destination ( _state_storage_1.data ( ) ) {
        _state.randomize ( Rng::gen ( ), S );
        _state.make_odd ( );
        static_mpz_t multiplier ( _multiplier_storage );
        multiplier.randomize ( Rng::gen ( ) );
        multiplier.make_odd ( );
    }

    static_mpz_t & operator( ) ( ) noexcept {
        mpn_mul_n ( _destination, _state._mp_d, _multiplier_storage.data ( ), S );
        std::swap ( _destination, _state._mp_d );
        std::copy_n ( _state._mp_d + ( S - 1 ), S, _state._mp_d );
        return _state;
    }
};

template<std::size_t S>
struct GMPRng2 {

    static_assert ( S % 2 == 0, "size has to be even" );

    using result_type = std::uint64_t;
    [[nodiscard]] static constexpr result_type min ( ) noexcept { return result_type ( 0 ); }
    [[nodiscard]] static constexpr result_type max ( ) noexcept { return ~result_type ( 0 ); }

    static_mpz_storage_t<2 * S> _state_storage_0, _state_storage_1;
    static_mpz_storage_t<S> _multiplier_storage;
    static_mpz_t _state;
    mp_limb_t * _destination;
    int _limb = 0;

    GMPRng2 ( ) noexcept : _state ( _state_storage_0 ), _destination ( _state_storage_1.data ( ) + ( S - 1 ) ) {
        _state._mp_d += ( S - 1 );
        _state.randomize ( Rng::gen ( ), S );
        _state.make_odd ( );
        static_mpz_t multiplier ( _multiplier_storage );
        multiplier.randomize ( Rng::gen ( ) );
        multiplier.make_odd ( );
    }

    inline void advance ( ) noexcept {
        constexpr int used = 2;
        mpn_mul ( _destination - ( S - 1 ) + ( S - used ), _state._mp_d, S, _multiplier_storage.data ( ), used );
        std::swap ( _destination, _state._mp_d );
        _limb = 1;
    }

    [[nodiscard]] result_type operator( ) ( ) noexcept {
        if ( _limb != S )
            return _state._mp_d[ _limb++ ];
        advance ( );
        return _state._mp_d[ 0 ];
    }

    [[nodiscard]] bool operator== ( const GMPRng2 & rhs_ ) noexcept { return ( _state == rhs_._state ); }
    [[nodiscard]] bool operator!= ( const GMPRng2 & rhs_ ) noexcept { return not operator== ( rhs_ ); }
};




using Generator = jsf64;
 // GMPRng2<64>;

#if 1

int main ( ) {

    Generator prng;

    std::uint64_t x = 0, y = 0;

    plf::nanotimer timer;
    timer.start ( );

    for ( int i = 0; i < 100'000'000; ++i ) {
        x += prng ( );
        ++y;
    }

    std::uint64_t t = ( std::uint64_t ) timer.get_elapsed_ms ( );

    std::cout << x << ' ' << y << nl;
    std::cout << ( ( ( double ) t ) / 1000 ) << nl;

    return EXIT_SUCCESS;
}

#else

#    ifdef _WIN32 // needed to allow binary stdout on windows
#        include <fcntl.h>
#        include <io.h>
#    endif

#    include <cstdint>
#    include <iostream>

int main ( ) {

#    ifdef _WIN32 // Needed to allow binary stdout on Windhoze...
    _setmode ( _fileno ( stdout ), _O_BINARY );
#    endif

    using result_type = typename Generator::result_type;

    Generator rng;

    constexpr std::size_t buffer_size = 4'096 / sizeof ( result_type );
    std::vector<result_type> buffer ( buffer_size );

    while ( true ) {

        for ( auto & v : buffer )
            v = rng ( );

        std::cout.write ( reinterpret_cast<char *> ( buffer.data ( ) ), buffer_size * sizeof ( result_type ) );
    }

    return EXIT_SUCCESS;
}

#endif
