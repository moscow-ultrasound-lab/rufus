// SyntheticApertureFocuserDebug.cpp
#include "Pre.h"
#include "SyntheticApertureFocuserDebug.h"
//#include <XRADBasic/FFT.h>
#include <cmath>
#include <algorithm>

XRAD_BEGIN

// ============================================================
// Отображение A-line с теоретической кривой задержек
// ============================================================

// ============================================================
// Отображение A-line с теоретической кривой задержек (через GraphSet)
// ============================================================
void SyntheticApertureFocuserDebug::DisplayAlineWithWavefront(
    RawSFDataSource& source,
    const DebugFocusParams& params)
{
    printf("\n=== Wavefront Debug: A-line with delay curve ===\n");
    printf("  TX: %d, RX: %d\n", params.tx_element, params.rx_element);
    printf("  Focus depth sample: %d\n", params.focus_depth_sample);
    printf("  Speed: %.0f m/s\n", params.speed_of_sound);
    printf("  Pitch: %.3f mm\n", params.pitch_mm);
    fflush(stdout);

    auto row = source.GetRow(params.tx_element, params.rx_element);
    if (!row) {
        printf("ERROR: Cannot get row for TX=%d, RX=%d\n",
            params.tx_element, params.rx_element);
        return;
    }

    double fs = source.raw_signal_sample_rate.Hz();
    double pitch_cm = params.pitch_mm / 10.0;
    double c_cm = params.speed_of_sound * 100;
    double aperture_center = 0.5 * n_elements_debug * pitch_cm;

    double tx_pos = (params.tx_element + 0.5) * pitch_cm - aperture_center;
    double rx_pos = (params.rx_element + 0.5) * pitch_cm - aperture_center;
    double focus_depth_cm = params.focus_depth_sample * 0.5 * (c_cm / fs);

    size_t n_samples = n_samples_debug;

    // Подготавливаем данные
    RealFunctionF32 aline(n_samples);
    RealFunctionF32 envelope(n_samples);
    RealFunctionF32 normalized_delay(n_samples);

    float max_envelope = 0;
    int max_envelope_idx = 0;
    double focus_sample = 0;

    for (size_t i = 0; i < n_samples; ++i) {
        aline[i] = (float)row->in((double)i).re;
        envelope[i] = cabs(row->in((double)i));

        if (envelope[i] > max_envelope) {
            max_envelope = envelope[i];
            max_envelope_idx = (int)i;
        }

        double depth_cm = i * 0.5 * (c_cm / fs);
        double tx_path = sqrt(tx_pos * tx_pos + depth_cm * depth_cm);
        double rx_path = sqrt(rx_pos * rx_pos + depth_cm * depth_cm);
        double total_time = (tx_path + rx_path) / c_cm;
        double sample_idx = total_time * fs;

        if (i == (size_t)params.focus_depth_sample) {
            focus_sample = sample_idx;
        }

        double diff = sample_idx - params.focus_depth_sample;
        double sigma = 15.0;
        normalized_delay[i] = max_envelope * exp(-diff * diff / (2 * sigma * sigma));
    }

    // --- Создаем GraphSet ---
    GraphSet gs(
        L"Wavefront Analysis - TX=" + ssprintf(L"%d", params.tx_element) +
        L" RX=" + ssprintf(L"%d", params.rx_element),
        L"Amplitude",
        L"Sample index"
    );

    // Добавляем основные графики
    gs.AddGraphUniform(aline, 0, 1, "A-line (RF)");
    gs.AddGraphUniform(envelope, 0, 1, "Envelope");
    gs.AddGraphUniform(normalized_delay, 0, 1, "Wavefront profile");

    // --- Создаем вертикальную линию для фокуса (красная пунктирная) ---
    RealFunctionF32 focus_line(n_samples, 0);
    float min_val = 0;
    float max_val = max_envelope;

    // Находим минимальное значение среди всех графиков для фона
    for (size_t i = 0; i < n_samples; ++i) {
        if (envelope[i] < min_val) min_val = envelope[i];
        if (aline[i] < min_val) min_val = aline[i];
    }
    min_val = min_val * 0.5; // Немного ниже минимума

    // Вертикальная линия фокуса: только в точке фокуса максимальное значение
    for (size_t i = 0; i < n_samples; ++i) {
        if (i == (size_t)params.focus_depth_sample) {
            focus_line[i] = max_val;
        }
        else {
            focus_line[i] = min_val;
        }
    }
    gs.AddGraphUniform(focus_line, 0, 1, "Focus (theoretical)");

    // --- Создаем вертикальную линию для реального максимума (синяя пунктирная) ---
    RealFunctionF32 actual_peak_line(n_samples, 0);
    for (size_t i = 0; i < n_samples; ++i) {
        if (i == (size_t)max_envelope_idx) {
            actual_peak_line[i] = max_val;
        }
        else {
            actual_peak_line[i] = min_val;
        }
    }
    gs.AddGraphUniform(actual_peak_line, 0, 1, "Actual peak");

    // --- Отображаем ---
    gs.Display();

    // Выводим статистику
    double focus_time = (sqrt(tx_pos * tx_pos + focus_depth_cm * focus_depth_cm) +
        sqrt(rx_pos * rx_pos + focus_depth_cm * focus_depth_cm)) / c_cm;
    double theoretical_sample = focus_time * fs;

    printf("\n=== Wavefront Analysis Results ===\n");
    printf("  Theoretical focus sample: %.1f\n", theoretical_sample);
    printf("  Actual max envelope at sample: %d\n", max_envelope_idx);
    printf("  Difference: %.1f samples (%.2f mm)\n",
        theoretical_sample - max_envelope_idx,
        (theoretical_sample - max_envelope_idx) * 0.5 * (c_cm / fs) * 10);
    printf("  Max envelope amplitude: %.3f\n", max_envelope);
    fflush(stdout);
}



// ============================================================
// Отображение 2D карты с наложением волнового фронта
// ============================================================
void SyntheticApertureFocuserDebug::Display2DWithWavefront(
    RawSFDataSource& source,
    const DebugFocusParams& params)
{
    printf("\n=== 2D Wavefront Visualization ===\n");

    // Получаем данные
    auto row = source.GetRow(params.tx_element, params.rx_element);
    if (!row) {
        printf("ERROR: Cannot get row for TX=%d, RX=%d\n",
            params.tx_element, params.rx_element);
        return;
    }

    // Параметры
    double fs = source.raw_signal_sample_rate.Hz();
    double pitch_cm = params.pitch_mm / 10.0;
    double c_cm = params.speed_of_sound * 100;
    double aperture_center = 0.5 * n_elements_debug * pitch_cm;

    double tx_pos = (params.tx_element + 0.5) * pitch_cm - aperture_center;
    double rx_pos = (params.rx_element + 0.5) * pitch_cm - aperture_center;
    double focus_depth_cm = params.focus_depth_sample * 0.5 * (c_cm / fs);

    // Создаем 2D изображение: [samples x elements]
    int n_elements_to_show = 64;
    size_t n_samples = n_samples_debug;
    int start_rx = std::max(0, params.rx_element - n_elements_to_show / 2);
    int end_rx = std::min(192, start_rx + n_elements_to_show);
    start_rx = std::max(0, end_rx - n_elements_to_show);

    // Создаем 2D массивы для отображения
    ComplexFunction2D_F32 image(n_samples, end_rx - start_rx, complexF32(0));

    // Заполняем изображение
    printf("  Building 2D map for RX elements %d..%d\n", start_rx, end_rx - 1);

    for (int rx = start_rx; rx < end_rx; ++rx) {
        int col = rx - start_rx;
        auto row_data = source.GetRow(params.tx_element, rx);
        if (!row_data) continue;

        for (int s = 0; s < n_samples; ++s) {
            image.at(s, col) = row_data->in(s);
        }
    }

    // Вычисляем амплитуду
    ComplexFunction2D_F32 amplitude(image.vsize(), image.hsize());
    float max_val = 0;
    for (size_t r = 0; r < image.vsize(); ++r) {
        for (size_t c = 0; c < image.hsize(); ++c) {
            amplitude.at(r, c) = cabs(image.at(r, c));
            if (cabs(amplitude.at(r, c)) > max_val) {
                max_val = cabs(amplitude.at(r, c));
            }
        }
    }

    // Нормализуем
    if (max_val > 0) {
        for (size_t r = 0; r < image.vsize(); ++r) {
            for (size_t c = 0; c < image.hsize(); ++c) {
                amplitude.at(r, c) = (complexF32)(cabs(amplitude.at(r, c)) / max_val);
            }
        }
    }

    // Отображаем 2D карту с помощью DisplayMathFunction2D
    // (GraphSet не поддерживает 2D напрямую, используем DisplayMathFunction2D)
    DisplayMathFunction2D(amplitude,
        ssprintf("RF Data 2D - TX=%d RX=%d", params.tx_element, params.rx_element));

    // Дополнительно отображаем профиль вдоль строки для выбранной глубины
    int selected_depth = params.focus_depth_sample;
    if (selected_depth < n_samples) {
        RealFunctionF32 depth_profile(end_rx - start_rx);
        for (int rx = start_rx; rx < end_rx; ++rx) {
            int col = rx - start_rx;
            depth_profile[col] = cabs(image.at(selected_depth, col));
        }

        GraphSet gs(
            L"Depth Profile at sample " + ssprintf(L"%d", selected_depth),
            L"Amplitude",
            L"Element index"
        );

        gs.AddGraphUniform(depth_profile, start_rx, 1,
            ssprintf(L"Depth=%.2f cm", selected_depth * 0.5 * (c_cm / fs)));

        gs.Display();
    }

    fflush(stdout);
}

