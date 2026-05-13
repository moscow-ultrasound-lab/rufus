/*!
	\file
	\date 8:11:2016 17:41
	\author kns
*/
#ifndef CombineVolumes_h__
#define CombineVolumes_h__

#include <XRADBasic/MathFunctionTypesMD.h>

XRAD_BEGIN



//void	SimulateSponge(RealFunctionMD_UI8	&f, size_t n_points, double agility, double average_speed, double string_radius, size_t foundation_thickness);
//void	SimulateSponge(RealFunctionMD_UI8	&f, size_t n_points, double agility, double average_speed, double string_radius, size_t foundation_thickness, size_t x_void, size_t y_void);
//void	SimulateOnePhantom(RealFunctionMD_UI8	&f, size_t n_points, double agility, double average_speed, double string_radius, size_t foundation_thickness, point2_F32	start, point2_F32	end, bool flag);
void	TestSpongeSimulation(size_t	n_slices, size_t w, size_t h);
void	CreateTestObjects(size_t	n_slices, size_t w, size_t h);
void	CreateSphere(size_t	n_slices, size_t w, size_t h);
void	CombineVolumes(size_t	n_slices, size_t w, size_t h);
void	CreateEmptyVolume(size_t	n_slices, size_t w, size_t h);
XRAD_END

#endif // CombineVolumes_h__
