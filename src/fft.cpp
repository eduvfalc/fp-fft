#include "fft.hpp"
#include <cmath>
#include <complex>
#include <memory>
#include <numbers>
#include <vector>
#include "fft_types.hpp"

void
FFT::Compute(std::vector<Complex>& signal)
{
    mDSPUtils->ZeroPadding(signal);
    mDSPUtils->BitReversal(signal);

    int n = signal.size();
    for (uint32_t len = 2; len <= n; len <<= 1) {
        double  angle = 2 * std::numbers::pi / len;
        Complex wlen(std::cos(angle), std::sin(angle));

        for (uint32_t i = 0; i < n; i += len) {
            Complex w(1);
            for (uint32_t j = 0; j < len / 2; ++j) {
                auto u               = signal[i + j];
                auto v               = w * signal[i + j + len / 2];
                signal[i + j]           = u + v;
                signal[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}