// ============================================================
// Поиск оптимальной скорости звука
// ============================================================

    double SyntheticApertureFocuserDebug::FindOptimalSpeed(
        RawSFDataSource& source,
        int tx_element,
        int rx_element,
        int focus_depth_sample,
        double speed_min,
        double speed_max,
        int steps)
    {
        printf("\n=== Speed of Sound Optimization ===\n");
        printf("  TX: %d, RX: %d\n", tx_element, rx_element);
        printf("  Focus depth sample: %d\n", focus_depth_sample);
        printf("  Speed range: %.0f - %.0f m/s\n", speed_min, speed_max);
        fflush(stdout);

        auto row = source.GetRow(tx_element, rx_element);
        if (!row) {
            printf("ERROR: Cannot get row for TX=%d, RX=%d\n", tx_element, rx_element);
            return 1540.0;
        }

        double fs = source.raw_signal_sample_rate.Hz();
        double pitch_cm = source.array_pitch.mm() / 10.0;
        double aperture_center = 0.5 * n_elements_debug * pitch_cm;

        double tx_pos = (tx_element + 0.5) * pitch_cm - aperture_center;
        double rx_pos = (rx_element + 0.5) * pitch_cm - aperture_center;

        std::vector<double> speeds(steps);
        std::vector<double> correlations(steps);

        // Получаем сигнал вокруг фокальной точки
        int window = 20; // семплов вокруг фокуса
        int start_sample = std::max(0, focus_depth_sample - window);
        int end_sample = focus_depth_sample + window;
        size_t n_window = end_sample - start_sample;

        ComplexFunctionF32 reference_signal(n_window);
        for (size_t i = 0; i < n_window; ++i) {
            reference_signal[i] = row->in(start_sample + i);
        }

        printf("  Reference signal: samples %d..%d\n", start_sample, end_sample - 1);

        // Для каждой скорости вычисляем корреляцию
        for (int step = 0; step < steps; ++step) {
            double c = speed_min + step * (speed_max - speed_min) / (steps - 1);
            double c_cm = c * 100;
            speeds[step] = c;

            // Вычисляем теоретическое время для фокуса
            double focus_depth_cm = focus_depth_sample * 0.5 * (c_cm / fs);
            double tx_to_focus = sqrt(tx_pos * tx_pos + focus_depth_cm * focus_depth_cm) / c_cm;
            double rx_to_focus = sqrt(rx_pos * rx_pos + focus_depth_cm * focus_depth_cm) / c_cm;
            double focus_time = tx_to_focus + rx_to_focus;

            // Извлекаем сигнал в соответствии с теоретическим временем
            ComplexFunctionF32 test_signal(n_window);
            for (size_t i = 0; i < n_window; ++i) {
                // Вычисляем глубину для каждого семпла
                double depth_cm = (start_sample + i) * 0.5 * (c_cm / fs);
                double tx_path = sqrt(tx_pos * tx_pos + depth_cm * depth_cm);
                double rx_path = sqrt(rx_pos * rx_pos + depth_cm * depth_cm);
                double total_time = (tx_path + rx_path) / c_cm;
                double sample_idx = total_time * fs;

                test_signal[i] = row->in(sample_idx);
            }

            // Вычисляем корреляцию
            double corr = 0;
            double ref_power = 0;
            double test_power = 0;
            for (size_t i = 0; i < n_window; ++i) {
                corr += (reference_signal[i].re * test_signal[i].re +
                    reference_signal[i].im * test_signal[i].im);
                ref_power += reference_signal[i].re * reference_signal[i].re +
                    reference_signal[i].im * reference_signal[i].im;
                test_power += test_signal[i].re * test_signal[i].re +
                    test_signal[i].im * test_signal[i].im;
            }

            correlations[step] = corr / (sqrt(ref_power) * sqrt(test_power));
        }

        // Находим максимум корреляции
        double max_corr = -1;
        double optimal_speed = 1540.0;
        int optimal_idx = 0;
        for (int step = 0; step < steps; ++step) {
            if (correlations[step] > max_corr) {
                max_corr = correlations[step];
                optimal_speed = speeds[step];
                optimal_idx = step;
            }
        }

        // --- Отображаем результаты через GraphSet ---

        // Создаем массивы для отображения
        RealFunctionF32 corr_func(steps);
        for (int step = 0; step < steps; ++step) {
            corr_func[step] = (float)correlations[step];
        }

        // Создаем GraphSet
        GraphSet gs(
            L"Speed of Sound Optimization - Max at " + ssprintf(L"%.0f", optimal_speed) + L" m/s",
            L"Correlation coefficient",
            L"Speed of sound (m/s)"
        );

        // Добавляем график корреляции (синий)
        gs.AddGraphUniform(corr_func, speed_min, (speed_max - speed_min) / steps,
            "Correlation");

        // --- Создаем вертикальную линию через массив ---
        // Создаем массив, где все элементы равны минимальному значению корреляции,
        // а в точке оптимума - максимальному значению
        RealFunctionF32 vertical_line(steps);
        float min_corr = 0;
        float max_corr_val = (float)max_corr;

        // Находим минимальное значение корреляции
        for (int step = 0; step < steps; ++step) {
            if (corr_func[step] < min_corr) {
                min_corr = corr_func[step];
            }
        }

        // Заполняем массив: везде min_corr, в точке оптимума max_corr
        for (int step = 0; step < steps; ++step) {
            if (step == optimal_idx) {
                vertical_line[step] = max_corr_val;
            }
            else {
                vertical_line[step] = min_corr;
            }
        }

        // Добавляем вертикальную линию (красная)
        gs.AddGraphUniform(vertical_line, speed_min, (speed_max - speed_min) / steps,
            "Optimal speed");

        // --- Отображаем ---
        gs.Display();

        printf("\n=== Optimization Results ===\n");
        printf("  Optimal speed: %.1f m/s\n", optimal_speed);
        printf("  Max correlation: %.3f\n", max_corr);
        fflush(stdout);

        return optimal_speed;
    }

