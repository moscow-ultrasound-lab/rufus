#include "pre.h"

#include <RFDataImport/S500_CFMFrameSet.h>
#include <RFDataImport/S500_CFMRawDataDisplay.h>

#include <DopplerBasics/CFM/WallFilters/WallFiltersInteractive.h>

// тесты универсальных методов доплерографии
#include "CFMModesDetailedTest.h"

// локальные тесты тонкостей алгоритмов, довольно беспорядочные
#include "SetOfPulses.h"
#include "DummyHypothesis.h"
#include "SimulateSignal.h"
#include <BBasics\BModeDetailedTest.h>
#include <OASBUDFrameSet.h>





XRAD_USING;

void RawDataProcessingMenu(S500_CFMFrameSet &frames)
{
	try
	{
		enum
		{
			e_display,
			e_analyze, 
			e_exit
		};
		while (true)
		{
			auto answer = Decide(L"Choose option", 
				{ 
				MakeButton(L"Display", e_display),
				MakeButton("End with this file", e_exit)
			});

			switch (answer)
			{
			case e_display:
			{
				DisplayMathFunction3D(frames.b_frames, "B-frames", frames.sco_b);
			}
				break;
				break;
			default:
				throw canceled_operation("End with this file");
			}
		}
	}
	catch (canceled_operation &) {}
	catch (quit_application &ex) { throw ex; }
	catch(...) { Error(GetExceptionString()); }
}


S500_CFMParamFileData	GetS500FileParams()
{
	wstring	filename = GetFileNameRead(L"Choose data files", saved_default_value, L"*.par");
	S500_CFMParamFileData	params;
	params.Init(filename);
	return params;
}

S500_CFMFrameSet GetS500Frames()
{
	S500_CFMParamFileData	params = GetS500FileParams();
	size_t	start_frame = 0;
	size_t	end_frame = params.NumOfFrames - 1;
	end_frame = GetUnsigned("How many frames to read?", MakeGUIValue(params.NumOfFrames, saved_default_value), 1, params.NumOfFrames) - 1;
	S500_CFMFrameSet	frames(params, start_frame, end_frame);
	bool unsweep = !CapsLock() ? true : YesOrNo("Unsweep data?", true);
	frames.ReadAllFrames(unsweep);
	return frames;
}

void ProcessOASBUDFrames()
{
    XRAD_USING

        try
    {
        OASBUDFrameSet frames;
        wstring filename = GetFileNameRead(L"Select OASBUD .mat file", saved_default_value, L"*.mat");
        if (!frames.ReadMatFile(wstring_to_string(filename, e_encode_literals)))
        {
            Error("Failed to load OASBUD file");
            return;
        }
        size_t num_lesions = frames.GetNumLesions();
        while (true)
        {
            size_t mode = GetButtonDecision("OASBUD Viewer",
                {
                    MakeButton(L"Display single lesion", 0),
                    MakeButton(L"Show metadata only", 2),
                    MakeButton(L"Exit to main menu", 4)
                });

            try
            {
                if (mode == 0)                       // Отображение одного поражения с меню
                {
                    size_t idx = GetUnsigned("Lesion index (0-" + std::to_string(num_lesions - 1) + ")", 0, 0, num_lesions - 1);
                    frames.DisplayLesionWithMenu(idx);
                }
                else if (mode == 2)                    // Показ метаданных
                {
                    frames.DisplayMetadata();
                }
                else if (mode == 4)                       // Выход в главное меню
                {
                    printf("\nReturning to main menu...\n");
                    break;
                }
            }
            catch (canceled_operation&)
            {
                printf("\nOperation cancelled. Returning to OASBUD menu...\n");
                continue;
            }
            catch (quit_application&) {
                throw;
            }
            catch (exception& ex)
            {
                Error(ssprintf("Error: %s", ex.what()));
                continue;
            }
        }
    }
    catch (canceled_operation&)
    {
        printf("\nOASBUD loading cancelled.\n");
    }
    catch (quit_application&)
    {
        throw;
    }
    catch (exception& ex)
    {
        Error(ssprintf("Error: %s", ex.what()));
    }
}

int xrad::xrad_main(int, char** const)
{
    XRAD_USING

        try
    {
        size_t answer(0);
        while (true)
        {
            answer = Decide("Choose source", { "Sonomed-500", "OASBUD", "Exit" });
            switch (answer)
            {
            case 0: // Sonomed-500
            {
                try
                {
                    S500_CFMFrameSet frames = GetS500Frames();
                    RawDataProcessingMenu(frames);
                }
                catch (canceled_operation) {}
                catch (...) { Error(GetExceptionStringOrRethrow()); }
            }
            break;

            case 1: // OASBUD
            {
                ProcessOASBUDFrames();
            }
            break;

            default:
                throw canceled_operation("");
            }
        }
    }
    catch (canceled_operation&) {}
    catch (quit_application&) {}
    catch (...)
    {
        Error(GetExceptionString());
        return 1;
    }
    return 0;
}
