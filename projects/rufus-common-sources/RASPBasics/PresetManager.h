#ifndef __preset_manager_h
#define __preset_manager_h


#include <DataArray.h>
#include "TextureProcessorSettings.h"

XRAD_BEGIN

class   RASP_PresetManager: public DataArray<TextureProcessorSettings>
	{
        float   Interpolate(float t1, float t2, float v) const;
        TextureProcessorSettings Interpolate(const TextureProcessorSettings &t1, const TextureProcessorSettings &t2, float v) const;
public:
        RASP_PresetManager();
        TextureProcessorSettings GetPreset(float);
        };

XRAD_END

#endif //__preset_manager_h