// ============================================================
// Визуализация профиля волны на кадре SA данных
// ============================================================

    void SyntheticApertureFocuserDebug::VisualizeWavefrontOnSAData(
        RawSFDataSource& source,
        int frame_index,
        int focus_depth_sample,
        double speed_of_sound)
    {
        printf("\n========================================\n");
        printf("=== WAVEFRONT FROM SCATTERER ===\n");
        printf("========================================\n");
        printf("This mode visualizes the theoretical spherical wavefront\n");
        printf("from a selected scatterer (bright spot in the image).\n");
        printf("You can compare it with the actual SA data.\n");
        fflush(stdout);

        // Получаем размеры данных
        size_t n_tx = source.n_tx_elements;
        size_t n_rx = source.n_rx_elements;
        size_t n_samples = source.samples_per_element;

        double fs = source.raw_signal_sample_rate.Hz();
        double pitch_cm = source.array_pitch.mm() / 10.0;
        double c_cm = speed_of_sound * 100;
        double aperture_center = 0.5 * n_rx * pitch_cm;

        // --- ШАГ 1: ПОСТРОЕНИЕ ИЗОБРАЖЕНИЯ SA ДАННЫХ ---
        printf("\n[1] Building SA data image...\n");

        int n_elements_to_show = (int)n_rx;
        int n_samples_to_show = std::min((int)n_samples, 3000);

        // Используем переданный frame_index как TX элемент
        int tx_to_use = frame_index;
        if (tx_to_use < 0 || tx_to_use >= (int)n_tx) {
            printf("  WARNING: frame_index %d out of range [0, %zu]. Using 0.\n", tx_to_use, n_tx - 1);
            tx_to_use = 0;
        }

        printf("  Using TX element: %d\n", tx_to_use);
        printf("  Building SA image (%d samples x %d elements)...\n", n_samples_to_show, n_elements_to_show);

        ComplexFunction2D_F32 sa_image(n_samples_to_show, n_elements_to_show, complexF32(0));
        ComplexFunction2D_F32 amplitude(n_samples_to_show, n_elements_to_show, complexF32(0));

        float max_amp = 0;
        for (int rx = 0; rx < n_elements_to_show; ++rx) {
            auto row_data = source.GetRow(tx_to_use, rx);
            if (!row_data) continue;

            for (int s = 0; s < n_samples_to_show; ++s) {
                complexF32 val = row_data->in(s);
                sa_image.at(s, rx) = val;
                float amp = cabs(val);
                amplitude.at(s, rx) = complexF32(amp, 0);
                if (amp > max_amp) max_amp = amp;
            }
        }

        // Нормализуем амплитуду для отображения
        if (max_amp > 0) {
            for (int s = 0; s < n_samples_to_show; ++s) {
                for (int rx = 0; rx < n_elements_to_show; ++rx) {
                    amplitude.at(s, rx) = (complexF32)(cabs(amplitude.at(s, rx)) / max_amp);
                }
            }
        }

        // Отображаем SA данные для выбора отражателя
        printf("\n[2] SA Data Image displayed. Find a bright spot (scatterer).\n");
        DisplayMathFunction2D(amplitude,
            ssprintf("SA Data - TX=%d (Click on a bright spot)", tx_to_use));

        // --- ШАГ 2: ИНТЕРАКТИВНЫЙ ВЫБОР ОТРАЖАТЕЛЯ ---
        printf("\n[3] SELECT SCATTERER\n");
        printf("  On the displayed image, identify a bright spot (tissue interface).\n");
        printf("  Typical scatterers: vessel wall, tissue boundary, point target.\n");
        printf("\n  Enter coordinates of the scatterer:\n");

        // Показываем доступные диапазоны
        printf("\n  Available ranges:\n");
        printf("    Sample (depth): 0..%d\n", n_samples_to_show - 1);
        printf("    Element (lateral): 0..%d\n", n_elements_to_show - 1);

        // Запрашиваем координаты отражателя
        int scatterer_sample = GetSigned(
            "Scatterer depth (sample index)",
            MakeGUIValue(focus_depth_sample, saved_default_value),
            0, n_samples_to_show - 1);

        int scatterer_element = GetSigned(
            "Scatterer lateral position (element index)",
            MakeGUIValue(n_elements_to_show / 2, saved_default_value),
            0, n_elements_to_show - 1);

        // Вычисляем физические координаты отражателя
        double scatterer_x = (scatterer_element + 0.5) * pitch_cm - aperture_center;
        double scatterer_depth_cm = scatterer_sample * 0.5 * (c_cm / fs);
        double scatterer_depth_mm = scatterer_depth_cm * 10;

        printf("\n=== SCATTERER POSITION ===\n");
        printf("  Sample: %d\n", scatterer_sample);
        printf("  Element: %d\n", scatterer_element);
        printf("  Depth: %.2f cm (%.1f mm)\n", scatterer_depth_cm, scatterer_depth_mm);
        printf("  Lateral position: %.2f cm (%.1f mm)\n", scatterer_x, scatterer_x * 10);
        fflush(stdout);

        // --- ШАГ 3: ПОСТРОЕНИЕ ТЕОРЕТИЧЕСКОГО ФРОНТА ---
        printf("\n[4] Building theoretical wavefront from scatterer...\n");
        printf("  Using speed of sound: %.0f m/s\n", speed_of_sound);
        printf("  Pitch: %.3f mm\n", pitch_cm * 10);

        RealFunction2D_F32 wavefront_profile(n_samples_to_show, n_elements_to_show, 0);
        RealFunction2D_F32 wavefront_time(n_samples_to_show, n_elements_to_show, 0);

        // Ширина гауссиана в семплах (соответствует длительности импульса)
        double sigma_samples = 12.0;

        // Позиция передающего элемента (для расчета времени излучения)
        double tx_pos = (tx_to_use + 0.5) * pitch_cm - aperture_center;

        int wavefront_center_element = -1;
        double min_time_diff = 1e9;

        for (int rx = 0; rx < n_elements_to_show; ++rx) {
            double rx_pos = (rx + 0.5) * pitch_cm - aperture_center;

            // Расстояние от передатчика до отражателя
            double tx_to_scatterer = sqrt((tx_pos - scatterer_x) * (tx_pos - scatterer_x) +
                scatterer_depth_cm * scatterer_depth_cm);
            double time_tx_to_scatterer = tx_to_scatterer / c_cm;

            // Расстояние от отражателя до приемного элемента
            double scatterer_to_rx = sqrt((scatterer_x - rx_pos) * (scatterer_x - rx_pos) +
                scatterer_depth_cm * scatterer_depth_cm);
            double time_scatterer_to_rx = scatterer_to_rx / c_cm;

            // Общее время распространения (эхо)
            double total_time = time_tx_to_scatterer + time_scatterer_to_rx;
            double sample_idx = total_time * fs;

            // Находим элемент, где время минимально (центр волнового фронта)
            if (sample_idx < min_time_diff) {
                min_time_diff = sample_idx;
                wavefront_center_element = rx;
            }

            // Заполняем профиль волнового фронта
            for (int s = 0; s < n_samples_to_show; ++s) {
                // Гауссиан вокруг теоретического времени прихода
                double diff = s - sample_idx;
                double amplitude_wf = exp(-diff * diff / (2 * sigma_samples * sigma_samples));
                wavefront_profile.at(s, rx) = (float)amplitude_wf;
                wavefront_time.at(s, rx) = (float)sample_idx;
            }
        }

        printf("  Wavefront center at element: %d\n", wavefront_center_element);
        printf("  Minimum time: %.2f us at sample %.1f\n",
            min_time_diff / fs * 1e6, min_time_diff);
        fflush(stdout);

        // --- ШАГ 4: ОТОБРАЖЕНИЕ РЕЗУЛЬТАТОВ ---
        printf("\n[5] Displaying results...\n");

        // 4a. Отображаем SA данные с маркером отражателя
        ComplexFunction2D_F32 sa_with_marker = amplitude;

        // Добавляем маркер отражателя (красный крест)
        int marker_size = 5;
        for (int dr = -marker_size; dr <= marker_size; ++dr) {
            for (int dc = -marker_size; dc <= marker_size; ++dc) {
                int r = scatterer_sample + dr;
                int c = scatterer_element + dc;
                if (r >= 0 && r < (int)sa_with_marker.vsize() &&
                    c >= 0 && c < (int)sa_with_marker.hsize()) {
                    float dist = sqrt((float)(dr * dr + dc * dc));
                    if (dist <= marker_size) {
                        float brightness = 1.0f - dist / (marker_size + 1);
                        // Используем зеленый цвет для маркера
                        sa_with_marker.at(r, c) = complexF32(0, brightness * 1.2f);
                    }
                }
            }
        }

        DisplayMathFunction2D(sa_with_marker,
            ssprintf("SA Data with Scatterer Marker (TX=%d, Sample=%d, Element=%d)",
                tx_to_use, scatterer_sample, scatterer_element));

        // 4b. Отображаем теоретический волновой фронт
        DisplayMathFunction2D(wavefront_profile,
            ssprintf("Theoretical Wavefront from Scatterer (c=%.0f m/s)", speed_of_sound));

        // 4c. Отображаем карту времени прихода
        DisplayMathFunction2D(wavefront_time,
            "Time-of-Flight Map (samples)");

        // 4d. Наложение: SA данные + волновой фронт
        ComplexFunction2D_F32 overlay(n_samples_to_show, n_elements_to_show, complexF32(0));
        for (int s = 0; s < n_samples_to_show; ++s) {
            for (int rx = 0; rx < n_elements_to_show; ++rx) {
                float sa_val = cabs(amplitude.at(s, rx));
                float wf_val = wavefront_profile.at(s, rx);
                // SA данные - синий канал, волновой фронт - красный канал
                overlay.at(s, rx) = complexF32(wf_val * 0.8f, sa_val);
            }
        }

        DisplayMathFunction2D(overlay,
            ssprintf("Overlay: SA Data (blue) + Wavefront (red) - c=%.0f m/s", speed_of_sound));

        // --- ШАГ 5: АНАЛИЗ СОВПАДЕНИЯ ---
        printf("\n[6] Wavefront matching analysis...\n");

        // Находим реальные пики в SA данных на глубине отражателя
        struct PeakInfo {
            int element;
            float amplitude;
            float sample_idx;
        };
        std::vector<PeakInfo> peaks_at_depth;

        for (int rx = 0; rx < n_elements_to_show; ++rx) {
            // Ищем максимум в окрестности глубины отражателя
            int search_range = 20;
            float max_val = 0;
            int max_idx = scatterer_sample;
            for (int s = std::max(0, scatterer_sample - search_range);
                s < std::min(n_samples_to_show, scatterer_sample + search_range); ++s) {
                float val = cabs(amplitude.at(s, rx));
                if (val > max_val) {
                    max_val = val;
                    max_idx = s;
                }
            }

            if (max_val > 0.05) { // Порог
                PeakInfo peak;
                peak.element = rx;
                peak.amplitude = max_val;
                peak.sample_idx = (float)max_idx;
                peaks_at_depth.push_back(peak);
            }
        }

        printf("  Found %zu peaks at scatterer depth\n", peaks_at_depth.size());

        if (!peaks_at_depth.empty()) {
            // Находим центр волнового фронта в реальных данных
            int center_element = 0;
            float min_sample = 1e9;
            for (const auto& p : peaks_at_depth) {
                if (p.sample_idx < min_sample) {
                    min_sample = p.sample_idx;
                    center_element = p.element;
                }
            }

            printf("  Wavefront center in SA data: element %d\n", center_element);
            printf("  Theoretical wavefront center: element %d\n", wavefront_center_element);
            printf("  Difference: %d elements (%.2f mm)\n",
                center_element - wavefront_center_element,
                (center_element - wavefront_center_element) * pitch_cm * 10);

            // Проверяем качество совпадения
            if (abs(center_element - wavefront_center_element) < 3) {
                printf("\n  ✅ GOOD: Wavefront matches SA data!\n");
                printf("  Speed of sound and pitch are correct.\n");
            }
            else {
                printf("\n  ❌ MISMATCH: Wavefront does not match SA data.\n");
                printf("  Possible causes:\n");
                printf("    - Wrong speed of sound (differs by %.0f m/s)\n",
                    speed_of_sound - 1540.0);
                printf("    - Wrong pitch (differs by %.3f mm)\n",
                    pitch_cm * 10 - 0.245);
                printf("    - Wrong scatterer position\n");
                printf("    - Wrong sampling frequency\n");

                // Предлагаем автоматическую коррекцию
                if (YesOrNo("Try to automatically find correct speed?",
                    MakeGUIValue(true, saved_default_value))) {

                    printf("\n  Searching for optimal speed...\n");

                    double best_speed = speed_of_sound;
                    float best_match = 0;
                    std::vector<double> test_speeds;
                    std::vector<float> match_scores;

                    GUIProgressBar progress;
                    progress.start("Speed search", 40);

                    for (int s = 0; s < 40; ++s) {
                        double test_speed = 1400 + s * (300.0 / 39.0);
                        double test_c_cm = test_speed * 100;

                        // Пересчитываем волновой фронт
                        float match = 0;
                        for (int rx = 0; rx < n_elements_to_show; ++rx) {
                            double rx_pos = (rx + 0.5) * pitch_cm - aperture_center;
                            double scatterer_to_rx = sqrt((scatterer_x - rx_pos) * (scatterer_x - rx_pos) +
                                scatterer_depth_cm * scatterer_depth_cm);
                            double total_time = (sqrt((tx_pos - scatterer_x) * (tx_pos - scatterer_x) +
                                scatterer_depth_cm * scatterer_depth_cm) +
                                scatterer_to_rx) / test_c_cm;
                            double sample_idx = total_time * fs;

                            int s_idx = (int)round(sample_idx);
                            if (s_idx >= 0 && s_idx < n_samples_to_show) {
                                float val = cabs(amplitude.at(s_idx, rx));
                                match += val;
                            }
                        }

                        test_speeds.push_back(test_speed);
                        match_scores.push_back(match);

                        if (match > best_match) {
                            best_match = match;
                            best_speed = test_speed;
                        }
                        progress.next();
                    }
                    progress.end();

                    printf("\n  Optimal speed: %.0f m/s\n", best_speed);
                    printf("  (Current speed: %.0f m/s)\n", speed_of_sound);

                    // Отображаем график зависимости match от скорости
                    GraphSet gs_speed(
                        L"Speed of Sound Optimization",
                        L"Match Score",
                        L"Speed (m/s)"
                    );

                    RealFunctionF32 match_func(40);
                    for (int i = 0; i < 40; ++i) {
                        match_func[i] = match_scores[i];
                    }
                    gs_speed.AddGraphUniform(match_func, 1400, 300.0 / 39.0,
                        "Match Score");
                    gs_speed.Display();

                    if (YesOrNo("Update speed to optimal value?",
                        MakeGUIValue(true, saved_default_value))) {
                        printf("  Speed updated to %.0f m/s\n", best_speed);
                        // Перерисовываем с новой скоростью
                        VisualizeWavefrontOnSAData(source, frame_index, scatterer_sample, best_speed);
                        return;
                    }
                }
            }
        }

        // --- ШАГ 6: ИНТЕРПРЕТАЦИЯ РЕЗУЛЬТАТОВ ---
        printf("\n[7] INTERPRETATION GUIDE\n");
        printf("========================================\n");
        printf("  ✅ If red wavefront follows bright spots in SA data:\n");
        printf("     → Speed of sound and geometry are CORRECT\n");
        printf("\n  ⚠️ If red wavefront is shifted horizontally:\n");
        printf("     → Check: Speed of sound (incorrect)\n");
        printf("     → Fix: Increase speed if shifted right, decrease if left\n");
        printf("\n  ⚠️ If red wavefront has wrong curvature:\n");
        printf("     → Check: Pitch (incorrect)\n");
        printf("     → Fix: Increase pitch if too narrow, decrease if too wide\n");
        printf("\n  ⚠️ If red wavefront is flat but SA data shows curve:\n");
        printf("     → Check: Element order (incorrect)\n");
        printf("     → Fix: Reverse element order or check read order\n");
        printf("\n  ⚠️ If wavefront doesn't move when changing speed:\n");
        printf("     → Check: Sampling frequency (incorrect)\n");
        printf("     → Fix: Adjust fs in source object\n");
        printf("========================================\n");
        fflush(stdout);
    }

