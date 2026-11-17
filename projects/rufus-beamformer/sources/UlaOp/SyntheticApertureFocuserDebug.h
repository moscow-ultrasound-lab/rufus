// SyntheticApertureFocuserDebug.h
#ifndef __SyntheticApertureFocuserDebug_h__
#define __SyntheticApertureFocuserDebug_h__

#include "Pre.h"
#include "SignalProcessing/RawSFDataSource.h"
//#include <XRADBasic/ComplexFunction.h>

XRAD_BEGIN




struct DebugFocusParams {
    int tx_element = 0;           // Номер передающего элемента
    int rx_element = 0;           // Номер приемного элемента
    int focus_depth_sample = 500; // Глубина фокуса в семплах
    int frame_index = 0;          // Номер кадра
    double speed_of_sound = 1540.0; // Скорость звука в м/с
    double pitch_mm = 0.245;      // Шаг решетки в мм
    double time_window_us = 5.0;  // Временное окно для отображения
};

class SyntheticApertureFocuserDebug
{
public:
    // Отображение одного A-line с теоретической кривой задержек
    static void DisplayAlineWithWavefront(
        RawSFDataSource& source,
        const DebugFocusParams& params);
    
    // Отображение 2D карты задержек (B-mode с наложением)
    static void Display2DWithWavefront(
        RawSFDataSource& source,
        const DebugFocusParams& params);
    
    // Поиск оптимальной скорости звука по корреляции
    static double FindOptimalSpeed(
        RawSFDataSource& source,
        int tx_element,
        int rx_element,
        int focus_depth_sample,
        double speed_min = 1400,
        double speed_max = 1600,
        int steps = 20);

    static void SyntheticApertureFocuserDebug::DiagnoseParameters(
        RawSFDataSource& source,
        const DebugFocusParams& params);
    
    static void SyntheticApertureFocuserDebug::DiagnoseElementOrder(
        RawSFDataSource& source,
        const DebugFocusParams& params);

    // Визуализация профиля волны на кадре SA данных
    static void VisualizeWavefrontOnSAData(
        RawSFDataSource& source,
        int frame_index,
        int focus_depth_sample,
        double speed_of_sound);
    

   const static  size_t n_samples_debug = 8192;
   const static size_t n_elements_debug = 256;

    // Интерактивный режим отладки
    static void InteractiveDebug(RawSFDataSource& source);
};

XRAD_END

#endif