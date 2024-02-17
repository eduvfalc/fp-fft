#include <chrono>
#include <functional>
#include <memory>
#include <numbers>
#include <numeric>
#include "dsp_utils.hpp"
#include "etl/vector.h"
#include "fft.hpp"
#include "fft_types.hpp"
#include "gtest/gtest.h"
#include "utils/include/signal_generator.hpp"
#include "utils/include/testing_utils.hpp"

using SignalParameters = std::vector<std::pair<double, double>>;
using namespace fftemb;

// buffer size
constexpr int k_buffer_size = 2048;

// error tolerances
constexpr auto k_peak_tolerance      = 0.17;
constexpr auto k_frequency_tolerance = 0.05;

// signal generator
constexpr std::chrono::nanoseconds k_duration        = std::chrono::seconds(2);
constexpr std::chrono::nanoseconds k_sampling_period = std::chrono::milliseconds(1);
const SignalGenerator              g_generator       = SignalGenerator(k_duration, k_sampling_period);

// signal generator functions
auto pSineWaveGenerator
    = std::bind(&SignalGenerator::generate_sine_wave, &g_generator, std::placeholders::_1, std::placeholders::_2);
auto pSquareWaveGenerator
    = std::bind(&SignalGenerator::generate_square_wave, &g_generator, std::placeholders::_1, std::placeholders::_2);

class TestSinusoidFFT
  : public ::testing::TestWithParam<
        std::pair<std::function<void(etl::ivector<Complex>&, const SignalParameters&)>, SignalParameters>>
{
protected:
    std::shared_ptr<DSPUtils> dsp_utils = std::make_shared<DSPUtils>();
    std::shared_ptr<FFT>      fft       = std::make_shared<FFT>(dsp_utils);
};

class TestSquareFFT
  : public ::testing::TestWithParam<std::pair<std::function<void(etl::ivector<Complex>&, double)>, double>>
{
protected:
    std::shared_ptr<DSPUtils> dsp_utils = std::make_shared<DSPUtils>();
    std::shared_ptr<FFT>      fft       = std::make_shared<FFT>(dsp_utils);
};

TEST_P(TestSinusoidFFT, SinusoidSpectrumWithinTolerance)
{
    etl::vector<Complex, k_buffer_size> test_signal(k_buffer_size);
    auto                                test_params       = GetParam();
    auto                                signal_parameters = test_params.second;
    test_params.first(test_signal, signal_parameters);
    auto max_peak = dsp_utils->normalize(test_signal);
    dsp_utils->apply_hann_window(test_signal);

    fft->compute(test_signal);

    auto peak_data = dsp_utils->find_peaks(test_signal, k_sampling_period, signal_parameters.size());
    for (auto& peak : peak_data) {
        peak.first *= max_peak;
    }
    test_utils::sort_pairs(peak_data);
    test_utils::sort_pairs(signal_parameters);
    const auto peak_errors = test_utils::calculate_error(peak_data, signal_parameters);
    for (const auto& error : peak_errors) {
        EXPECT_LE(error.first, k_peak_tolerance);
        EXPECT_LE(error.second, k_frequency_tolerance);
    }
}

TEST_P(TestSquareFFT, SquareWaveSpectrumWithinTolerance)
{
    etl::vector<Complex, k_buffer_size>    test_signal(k_buffer_size);
    auto                                   test_params = GetParam();
    std::vector<std::pair<double, double>> signal_parameters;
    auto                                   frequency = test_params.second;
    test_params.first(test_signal, frequency);
    auto max_peak = dsp_utils->normalize(test_signal);
    dsp_utils->apply_hann_window(test_signal);
    auto num_peaks = 3;
    for (int i = 1; i <= num_peaks * 2; i += 2) {
        signal_parameters.emplace_back(std::make_pair(4 / (i * std::numbers::pi), i * frequency));
    }

    fft->compute(test_signal);

    auto peak_data = dsp_utils->find_peaks(test_signal, k_sampling_period, num_peaks);
    for (auto& peak : peak_data) {
        peak.first *= max_peak;
    }
    test_utils::sort_pairs(peak_data);
    test_utils::sort_pairs(signal_parameters);
    const auto peak_errors = test_utils::calculate_error(peak_data, signal_parameters);
    for (const auto& error : peak_errors) {
        EXPECT_LE(error.first, k_peak_tolerance);
        EXPECT_LE(error.second, k_frequency_tolerance);
    }
}


INSTANTIATE_TEST_CASE_P(TestSinusoidSpectra,
                        TestSinusoidFFT,
                        ::testing::Values(std::make_pair(pSineWaveGenerator, SignalParameters{{5, 60}}),
                                          std::make_pair(pSineWaveGenerator, SignalParameters{{5, 60}, {10, 100}}),
                                          std::make_pair(pSineWaveGenerator,
                                                         SignalParameters{{8, 30}, {3, 60}, {12, 90}})));

INSTANTIATE_TEST_CASE_P(TestSquareWaveSpectra,
                        TestSquareFFT,
                        ::testing::Values(std::make_pair(pSquareWaveGenerator, 60),
                                          std::make_pair(pSquareWaveGenerator, 90)));