// ============================================================
// ДИАГНОСТИКА ПАРАМЕТРОВ - добавьте эту функцию в класс
// ============================================================

void SyntheticApertureFocuserDebug::DiagnoseParameters(
    RawSFDataSource& source,
    const DebugFocusParams& params)
{
    printf("\n========================================\n");
    printf("=== PARAMETER DIAGNOSTICS ===\n");
    printf("========================================\n");

    double fs = source.raw_signal_sample_rate.Hz();
    double c = params.speed_of_sound;
    double c_cm = c * 100;
    double pitch_cm = params.pitch_mm / 10.0;
    double aperture_center = 0.5 * n_elements_debug * pitch_cm;

    double tx_pos = (params.tx_element + 0.5) * pitch_cm - aperture_center;
    double rx_pos = (params.rx_element + 0.5) * pitch_cm - aperture_center;

    // 1. Проверяем частоту дискретизации
    printf("\n[1] SAMPLING FREQUENCY CHECK\n");
    printf("  Reported fs: %.3f MHz\n", fs / 1e6);
    printf("  Expected fs for ULA-OP: 78.125 MHz (or 39.0625 MHz with decimation)\n");

    // Проверяем, не является ли частота дискретизации вдвое меньше
    double possible_fs[] = { 78.125e6, 39.0625e6, 19.53125e6, 9.765625e6 };
    for (double test_fs : possible_fs) {
        double ratio = fs / test_fs;
        if (fabs(ratio - 1.0) < 0.01 || fabs(ratio - 0.5) < 0.01 || fabs(ratio - 2.0) < 0.01) {
            printf("  NOTE: fs=%.3f MHz is %.1f x of %.3f MHz\n",
                fs / 1e6, ratio, test_fs / 1e6);
        }
    }

    // 2. Получаем данные и анализируем сигнал
    printf("\n[2] SIGNAL ANALYSIS\n");
    auto row = source.GetRow(params.tx_element, params.rx_element);
    if (!row) {
        printf("  ERROR: Cannot get row for TX=%d, RX=%d\n",
            params.tx_element, params.rx_element);
        return;
    }

    size_t n_samples = n_samples_debug;

    // Находим все пики в сигнале (не только максимум)
    printf("\n  Finding peaks in signal...\n");

    // Используем огибающую для поиска пиков
    RealFunctionF32 envelope(n_samples);
    for (size_t i = 0; i < n_samples; ++i) {
        envelope[i] = cabs(row->in(i));
    }

    // Находим пики (локальные максимумы)
    struct PeakInfo {
        int index;
        float amplitude;
        float depth_cm;
        float snr;
    };
    std::vector<PeakInfo> peaks;

    // Порог для пиков (5% от максимума)
    float max_amp = 0;
    for (size_t i = 0; i < n_samples; ++i) {
        if (envelope[i] > max_amp) max_amp = envelope[i];
    }
    float threshold = max_amp * 0.05;

    // Ищем локальные максимумы
    for (size_t i = 1; i < n_samples - 1; ++i) {
        if (envelope[i] > envelope[i - 1] && envelope[i] > envelope[i + 1] && envelope[i] > threshold) {
            PeakInfo peak;
            peak.index = (int)i;
            peak.amplitude = envelope[i];
            peak.depth_cm = i * 0.5 * (c_cm / fs);

            // Вычисляем SNR (отношение сигнал/шум)
            int noise_window = 50;
            float noise_level = 0;
            
            for (int j = std::max(0, (int)i - noise_window); j < i - 10; ++j) {
                noise_level += envelope[j];
            }
            noise_level /= (noise_window);
            peak.snr = peak.amplitude / (noise_level + 0.001);

            peaks.push_back(peak);
        }
    }

    // Сортируем пики по глубине
    std::sort(peaks.begin(), peaks.end(),
        [](const PeakInfo& a, const PeakInfo& b) {
            return a.index < b.index;
        });

    printf("\n  Found %zu peaks:\n", peaks.size());
    for (size_t p = 0; p < peaks.size(); ++p) {
        printf("    Peak %zu: sample %d, depth %.2f cm, amplitude %.2f, SNR %.1f\n",
            p, peaks[p].index, peaks[p].depth_cm, peaks[p].amplitude, peaks[p].snr);
    }

    if (peaks.empty()) {
        printf("  WARNING: No significant peaks found!\n");
        printf("  Try changing TX/RX elements or increasing gain.\n");
        return;
    }

    // 3. Предлагаем выбрать пик для анализа
    printf("\n[3] SELECT PEAK FOR ANALYSIS\n");
    printf("  The first peak (at the skin surface) is usually NOT good for calibration.\n");
    printf("  Recommended: choose a peak at deeper tissue interface (thyroid, vessel wall, etc.)\n");
    printf("  Avoid peaks in the first 1-2 cm (near field artifacts).\n");

    // Отображаем пики с их глубинами
    printf("\n  Available peaks:\n");
    for (size_t p = 0; p < std::min(peaks.size(), (size_t)20); ++p) {
        char marker = ' ';
        if (peaks[p].depth_cm < 1.0) marker = '!'; // Слишком близко
        else if (peaks[p].depth_cm > 1.0 && peaks[p].depth_cm < 3.0) marker = '*'; // Хорошая зона
        else if (peaks[p].depth_cm >= 3.0) marker = '+'; // Глубокая зона
        printf("    %zu: sample %d, depth %.2f cm, amp %.2f, SNR %.1f %c\n",
            p, peaks[p].index, peaks[p].depth_cm, peaks[p].amplitude, peaks[p].snr, marker);
    }
    printf("    Legend: ! = too shallow (<1 cm), * = good (1-3 cm), + = deep (>3 cm)\n");

    // Предлагаем автоматический выбор пика на глубине > 1 см
    int selected_peak_idx = -1;
    printf("\n  Auto-selection: looking for peak at depth > 1.0 cm with good SNR...\n");

    for (size_t p = 0; p < peaks.size(); ++p) {
        if (peaks[p].depth_cm > 1.0 && peaks[p].snr > 2.0) {
            selected_peak_idx = (int)p;
            printf("  Auto-selected peak %d at depth %.2f cm (SNR=%.1f)\n",
                p, peaks[p].depth_cm, peaks[p].snr);
            break;
        }
    }

    if (selected_peak_idx < 0) {
        // Если нет пика > 1 см, используем самый сильный пик
        float max_snr = 0;
        for (size_t p = 0; p < peaks.size(); ++p) {
            if (peaks[p].snr > max_snr) {
                max_snr = peaks[p].snr;
                selected_peak_idx = (int)p;
            }
        }
        printf("  No good peak found > 1 cm. Using strongest peak at %.2f cm\n",
            peaks[selected_peak_idx].depth_cm);
    }

    // Даем пользователю выбрать
    int user_choice = GetSigned(
        "Enter peak index to analyze (or -1 to use auto-selected)",
        MakeGUIValue(selected_peak_idx, saved_default_value),
        -1, (int)peaks.size() - 1);

    if (user_choice >= 0 && user_choice < (int)peaks.size()) {
        selected_peak_idx = user_choice;
    }

    if (selected_peak_idx < 0 || selected_peak_idx >= (int)peaks.size()) {
        printf("  Invalid peak selection. Using auto-selected.\n");
    }

    const PeakInfo& selected_peak = peaks[selected_peak_idx];
    int max_idx = selected_peak.index;
    float max_val = selected_peak.amplitude;
    float actual_depth_cm = selected_peak.depth_cm;

    printf("\n  Selected peak: sample %d, depth %.2f cm, amplitude %.2f, SNR %.1f\n",
        max_idx, actual_depth_cm, max_val, selected_peak.snr);

    // 4. Проверяем глубину
    printf("\n[4] DEPTH CALCULATION\n");
    double focus_depth_cm = params.focus_depth_sample * 0.5 * (c_cm / fs);
    printf("  Focus depth: %d samples -> %.2f cm (%.1f mm)\n",
        params.focus_depth_sample, focus_depth_cm, focus_depth_cm * 10);
    printf("  Actual peak depth: %.2f cm (%.1f mm)\n",
        actual_depth_cm, actual_depth_cm * 10);

    // 5. Проверяем время прихода для выбранного пика
    printf("\n[5] TIME OF FLIGHT ANALYSIS\n");
    double tx_to_point = sqrt(tx_pos * tx_pos + actual_depth_cm * actual_depth_cm) / c_cm;
    double rx_to_point = sqrt(rx_pos * rx_pos + actual_depth_cm * actual_depth_cm) / c_cm;
    double total_time = tx_to_point + rx_to_point;
    double sample_idx = total_time * fs;

    printf("  Actual peak at sample %d\n", max_idx);
    printf("  Theoretical sample for this depth: %.1f\n", sample_idx);
    printf("  Difference: %.1f samples (%.2f mm)\n",
        sample_idx - max_idx,
        (sample_idx - max_idx) * 0.5 * (c_cm / fs) * 10);

    // 6. Вычисляем требуемую скорость для совпадения
    printf("\n[6] SPEED CALIBRATION\n");
    if (max_idx > 0) {
        // Вычисляем, какая скорость нужна, чтобы совместить пики
        double depth_at_peak = max_idx * 0.5 * (c_cm / fs);
        double tx_path_at_peak = sqrt(tx_pos * tx_pos + depth_at_peak * depth_at_peak);
        double rx_path_at_peak = sqrt(rx_pos * rx_pos + depth_at_peak * depth_at_peak);
        double time_at_peak = (tx_path_at_peak + rx_path_at_peak) / c_cm;

        // Теоретическая скорость для совпадения
        double required_speed = (tx_path_at_peak + rx_path_at_peak) / (max_idx / fs);
        printf("  Required speed to match peak: %.0f m/s\n", required_speed);
        printf("  Current speed: %.0f m/s\n", c);
        printf("  Difference: %.0f m/s (%.1f%%)\n",
            required_speed - c, (required_speed - c) / c * 100);

        // Рекомендация
        if (fabs(required_speed - c) > 50) {
            printf("\n  RECOMMENDATION: Update speed of sound to %.0f m/s\n", required_speed);
            if (YesOrNo("Update speed to the recommended value?",
                MakeGUIValue(true, saved_default_value))) {
                // Здесь нужно обновить params
                // Это будет сделано через возврат значения
            }
        }
    }

    // 7. Проверка на наличие артефактов ближнего поля
    printf("\n[7] NEAR FIELD CHECK\n");
    if (actual_depth_cm < 1.0) {
        printf("  WARNING: Selected peak is in the near field (< 1 cm)\n");
        printf("  This may cause inaccurate calibration due to:\n");
        printf("    - High impedance mismatch at skin-transducer interface\n");
        printf("    - Near field artifacts\n");
        printf("    - Non-uniform wavefront\n");
        printf("  RECOMMENDATION: Select a deeper peak (e.g., at thyroid or vessel wall)\n");
    }
    else if (actual_depth_cm < 2.0) {
        printf("  NOTE: Peak is in the transition zone (1-2 cm)\n");
        printf("  Calibration may be less accurate than deeper peaks.\n");
    }
    else {
        printf("  GOOD: Peak is in the far field (> 2 cm)\n");
        printf("  This is suitable for speed of sound calibration.\n");
    }

    printf("\n========================================\n");
    printf("=== DIAGNOSTICS SUMMARY ===\n");
    printf("  Selected peak depth: %.2f cm\n", actual_depth_cm);
    printf("  Recommended speed: %.0f m/s\n",
        max_idx > 0 ? (sqrt(tx_pos * tx_pos + actual_depth_cm * actual_depth_cm) +
            sqrt(rx_pos * rx_pos + actual_depth_cm * actual_depth_cm)) / (max_idx / fs) : c);
    printf("  Current parameters: speed=%.0f m/s, pitch=%.3f mm\n", c, params.pitch_mm);
    printf("========================================\n");
    fflush(stdout);
}

