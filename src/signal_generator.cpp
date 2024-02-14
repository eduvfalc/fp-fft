#include "signal_generator.hpp"
#include <math.h>
#include <numbers>
#include <vector>
#include "fft_types.hpp"

SignalGenerator::SignalGenerator(std::chrono::nanoseconds duration, std::chrono::nanoseconds sampling_period)
  : mDuration(duration)
  , mSamplingPeriod(sampling_period){};

void
SignalGenerator::GenerateSines(std::vector<Complex>&                         signal,
                               const std::vector<std::pair<double, double>>& parameters) const
{
    for (std::chrono::nanoseconds dt(0); dt <= mDuration; dt += mSamplingPeriod) {
        double signal_value = 0;
        for (const auto& signal_params : parameters) {
            signal_value
                += signal_params.first
                   * std::sin(2 * std::numbers::pi * signal_params.second * std::chrono::duration<double>(dt).count());
        }
        Complex sample{signal_value, 0};
        signal.emplace_back(sample);
    }
}

void
SignalGenerator::ApplyHannWindow(std::vector<Complex>& signal) const
{
    Complex correction_fator{2, 0};
    auto    signal_size = signal.size();
    for (int i = 0; i <= signal_size; ++i) {
        Complex hanning_bin = {0.5 - 0.5 * std::cos(2 * std::numbers::pi * i / signal_size), 0};
        signal[i]           = signal[i] * hanning_bin * correction_fator;
    }
}