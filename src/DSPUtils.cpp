#include "DSPUtils.hpp"
#include <chrono>
#include <cmath>
#include <complex>
#include <iostream>
#include <vector>

using Complex = std::complex<double>;

void
DSPUtils::BitReversal(std::vector<Complex>& signal)
{
    auto n      = signal.size();
    int  levels = log2(n);

    for (int i = 0; i < n; ++i) {
        int j = 0;
        for (int bit = 0; bit < levels; ++bit) {
            if (i & (1 << bit)) {
                j |= (1 << (levels - 1 - bit));
            }
        }
        if (j > i) {
            std::swap(signal[i], signal[j]);
        }
    }
}

void
DSPUtils::ZeroPadding(std::vector<Complex>& signal)
{
    int signalSize = signal.size();
    int nextPow2   = 1;
    while (nextPow2 < signalSize) {
        nextPow2 <<= 1;
    }

    if (nextPow2 > signalSize) {
        signal.resize(nextPow2, 0.0);
    }
}

std::vector<std::pair<double, double>>
DSPUtils::FindPeaks(std::vector<Complex>& signal, std::chrono::nanoseconds sampling_period, int max_peaks)
{
    std::vector<std::pair<double, double>> peak_data(max_peaks);
    const auto                             tolerance = 0.10;
    const auto t_s         = std::chrono::duration_cast<std::chrono::duration<double>>(sampling_period).count();
    const int  signal_size = signal.size();
    for (int i = 1; i <= signal_size / 2; ++i) {
        const auto amplitude = 2 * std::abs(signal[i]) / signal_size;
        auto       min_it    = std::min_element(peak_data.begin(), peak_data.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });
        if (min_it != peak_data.end() && amplitude > min_it->first) {
            const auto frequency = i * 1 / (signal_size * t_s);
            auto       neighbor_it
                = std::find_if(peak_data.begin(), peak_data.end(), [&frequency, &tolerance](const auto& peak) {
                      return std::abs(1 - frequency / peak.second) < tolerance;
                  });
            if (neighbor_it != peak_data.end() && amplitude > neighbor_it->first) {
                *neighbor_it = {amplitude, frequency};
            }
            else {
                *min_it = {amplitude, frequency};
            }
        }
    }
    return peak_data;
}
