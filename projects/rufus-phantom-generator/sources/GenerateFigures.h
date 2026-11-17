/*!
	\file
	\date 8:11:2016 17:41
	\author kns
*/
#ifndef GenerateFigures_h__
#define GenerateFigures_h__

#include <XRADBasic/MathFunctionTypesMD.h>
#include "pre.h"

XRAD_BEGIN


//recommended start params
// n_points = 4*s
// agility = 0.05
// average_speed = 0.02
// string_radius = 2.5
// 	foundation_thickness = 20;



//void	SimulateSponge(RealFunctionMD_UI8	&f, size_t n_points, double agility, double average_speed, double string_radius, size_t foundation_thickness);
//void	SimulateSponge(RealFunctionMD_UI8	&f, size_t n_points, double agility, double average_speed, double string_radius, size_t foundation_thickness, size_t x_void, size_t y_void);
//void	SimulateOnePhantom(RealFunctionMD_UI8	&f, size_t n_points, double agility, double average_speed, double string_radius, size_t foundation_thickness, point2_F32	start, point2_F32	end, bool flag);
//void	TestSpongeSimulation(size_t	n_slices, size_t w, size_t h);
//void	CreateTestObjects(size_t	n_slices, size_t w, size_t h);
//void	CreateSphere(size_t	n_slices, size_t w, size_t h);
//void	CombineVolumes(size_t	n_slices, size_t w, size_t h);
//void	CreateEmptyVolume(size_t	n_slices, size_t w, size_t h);
//void Save(RealFunctionMD_UI8& volume, wstring folder, wstring adress);
void Save(RealFunctionMD_UI8& volume, wstring folder);
//void Load(RealFunctionMD_UI8& volume, wstring folder, wstring adress);
void	LoadFullVolume(RealFunctionMD_UI8& volume, wstring folder);
void	PutCircle(RealFunction2D_UI8& slice, point2_F32& p, double r);
void	PutFigure(RealFunction2D_UI8& slice, point2_F32& p, double r);
void	DrawLine(RealFunction2D_UI8& img, const range2_F32& ends, double radius);
//void	DrawWall(RealFunction2D_UI8& img, const range2_F32& ends, double wall_thickness);
void	DrawWall(RealFunction2D_UI8& img, const range2_F32& ends);
//void CreateElementsOfSupport(RealFunctionMD_UI8& volume, size_t wall_thickness, point2_F32	start, point2_F32 end, bool flag);
void CreateElementsOfSupport(RealFunctionMD_UI8& volume, size_t wall_thickness, point2_F32 start, point2_F32 end, bool flag_horizontal_strings, size_t n_slices, wstring address);

XRAD_END

#endif // GenerateFigures_h__
