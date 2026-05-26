#ifndef roi_vocabulary_h__
#define roi_vocabulary_h__

/*!
	\file
	\date 2019/11/22 15:09
	\author kulberg

	\brief  
*/


XRAD_BEGIN

using roi_type_vocabulary_t = map<int, pair<wstring, ColorSampleUI8>>;


roi_type_vocabulary_t	mmg_vocabulary();
roi_type_vocabulary_t	lung_vocabulary();


XRAD_END

#endif // roi_vocabulary_h__
