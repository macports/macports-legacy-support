#if defined(__cplusplus)

#if __cplusplus == 201707L
#warning "Using STD c++2a (will be c++20)"
#elif __cplusplus == 201703L
#warning "Using STD c++17 or c++1z (will be c++17)"
#elif __cplusplus == 201402L
#warning "Using STD c++14"
#elif __cplusplus == 201103L
#warning "Using STD c++11"
#elif __cplusplus == 199711L
#warning "Using STD c++98 or c++03"
#else
#warning "Unknown C++ standard in use ... this should never happen!"
#endif

#if __cplusplus >= 201103L

#if defined(__clang__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1090)
#warning "Building using some Clang compiler on OSX 10.8 or earlier; this will probably fail due to there being no relevant c++11 headers."
#else
#warning "Using c++11 with a compiler that can correctly support it; this build should succeed correctly."
#endif

#else /* __cplusplus < 201103L, so something older than c++11 */

#if defined(__clang__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1080)
#warning "Building using some Clang compiler using OSX 10.9 or newer; this build will probably succeed incorrectly."
#else
#warning "This build should fail correctly for all compilers."
#endif

#endif /* __cplusplus >= 201103L */

#else /* !defined(__cplusplus) */

#warning "No C++ standard set ... this should never happen!"

#endif  /* defined(__cplusplus) */

#include <cmath>
#include <iostream>

#define _STR(A) #A
#define STR(A) _STR(A)
#define DOIT(AAA) std::cout << STR(AAA) << " : " << AAA << std::endl

int main() {
  DOIT(std::acosh(0.0));
  DOIT(std::acoshf(1.1));
  DOIT(std::acoshl(1.1));
  DOIT(std::asinh(1.1));
  DOIT(std::asinhf(1.1));
  DOIT(std::asinhl(1.1));
  DOIT(std::atanh(1.1));
  DOIT(std::atanhf(1.1));
  DOIT(std::atanhl(1.1));
  DOIT(std::cbrt(1.1));
  DOIT(std::cbrtf(1.1));
  DOIT(std::cbrtl(1.1));
  DOIT(std::copysign(1.1, 1.1));
  DOIT(std::copysignf(1.1, 1.1));
  DOIT(std::copysignl(1.1, 1.1));
  DOIT(std::erf(1.1));
  DOIT(std::erff(1.1));
  DOIT(std::erfl(1.1));
  DOIT(std::erfc(1.1));
  DOIT(std::erfcf(1.1));
  DOIT(std::erfcl(1.1));
  DOIT(std::exp2(1.1));
  DOIT(std::exp2f(1.1));
  DOIT(std::exp2l(1.1));
  DOIT(std::expm1(1.1));
  DOIT(std::expm1f(1.1));
  DOIT(std::expm1l(1.1));
  DOIT(std::fdim(1.1, 1.1));
  DOIT(std::fdimf(1.1, 1.1));
  DOIT(std::fdiml(1.1, 1.1));
  DOIT(std::fma(1.1, 1.1, 1.1));
  DOIT(std::fmaf(1.1, 1.1, 1.1));
  DOIT(std::fmal(1.1, 1.1, 1.1));
  DOIT(std::fmax(1.1, 1.1));
  DOIT(std::fmaxf(1.1, 1.1));
  DOIT(std::fmaxl(1.1, 1.1));
  DOIT(std::fmin(1.1, 1.1));
  DOIT(std::fminf(1.1, 1.1));
  DOIT(std::fminl(1.1, 1.1));
  DOIT(std::hypot(1.1, 1.1));
  DOIT(std::hypotf(1.1, 1.1));
  DOIT(std::hypotl(1.1, 1.1));
  DOIT(std::ilogb(1.1));
  DOIT(std::ilogbf(1.1));
  DOIT(std::ilogbl(1.1));
  DOIT(std::lgamma(1.1));
  DOIT(std::lgammaf(1.1));
  DOIT(std::lgammal(1.1));
  DOIT(std::llrint(1.1));
  DOIT(std::llrintf(1.1));
  DOIT(std::llrintl(1.1));
  DOIT(std::llround(1.1));
  DOIT(std::llroundf(1.1));
  DOIT(std::llroundl(1.1));
  DOIT(std::log1p(1.1));
  DOIT(std::log1pf(1.1));
  DOIT(std::log1pl(1.1));
  DOIT(std::log2(1.1));
  DOIT(std::log2f(1.1));
  DOIT(std::log2l(1.1));
  DOIT(std::logb(1.1));
  DOIT(std::logbf(1.1));
  DOIT(std::logbl(1.1));
  DOIT(std::lrint(1.1));
  DOIT(std::lrintf(1.1));
  DOIT(std::lrintl(1.1));
  DOIT(std::lround(1.1));
  DOIT(std::lroundf(1.1));
  DOIT(std::lroundl(1.1));
  DOIT(std::nan("1"));
  DOIT(std::nanf("1"));
  DOIT(std::nanl("1"));
  DOIT(std::nearbyint(1.1));
  DOIT(std::nearbyintf(1.1));
  DOIT(std::nearbyintl(1.1));
  DOIT(std::nextafter(1.1, 1.1));
  DOIT(std::nextafterf(1.1, 1.1));
  DOIT(std::nextafterl(1.1, 1.1));
  DOIT(std::nexttoward(1.1, 1.1));
  DOIT(std::nexttowardf(1.1, 1.1));
  DOIT(std::nexttowardl(1.1, 1.1));
  DOIT(std::remainder(1.1, 1.1));
  DOIT(std::remainderf(1.1, 1.1));
  DOIT(std::remainderl(1.1, 1.1));
  int quo;
  DOIT(std::remquo(1.1, 1.1, &quo));
  DOIT(std::remquof(1.1, 1.1, &quo));
  DOIT(std::remquol(1.1, 1.1, &quo));
  DOIT(std::rint(1.1));
  DOIT(std::rintf(1.1));
  DOIT(std::rintl(1.1));
  DOIT(std::round(1.1));
  DOIT(std::roundf(1.1));
  DOIT(std::roundl(1.1));
  DOIT(std::scalbln(1.1, 1));
  DOIT(std::scalblnf(1.1, 1));
  DOIT(std::scalblnl(1.1, 1));
  DOIT(std::scalbn(1.1, 1));
  DOIT(std::scalbnf(1.1, 1));
  DOIT(std::scalbnl(1.1, 1));
  DOIT(std::tgamma(1.1));
  DOIT(std::tgammaf(1.1));
  DOIT(std::tgammal(1.1));
  DOIT(std::trunc(1.1));
  DOIT(std::truncf(1.1));
  DOIT(std::truncl(1.1));
  return 0;
}