// ============================================================
// ДИАГНОСТИКА ПЕРЕСТАНОВКИ ЭЛЕМЕНТОВ
// ============================================================

void SyntheticApertureFocuserDebug::DiagnoseElementOrder(
    RawSFDataSource& source,
    const DebugFocusParams& params)
{
    printf("\n========================================\n");
    printf("=== ELEMENT ORDER DIAGNOSTICS ===\n");
    printf("========================================\n");
    printf("Your parameters suggest element reordering issue:\n");
    printf("  Effective pitch: %.3f mm (should be ~0.245 mm for LA533)\n", params.pitch_mm);
    printf("  Effective speed: %.0f m/s (should be ~1540 m/s for tissue)\n", params.speed_of_sound);
    printf("\nThis typically happens when elements are not in correct order.\n");
    fflush(stdout);

    // Получаем данные для двух соседних элементов
    int tx = params.tx_element;
    int rx1 = params.rx_element;
    int rx2 = std::min(rx1 + 1, (int)source.n_rx_elements - 1);

    printf("\n=== COMPARING ADJACENT ELEMENTS ===\n");
    printf("  TX: %d, RX1: %d, RX2: %d\n", tx, rx1, rx2);

    auto row1 = source.GetRow(tx, rx1);
    auto row2 = source.GetRow(tx, rx2);

    if (!row1 || !row2) {
        printf("Cannot get rows for comparison\n");
        return;
    }

    size_t n_samples = n_samples_debug;
    n_samples = std::min(n_samples, (size_t)1000);

    // Вычисляем кросс-корреляцию между соседними элементами
    // Это покажет, насколько сигналы похожи
    std::vector<double> correlation(n_samples);
    double max_corr = 0;
    int max_shift = 0;

    for (int shift = -50; shift <= 50; ++shift) {
        double corr = 0;
        double norm1 = 0, norm2 = 0;
        for (size_t i = 0; i < n_samples - abs(shift); ++i) {
            int idx1 = i;
            int idx2 = i + shift;
            if (idx2 < 0 || idx2 >= (int)n_samples) continue;

            float v1 = cabs(row1->in(idx1));
            float v2 = cabs(row2->in(idx2));
            corr += v1 * v2;
            norm1 += v1 * v1;
            norm2 += v2 * v2;
        }
        if (norm1 > 0 && norm2 > 0) {
            corr = corr / sqrt(norm1 * norm2);
            if (corr > max_corr) {
                max_corr = corr;
                max_shift = shift;
            }
        }
    }

    printf("  Cross-correlation between adjacent elements:\n");
    printf("    Max correlation: %.3f\n", max_corr);
    printf("    Time shift: %d samples\n", max_shift);

    if (max_corr > 0.8) {
        printf("    GOOD: Adjacent elements are highly correlated\n");
        printf("    (This means elements are in correct order)\n");
    }
    else if (max_corr > 0.5) {
        printf("    MODERATE: Some correlation but low\n");
        printf("    (Possible element reordering or noise)\n");
    }
    else {
        printf("    POOR: Very low correlation\n");
        printf("    (Elements may be in wrong order!)\n");
    }

    // Проверяем, что элементы действительно соседние
    printf("\n=== CHECKING PHYSICAL PROXIMITY ===\n");
    double pitch_cm = params.pitch_mm / 10.0;
    double aperture_center = 0.5 * source.n_rx_elements * pitch_cm;
    double pos1 = (rx1 + 0.5) * pitch_cm - aperture_center;
    double pos2 = (rx2 + 0.5) * pitch_cm - aperture_center;
    double distance = pos2 - pos1;

    printf("  Element positions:\n");
    printf("    RX1: %.3f mm\n", pos1 * 10);
    printf("    RX2: %.3f mm\n", pos2 * 10);
    printf("    Distance: %.3f mm\n", distance * 10);

    if (distance < 0) {
        printf("  WARNING: Elements are in REVERSE order!\n");
        printf("  (pos2 < pos1 means RX2 is to the left of RX1)\n");
    }

    // Проверяем, есть ли в данных структура, соответствующая физическому датчику
    printf("\n=== ANALYZING WAVEFRONT CURVATURE ===\n");
    printf("  With correct element order, wavefront should show CURVATURE\n");
    printf("  With incorrect order, wavefront may appear FLAT or DISTORTED\n");

    // Предлагаем попробовать разные перестановки
    printf("\n=== SUGGESTED ACTIONS ===\n");
    printf("  1. Try the correct pitch (0.245 mm) with current speed\n");
    printf("  2. Try the correct speed (1540 m/s) with current pitch\n");
    printf("  3. Try both correct pitch AND speed\n");
    printf("  4. Check if elements need to be reversed (mirrored)\n");
    printf("  5. Run full element order analysis\n");

    int action = GetSigned("Choose action",
        MakeGUIValue(1, saved_default_value), 1, 5);

    switch (action) {
    case 1: {
        double new_pitch = 0.245;
        printf("\n  Using correct pitch: %.3f mm\n", new_pitch);
        printf("  Current pitch: %.3f mm\n", params.pitch_mm);
        printf("  Ratio: %.2f\n", new_pitch / params.pitch_mm);
        // Пересчитываем ожидаемую скорость
        double new_speed = params.speed_of_sound * (new_pitch / params.pitch_mm);
        printf("  Suggested speed: %.0f m/s\n", new_speed);
        break;
    }
    case 2: {
        double new_speed = 1540.0;
        printf("\n  Using correct speed: %.0f m/s\n", new_speed);
        printf("  Current speed: %.0f m/s\n", params.speed_of_sound);
        printf("  Ratio: %.2f\n", new_speed / params.speed_of_sound);
        double new_pitch = params.pitch_mm * (new_speed / params.speed_of_sound);
        printf("  Suggested pitch: %.3f mm\n", new_pitch);
        break;
    }
    case 3: {
        printf("\n  Using correct parameters:\n");
        printf("    Speed: 1540 m/s\n");
        printf("    Pitch: 0.245 mm\n");
        break;
    }
    case 4: {
        printf("\n  Checking if elements are reversed...\n");
        // Меняем порядок элементов
        printf("  Try using RX elements in reverse order in beamforming\n");
        break;
    }
    case 5: {
        printf("\n  Running full element order analysis...\n");
        // Анализируем все элементы
        std::vector<double> element_positions;
        for (int rx = 0; rx < source.n_rx_elements; ++rx) {
            auto row = source.GetRow(tx, rx);
            if (row) {
                // Находим пик в каждом элементе
                float max_val = 0;
                int max_idx = 0;
                for (size_t i = 0; i < std::min(n_samples_debug, (size_t)2000); ++i) {
                    float amp = cabs(row->in(i));
                    if (amp > max_val) {
                        max_val = amp;
                        max_idx = (int)i;
                    }
                }
                element_positions.push_back(max_idx);
            }
        }

        // Анализируем последовательность пиков
        bool increasing = true;
        bool decreasing = true;
        for (size_t i = 1; i < element_positions.size(); ++i) {
            if (element_positions[i] <= element_positions[i - 1]) increasing = false;
            if (element_positions[i] >= element_positions[i - 1]) decreasing = false;
        }

        printf("  Element peak analysis:\n");
        if (increasing) {
            printf("    Peaks INCREASE across elements (normal)\n");
        }
        else if (decreasing) {
            printf("    Peaks DECREASE across elements (REVERSED!)\n");
        }
        else {
            printf("    No clear pattern (possible reordering issue)\n");
        }
        break;
    }
    }

    printf("\n========================================\n");
    fflush(stdout);
}

