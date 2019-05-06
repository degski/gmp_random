
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
    int _mp_alloc     = 0;
    int _mp_size      = 0;
    mp_limb_t * _mp_d = nullptr;

    static_mpz_t ( ) noexcept {}
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

    [[nodiscard]] mpz_ptr get_mpz_t ( ) noexcept { return reinterpret_cast<mpz_ptr> ( this ); }
    [[nodiscard]] mpz_srcptr get_mpz_t ( ) const noexcept { return reinterpret_cast<mpz_srcptr> ( this ); }
};

int main ( ) {

    auto gen = Rng::gen ( );

    static_mpz_storage_t<8> a1{};
    static_mpz_t m1 ( a1 );
    m1.randomize ( gen );
    m1.make_odd ( );
    std::cout << m1.get_mpz_t ( ) << nl;

    static_mpz_storage_t<16> a2{};
    static_mpz_t m2 ( a2 );
    m2.randomize ( gen, 8 );
    m2.make_odd ( );
    std::cout << m2.get_mpz_t ( ) << nl;

    m2 *= m1;
    std::cout << m2.get_mpz_t ( ) << nl;

    m2.resize ( 8 );
    std::cout << m2.get_mpz_t ( ) << nl;

    m2 *= m1;
    std::cout << m2.get_mpz_t ( ) << nl;

    m2.resize ( 8 );
    std::cout << m2.get_mpz_t ( ) << nl;

    return EXIT_SUCCESS;
}
