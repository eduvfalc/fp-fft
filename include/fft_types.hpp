#ifndef H_FFT_TYPES_HPP
#define H_FFT_TYPES_HPP

#include <complex>
#include "cnl/all.h"

using cnl_type = cnl::fixed_point<
    cnl::rounding_integer<cnl::elastic_integer<24, int32_t>, cnl::_impl::tag_t<cnl::rounding_integer<>>>,
    -20>;

using Complex = std::complex<cnl_type>;

#endif  // H_FFT_TYPES_HPP