void SyntheticApertureFocuserDebug::InteractiveDebug(RawSFDataSource& source)
{
    printf("\n========================================\n");
    printf("=== WAVEFRONT DEBUG MODE ===\n");
    printf("========================================\n");
    printf("This mode helps you visualize and debug beamforming parameters\n");
    printf("You can adjust speed of sound, pitch, and element positions\n");
    printf("to match the wavefront with actual data.\n\n");
    fflush(stdout);

    DebugFocusParams params;

    // --- ЗАПРАШИВАЕМ ВСЕ ПАРАМЕТРЫ У ПОЛЬЗОВАТЕЛЯ ---

    printf("\n=== BASIC PARAMETERS ===\n");

    // 1. Скорость звука
    printf("\n[1] Speed of Sound\n");
    printf("    Current source value: %.0f m/s\n", source.sound_speed.m_sec());
    printf("    Typical values:\n");
    printf("      - Fat:        1450 m/s\n");
    printf("      - Muscle:     1540-1580 m/s\n");
    printf("      - Blood:      1570 m/s\n");
    printf("      - Liver:      1550-1590 m/s\n");
    printf("      - Thyroid:    1550-1570 m/s\n");
    params.speed_of_sound = GetFloating("Enter speed of sound (m/s)", MakeGUIValue(source.sound_speed.m_sec(), saved_default_value), 1, 7000);

    // 2. Шаг решетки
    printf("\n[2] Array Pitch\n");
    printf("    Current source value: %.3f mm\n", source.array_pitch.mm());
    printf("    Typical values for linear arrays:\n");
    printf("      - High frequency:  0.1-0.2 mm\n");
    printf("      - Standard:        0.245-0.3 mm\n");
    printf("      - Low frequency:   0.3-0.5 mm\n");
    params.pitch_mm = GetFloating("Enter array pitch (mm)", MakeGUIValue(source.array_pitch.mm(), saved_default_value), 0.05, 1.0);

    // 3. Передающий элемент
    int max_element = source.n_tx_elements - 1;
    printf("\n[3] Transmit Element (TX)\n");
    printf("    Available elements: 0..%d\n", max_element);
    printf("    Suggested: center element (~%d)\n", max_element / 2);
    params.tx_element = GetSigned("Enter TX element index", MakeGUIValue(max_element / 2, saved_default_value), 0, max_element);

    // 4. Приемный элемент
    printf("\n[4] Receive Element (RX)\n");
    printf("    Available elements: 0..%d\n", max_element);
    printf("    Suggested: same as TX for pulse-echo (%d)\n", params.tx_element);
    params.rx_element = GetSigned("Enter RX element index", MakeGUIValue(params.tx_element, saved_default_value), 0, max_element);

    // 5. Глубина фокуса
    int max_sample = source.samples_per_element - 1;
    printf("\n[5] Focus Depth\n");
    printf("    Available samples: 0..%d\n", max_sample);
    printf("    Typical: 300-1000 for superficial structures\n");
    printf("    Current max sample: %d\n", max_sample);
    params.focus_depth_sample = GetSigned("Enter focus depth (sample index)", MakeGUIValue(500, saved_default_value), 0, max_sample);

    // 6. Номер кадра (для SA данных)
    int max_frame = source.n_tx_elements - 1;
    printf("\n[6] Frame Index (for SA data)\n");
    printf("    Available frames: 0..%d\n", max_frame);
    printf("    Each frame corresponds to one TX element\n");
    printf("    Suggested: 0 for first TX element\n");
    params.frame_index = GetSigned("Enter frame index", MakeGUIValue(0, saved_default_value), 0, max_frame);

    // --- ПОКАЗЫВАЕМ ИТОГОВЫЕ ПАРАМЕТРЫ ---
    printf("\n========================================\n");
    printf("=== CONFIRM PARAMETERS ===\n");
    printf("  Speed of sound:      %.0f m/s\n", params.speed_of_sound);
    printf("  Array pitch:         %.3f mm\n", params.pitch_mm);
    printf("  TX element:          %d\n", params.tx_element);
    printf("  RX element:          %d\n", params.rx_element);
    printf("  Focus depth:         %d samples (%.2f cm)\n",
        params.focus_depth_sample,
        params.focus_depth_sample * 0.5 * (params.speed_of_sound * 100 / source.raw_signal_sample_rate.Hz()));
    printf("  Frame index:         %d\n", params.frame_index);
    printf("========================================\n");

    // Проверяем, что параметры разумные
    if (params.tx_element == params.rx_element) {
        printf("\n[INFO] TX and RX are the same element (pulse-echo mode)\n");
        printf("       This is good for wavefront analysis.\n");
    }
    else {
        printf("\n[INFO] TX and RX are different elements (pitch-catch mode)\n");
        printf("       The wavefront will show propagation between elements.\n");
    }

    // Вычисляем и показываем ожидаемую глубину
    double fs = source.raw_signal_sample_rate.Hz();
    double c_cm = params.speed_of_sound * 100;
    double expected_depth_cm = params.focus_depth_sample * 0.5 * (c_cm / fs);
    printf("       Expected depth: %.2f cm (%.1f mm)\n",
        expected_depth_cm, expected_depth_cm * 10);

    // Спрашиваем подтверждение
    bool confirmed = YesOrNo("Proceed with these parameters?",
        MakeGUIValue(true, saved_default_value));
    if (!confirmed) {
        printf("Cancelled. You can re-enter parameters.\n");
        return;
    }

    // --- ГЛАВНЫЙ ЦИКЛ ОТЛАДКИ ---
    bool exit_debug = false;
    while (!exit_debug) {
        printf("\n========================================\n");
        printf("=== DEBUG MENU ===\n");
        printf("Current parameters:\n");
        printf("  TX=%d, RX=%d, Speed=%.0f m/s, Pitch=%.3f mm, Depth=%d samples\n",
            params.tx_element, params.rx_element,
            params.speed_of_sound, params.pitch_mm,
            params.focus_depth_sample);
        printf("\nOptions:\n");
        printf("  1. Display A-line with wavefront (GraphSet) + quick adjust\n");
        printf("  2. Display 2D wavefront map\n");
        printf("  3. Find optimal speed of sound (auto-calibration)\n");
        printf("  4. Visualize wavefront on SA data (2D image)\n");
        printf("  5. Change parameters (detailed)\n");
        printf("  6. Export current parameters to console\n");
        printf("  7. RUN FULL DIAGNOSTICS (recommended if wavefront doesn't match)\n");
        printf("  8. DIAGNOSE ELEMENT ORDER (check if elements are in correct order)\n");  // НОВАЯ ОПЦИЯ
        printf("  0. Exit debug mode\n");
        printf("========================================\n");

        int choice = GetSigned("Choose option", MakeGUIValue(0, saved_default_value), 0, 8);

        switch (choice) {
        case 0:
            exit_debug = true;
            printf("Exiting debug mode\n");
            break;

        case 1: {
            // Отображаем A-line с волновым фронтом
            DisplayAlineWithWavefront(source, params);

            // После отображения спрашиваем, хочет ли пользователь корректировать параметры
            printf("\n=== QUICK ADJUST ===\n");
            printf("Did the wavefront match the actual signal?\n");
            printf("  - If peak matches:    continue to next step\n");
            printf("  - If peak is shifted: adjust parameters and see updated graph\n");
            printf("  - Run diagnostics to find the problem\n");

            int diag_choice = GetSigned("Choose action: (1=Adjust, 2=Diagnose, 0=Skip)",
                MakeGUIValue(0, saved_default_value), 0, 2);

            if (diag_choice == 2) {
                // Запускаем диагностику
                DiagnoseParameters(source, params);

                // Предлагаем автоматическое исправление
                if (YesOrNo("Try to automatically fix common issues?",
                    MakeGUIValue(true, saved_default_value))) {

                    // Проверяем частоту дискретизации
                    double fs_diag = source.raw_signal_sample_rate.Hz();
                    auto row = source.GetRow(params.tx_element, params.rx_element);
                    if (row) {
                        // Находим реальный пик
                        float max_val = 0;
                        int max_idx = 0;
                        size_t n_samples = n_samples_debug;
                        for (size_t i = 0; i < n_samples; ++i) {
                            float amp = cabs(row->in(i));
                            if (amp > max_val) {
                                max_val = amp;
                                max_idx = (int)i;
                            }
                        }

                        if (max_val > 0) {
                            // Пробуем разные частоты дискретизации
                            double test_freqs[] = { 78.125e6, 39.0625e6, 19.53125e6, 9.765625e6, 4.8828125e6 };
                            printf("\nTrying different sampling frequencies...\n");
                            printf("  Current focus depth sample: %d\n", params.focus_depth_sample);
                            printf("  Actual peak at sample: %d\n", max_idx);
                            printf("\n  If fs changes, theoretical peak position changes:\n");

                            for (double test_fs : test_freqs) {
                                double depth_cm = params.focus_depth_sample * 0.5 * (params.speed_of_sound * 100 / test_fs);
                                double actual_depth_cm = max_idx * 0.5 * (params.speed_of_sound * 100 / test_fs);
                                printf("    fs=%.3f MHz: theory=%.2f cm, actual=%.2f cm, diff=%.2f mm\n",
                                    test_fs / 1e6, depth_cm, actual_depth_cm, (depth_cm - actual_depth_cm) * 10);
                            }

                            // Предлагаем выбрать правильную частоту
                            printf("\n  Which sampling frequency matches your data?\n");
                            printf("    ULA-OP typically uses 78.125 MHz (or 39.0625 MHz with decimation)\n");
                            printf("    Check your .uop file for SysDownsampling value\n");

                            if (YesOrNo("Use 39.0625 MHz (half of 78.125 MHz)?",
                                MakeGUIValue(false, saved_default_value))) {
                                // Здесь можно добавить логику изменения fs
                                printf("  To change fs, modify source.raw_signal_sample_rate\n");
                                printf("  Current fs: %.3f MHz\n", source.raw_signal_sample_rate.Hz() / 1e6);
                            }
                        }
                    }
                }

                // После диагностики предлагаем корректировку
                bool adjust_after_diag = YesOrNo("Adjust parameters now?",
                    MakeGUIValue(false, saved_default_value));

                if (adjust_after_diag) {
                    diag_choice = 1; // Переходим к корректировке
                }
                else {
                    break;
                }
            }

            if (diag_choice == 1) {
                bool adjust = true;
                while (adjust) {
                    printf("\nWhat do you want to adjust?\n");
                    printf("  1. Speed of sound (if peak shifted left/right)\n");
                    printf("  2. Pitch (if wavefront curvature doesn't match)\n");
                    printf("  3. Focus depth (if peak is at wrong depth)\n");
                    printf("  4. TX/RX elements (if signal is weak)\n");
                    printf("  5. Sampling frequency (if peak doesn't move with speed changes)\n");
                    printf("  0. Done adjusting\n");

                    int adj_choice = GetSigned("Choose adjustment",
                        MakeGUIValue(0, saved_default_value), 0, 5);

                    if (adj_choice == 0) {
                        adjust = false;
                        break;
                    }

                    switch (adj_choice) {
                    case 1: {
                        // Корректировка скорости звука
                        printf("\nCurrent speed: %.0f m/s\n", params.speed_of_sound);
                        printf("  - If actual peak is RIGHT of theoretical -> speed is too LOW (increase)\n");
                        printf("  - If actual peak is LEFT of theoretical  -> speed is too HIGH (decrease)\n");

                        double new_speed = GetFloating(
                            "New speed of sound (m/s)",
                            MakeGUIValue(params.speed_of_sound, saved_default_value),
                            1, 7000);

                        if (new_speed != params.speed_of_sound) {
                            params.speed_of_sound = new_speed;
                            printf("Speed updated to %.0f m/s\n", params.speed_of_sound);
                            // Перерисовываем график с новыми параметрами
                            DisplayAlineWithWavefront(source, params);
                        }
                        break;
                    }

                    case 2: {
                        // Корректировка шага решетки
                        printf("\nCurrent pitch: %.3f mm\n", params.pitch_mm);
                        printf("  - Increase pitch if wavefront curvature is too narrow\n");
                        printf("  - Decrease pitch if wavefront curvature is too wide\n");

                        double new_pitch = GetFloating(
                            "New pitch (mm)",
                            MakeGUIValue(params.pitch_mm, saved_default_value),
                            0.05, 1.0);

                        if (new_pitch != params.pitch_mm) {
                            params.pitch_mm = new_pitch;
                            printf("Pitch updated to %.3f mm\n", params.pitch_mm);
                            DisplayAlineWithWavefront(source, params);
                        }
                        break;
                    }

                    case 3: {
                        // Корректировка глубины фокуса
                        printf("\nCurrent focus depth: %d samples\n", params.focus_depth_sample);

                        int new_depth = GetSigned(
                            "New focus depth (samples)",
                            MakeGUIValue(params.focus_depth_sample, saved_default_value),
                            0, source.samples_per_element - 1);

                        if (new_depth != params.focus_depth_sample) {
                            params.focus_depth_sample = new_depth;
                            printf("Depth updated to %d samples\n", params.focus_depth_sample);
                            DisplayAlineWithWavefront(source, params);
                        }
                        break;
                    }

                    case 4: {
                        // Корректировка TX/RX элементов
                        printf("\nCurrent TX=%d, RX=%d\n", params.tx_element, params.rx_element);

                        int new_tx = GetSigned(
                            "New TX element",
                            MakeGUIValue(params.tx_element, saved_default_value),
                            0, n_elements_debug - 1);

                        int new_rx = GetSigned(
                            "New RX element",
                            MakeGUIValue(params.rx_element, saved_default_value),
                            0, n_elements_debug - 1);

                        if (new_tx != params.tx_element || new_rx != params.rx_element) {
                            params.tx_element = new_tx;
                            params.rx_element = new_rx;
                            printf("TX=%d, RX=%d updated\n", params.tx_element, params.rx_element);
                            DisplayAlineWithWavefront(source, params);
                        }
                        break;
                    }

                    case 5: {
                        // Корректировка частоты дискретизации
                        printf("\nCurrent sampling frequency: %.3f MHz\n", source.raw_signal_sample_rate.Hz() / 1e6);
                        printf("  ULA-OP typical values: 78.125 MHz, 39.0625 MHz, 19.53125 MHz\n");
                        printf("  Check SysDownsampling in .uop file\n");
                        printf("  SysDownsampling=1 -> 78.125 MHz\n");
                        printf("  SysDownsampling=2 -> 39.0625 MHz\n");
                        printf("  SysDownsampling=4 -> 19.53125 MHz\n");

                        double new_fs_mhz = GetFloating(
                            "New sampling frequency (MHz)",
                            MakeGUIValue(source.raw_signal_sample_rate.Hz() / 1e6, saved_default_value),
                            1.0, 100.0);

                        double new_fs = new_fs_mhz * 1e6;
                        // Примечание: в реальном коде нужно иметь возможность изменить fs в источнике
                        printf("  To change fs, modify source.raw_signal_sample_rate\n");
                        printf("  New fs: %.3f MHz\n", new_fs / 1e6);
                        printf("  NOTE: Changing fs requires reloading data or modifying source object\n");
                        break;
                    }

                    default:
                        printf("Invalid choice\n");
                    }
                }
            }

            break;
        }

        case 2:
            Display2DWithWavefront(source, params);
            break;

        case 3: {
            printf("\n=== AUTO-CALIBRATION ===\n");
            printf("This will find optimal speed of sound for current TX/RX pair\n");
            double speed_min = GetFloating("Speed min (m/s)",
                MakeGUIValue(1400.0, saved_default_value), 1, 7000);
            double speed_max = GetFloating("Speed max (m/s)",
                MakeGUIValue(1600.0, saved_default_value), 1, 7000);
            int steps = GetSigned("Number of steps",
                MakeGUIValue(20, saved_default_value), 5, 100);

            params.speed_of_sound = FindOptimalSpeed(
                source,
                params.tx_element,
                params.rx_element,
                params.focus_depth_sample,
                speed_min, speed_max, steps);
            break;
        }

        case 4:
            VisualizeWavefrontOnSAData(
                source,
                params.frame_index,
                params.focus_depth_sample,
                params.speed_of_sound);
            break;

        case 5: {
            printf("\n=== CHANGE PARAMETERS ===\n");
            // Можно изменить любой параметр
            int param_choice = GetSigned(
                "Which parameter to change? (1=Speed, 2=Pitch, 3=TX, 4=RX, 5=Depth, 6=Frame, 7=Sampling Frequency)",
                MakeGUIValue(0, saved_default_value), 1, 7);

            switch (param_choice) {
            case 1:
                params.speed_of_sound = GetFloating(
                    "Speed of sound (m/s)",
                    MakeGUIValue(params.speed_of_sound, saved_default_value),
                    1, 7000);
                break;
            case 2:
                params.pitch_mm = GetFloating(
                    "Array pitch (mm)",
                    MakeGUIValue(params.pitch_mm, saved_default_value),
                    0.05, 1.0);
                break;
            case 3:
                params.tx_element = GetSigned(
                    "TX element",
                    MakeGUIValue(params.tx_element, saved_default_value),
                    0, n_elements_debug - 1);
                break;
            case 4:
                params.rx_element = GetSigned(
                    "RX element",
                    MakeGUIValue(params.rx_element, saved_default_value),
                    0, n_elements_debug - 1);
                break;
            case 5:
                params.focus_depth_sample = GetSigned(
                    "Focus depth (samples)",
                    MakeGUIValue(params.focus_depth_sample, saved_default_value),
                    0, source.samples_per_element - 1);
                break;
            case 6:
                params.frame_index = GetSigned(
                    "Frame index",
                    MakeGUIValue(params.frame_index, saved_default_value),
                    0, source.n_tx_elements - 1);
                break;
            case 7: {
                double current_fs_mhz = source.raw_signal_sample_rate.Hz() / 1e6;
                double new_fs_mhz = GetFloating(
                    "Sampling frequency (MHz)",
                    MakeGUIValue(current_fs_mhz, saved_default_value),
                    1.0, 100.0);
                printf("  New fs: %.3f MHz\n", new_fs_mhz);
                printf("  NOTE: Changing fs requires modifying source object\n");
                break;
            }
            }
            break;
        }

        case 6: {
            printf("\n=== EXPORT PARAMETERS ===\n");
            printf("Copy these values for use in beamforming:\n");
            printf("  sound_speed = %.0f;  // m/s\n", params.speed_of_sound);
            printf("  pitch = %.4f;        // m (%.3f mm)\n",
                params.pitch_mm / 1000.0, params.pitch_mm);
            printf("  tx_element = %d;\n", params.tx_element);
            printf("  rx_element = %d;\n", params.rx_element);
            printf("  focus_depth = %d;    // samples\n", params.focus_depth_sample);
            printf("  focus_depth_m = %.4f; // m\n",
                params.focus_depth_sample * 0.5 * (params.speed_of_sound / fs));
            printf("  fs = %.3f;           // MHz\n", source.raw_signal_sample_rate.Hz() / 1e6);
            fflush(stdout);
            break;
        }

        case 7: {
            // RUN FULL DIAGNOSTICS
            DiagnoseParameters(source, params);
            break;
        }
        case 8: {
            DiagnoseElementOrder(source, params);
            break;
        }

        default:
            printf("Invalid choice. Please select 0-7.\n");
        }
    }

    printf("\n=== DEBUG MODE EXITED ===\n");
    printf("Final parameters:\n");
    printf("  Speed of sound: %.0f m/s\n", params.speed_of_sound);
    printf("  Pitch: %.3f mm\n", params.pitch_mm);
    printf("  TX: %d, RX: %d\n", params.tx_element, params.rx_element);
    printf("  Focus depth: %d samples\n", params.focus_depth_sample);
    printf("  Frame: %d\n", params.frame_index);
    printf("  Sampling frequency: %.3f MHz\n", source.raw_signal_sample_rate.Hz() / 1e6);
    printf("\nYou can use these values in your beamforming code.\n");
    fflush(stdout);
}

XRAD_END