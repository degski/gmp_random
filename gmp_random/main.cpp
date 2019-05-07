
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
#include <vector>

#include <sax/prng.hpp>
#include <sax/singleton.hpp>
#include <sax/uniform_int_distribution.hpp>

#if defined( _DEBUG )
#    define RANDOM 0
#else
#    define RANDOM 1
#endif

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
    static_mpz_t ( static_mpz_storage_t<S> & array_ ) noexcept : _mp_alloc ( S ), _mp_d ( array_.data ( ) ) {}

    [[maybe_unused]] static_mpz_t & operator+= ( const static_mpz_t & rhs_ ) noexcept {
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
                        [&] ( ) { return sax::uniform_int_distribution<mp_limb_t> ( ) ( gen_ ); } );
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

template<typename rtype, typename stype, auto multiplier>
class mcg {
    stype state_;
    static constexpr auto MCG_MULT = multiplier;

    static constexpr unsigned int STYPE_BITS = 8 * sizeof ( stype );
    static constexpr unsigned int RTYPE_BITS = 8 * sizeof ( rtype );

    public:
    using result_type = rtype;
    static constexpr result_type min ( ) { return result_type ( 0 ); }
    static constexpr result_type max ( ) { return ~result_type ( 0 ); }

    mcg ( stype state = stype ( 0x9f57c403d06c42fcUL ) ) : state_ ( state | 1 ) {
        // Nothing (else) to do.
    }

    void advance ( ) { state_ *= MCG_MULT; }

    result_type operator( ) ( ) {
        advance ( );
        return result_type ( state_ >> ( STYPE_BITS - RTYPE_BITS ) );
    }

    bool operator== ( const mcg & rhs ) { return ( state_ == rhs.state_ ); }

    bool operator!= ( const mcg & rhs ) { return !operator== ( rhs ); }

    // Not (yet) implemented:
    //   - arbitrary jumpahead (see PCG code for an implementation)
    //   - I/O
    //   - Seeding from a seed_seq.
};

} // namespace lehmer_detail

using mcg128 = lehmer_detail::mcg<uint64_t, __uint128_t, ( __uint128_t ( 5017888479014934897ULL ) << 64 ) + 2747143273072462557ULL>;

using mcg128_fast = lehmer_detail::mcg<uint64_t, __uint128_t, 0xda942042e4dd58b5ULL>;

*/

struct GMPRng {

    static_mpz_storage_t<16> _state_storage_0, _state_storage_1;
    static_mpz_storage_t<8> _multiplier_storage;
    static_mpz_t _state, _multiplier;
    mp_limb_t * _storage[ 2 ];

    GMPRng ( ) noexcept :
        _state ( _state_storage_0 ),
        _multiplier ( _multiplier_storage ), _storage{ _state_storage_0.data ( ), _state_storage_1.data ( ) } {
        _state.randomize ( Rng::gen ( ), 8 );
        _state.make_odd ( );
        _multiplier.randomize ( Rng::gen ( ) );
        _multiplier.make_odd ( );
    }

    void advance ( ) {
        mpn_mul_n ( _storage[ 1 ], _storage[ 0 ], _multiplier_storage.data ( ), 8 );
        std::swap ( _storage[ 0 ], _storage[ 1 ] );
    }

    static_mpz_t & operator( ) ( ) {
        advance ( );
        std::copy_n ( _storage[ 0 ] + 7, 8, _storage[ 0 ] );
        _state._mp_d = _storage[ 0 ];
        return _state;
    }
};


int main ( ) {

    GMPRng prng;

    for ( int i = 0; i < 1000; ++i )
        std::cout << prng ( ).get_mpz_t ( ) << nl;

    return EXIT_SUCCESS;
}
