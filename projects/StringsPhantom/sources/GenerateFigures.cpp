/*!
	\file
	\date 8:11:2016 17:41
	\author kns
*/
//#include "Q:\projects\StringsPhantom\StringsPhantom\StringsPhantom\pre.h"
#include "Q:\projects\StringsPhantom\StringsPhantom\pre.h"
#include "CreateVolumes.h"
#include "GenerateFigures.h"

//#include "pre.h"

#include "Q:\XRAD\XRADGUI\Sources\RasterImageFile\RasterImageFile.h"
#include <XRADDicom/XRADDicom.h>
#include <XRADDicomGUI/XRADDicomGUI.h>
#include "Q:\XRAD\XRADSystem\Sources\System\xrad_fopen.h"
XRAD_BEGIN

void	PutCircle(RealFunction2D_UI8 &slice, point2_F32 &p, double r)
{
	double	smoothness = 0.5;//0...1
	ptrdiff_t	v0 = range(p.y()-r-1, 0, slice.vsize()-1);
	ptrdiff_t	v1 = range(p.y()+r+1, 0, slice.vsize()-1);
	ptrdiff_t	h0=range(p.x()-r-1, 0, slice.hsize()-1);
	ptrdiff_t	h1=range(p.x()+r+1, 0, slice.hsize()-1);
	float	power = 0.5;
	for(ptrdiff_t i = v0; i < v1; ++i)
	{
		for(ptrdiff_t j = h0; j < h1; ++j)
		{
			double	distance = std::hypot(double(i)-p.y(), double(j)-p.x());
			float	b = 255.*pow(range((r-distance)/smoothness, 0, 1), power);
			slice.at(i,j) = max(uint8_t(b), slice.at(i,j));
		}
	}
}

void	PutFigure(RealFunction2D_UI8 &slice, point2_F32 &p, double r)
{
	double	smoothness = 0.5;//0...1
	ptrdiff_t	v0 = range(p.y() - r - 1, 0, slice.vsize() - 1);
	ptrdiff_t	v1 = range(p.y() + r + 1, 0, slice.vsize() - 1);
	ptrdiff_t	h0 = range(p.x() - r - 1, 0, slice.hsize() - 1);
	ptrdiff_t	h1 = range(p.x() + r + 1, 0, slice.hsize() - 1);
	float	power = 0.5;
	for (ptrdiff_t i = v0; i < v1; ++i)
	{
		for (ptrdiff_t j = h0; j < h1; ++j)
		{
			double	distance = std::hypot(double(i) - p.y(), double(j) - p.x());
			float	b = 255.*pow(range((r - distance) / smoothness, 0, 1), power);
			slice.at(i, j) = max(uint8_t(b), slice.at(i, j));
		}
	}
}

void Save(RealFunctionMD_UI8	&volume, wstring folder)
{
	//wstring folder = GetString(L"folder", L"sponge");
	//wstring adress = L"c://temp//";
	wstring	filename;
	GUIProgressBar prog;
	prog.start(L"writing plates", volume.sizes(0));
	//size_t j = 0;
	for (size_t i = 0; i < volume.sizes(0); ++i)
	{
		//wstring	filename;
		//filename = ssprintf(adress + folder + L"//%d.png", ++j);
		//filename = ssprintf(adress + folder + L"//%d.png", i);
		filename = ssprintf(folder + L"//%d.png", i);
		auto slice = volume.GetSlice({ i, slice_mask(1), slice_mask(0) });
		SaveRasterImage(filename, slice, 0, 255);
		++prog;

	}
	prog.end();
}

//void Load(RealFunctionMD_UI8& volume, wstring folder, wstring adress)
void LoadFullVolume(RealFunctionMD_UI8& volume, wstring folder)
{
	//wstring folder = GetString(L"folder", L"sponge");
	//wstring adress = L"c://temp//";
	wstring	filename;
	GUIProgressBar prog;
	prog.start(L"reading from folder: "+ folder, volume.sizes(0));
	//size_t j = 0;
	for (size_t i = 0; i < volume.sizes(0); ++i)
	{
		//wstring	filename;
		//filename = ssprintf(adress + folder + L"//%d.png", ++j);
		//filename = ssprintf(adress + folder + L"//%d.png", i);
		filename = ssprintf(folder + L"//%d.png", i);
		RasterImageFile my_file(filename);
		auto my_image = my_file.channel(xrad::color_channel::lightness);
		auto slice = volume.GetSlice({ i, slice_mask(1), slice_mask(0) });
		slice.CopyData(my_image);
		++prog;
	}
	prog.end();
	//DisplayMathFunction3D(volume, folder);
}

/*
void	DrawLine(RealFunction2D_UI8	&img, const range2_F32 &ends, double radius)
{
	//for(size_t i = ends.y1(); i < ends.y2(); ++i)
	for (size_t j = ends.x1(); j < ends.x2(); ++j)
	{
		//for(size_t j = ends.x1(); j < ends.x2(); ++j)
		for (size_t i = ends.y1(); i < ends.y2(); ++i)
		{
			double	y = i-ends.y1();
			double	x = j-ends.x1();

			if(fabs(y*ends.range_x().delta() - x*double(ends.range_y().delta())) < radius*norma(ends.delta()))
			{
				img.at(i, j) = 255;
			}
		}
	}
}
*/


void	DrawLine(RealFunction2D_UI8	&img, const range2_F32 &ends, double radius)
{
	size_t start_x(ends.x2()), end_x(ends.x1()), start_y(ends.y2()), end_y(ends.y1());
	
	if (ends.x1() < ends.x2())
	{
		start_x = ends.x1();
		end_x = ends.x2();
	}
	
	if (ends.y1() < ends.y2())
	{
		start_y = ends.y1();
		end_y = ends.y2();
	}
			
	for (size_t j = start_x; j < end_x; ++j)
	{
		for (size_t i = start_y; i < end_y; ++i)
		{
			double	y = i-ends.y1();
			double	x = j-ends.x1();

			if(fabs(y*ends.range_x().delta() - x*double(ends.range_y().delta())) < radius*norma(ends.delta()))
			{
				img.at(i, j) = 255;
			}
		}
	}
}

void	DrawWall(RealFunction2D_UI8& img, const range2_F32& ends)
{
	for (size_t i = ends.y1(); i <= ends.y2(); ++i)
	{
		for (size_t j = ends.x1(); j <= ends.x2(); ++j)
		{
			{
				img.at(i, j) = 255;
			}
		}
	}
}

void AddWall(RealFunctionMD_UI8& volume, size_t z, range2_F32 wall, wstring address)
{
	wstring filename;
	size_t local_z(0);
	auto slice = volume.GetSlice({ 0, slice_mask(1), slice_mask(0) });
	if (volume.sizes(0) > 1)
	{
		local_z = z;
		slice = volume.GetSlice({ local_z, slice_mask(1), slice_mask(0) });
	}
	else
	{
		filename = ssprintf(address + L"//%d.png", z + 1);
		RasterImageFile my_file(filename);
		auto my_image = my_file.channel(xrad::color_channel::lightness);
		slice = volume.GetSlice({ local_z, slice_mask(1), slice_mask(0) });
		slice.CopyData(my_image);
	}
	//auto slice = volume.GetSlice({ z, slice_mask(1), slice_mask(0) });

	DrawWall(slice, wall);

	if (volume.sizes(0) == 1)
	{
		wstring	filename = ssprintf(address + L"//%d.png", z + 1);
		SaveRasterImage(filename, slice, 0, 255);

		slice.fill(0);
	}
}


//void CreateElementsOfSupport(RealFunctionMD_UI8	&volume, size_t wall_thickness, point2_F32 start, point2_F32 end, bool flag_horizontal_strings)
void CreateElementsOfSupport(RealFunctionMD_UI8& volume, size_t wall_thickness, point2_F32 start, point2_F32 end, bool flag_horizontal_strings, size_t n_slices, wstring address)
{

	
	
	bool flag_wall_1(true);
	bool flag_wall_2(true);
	bool flag_wall_3(true);
	bool flag_wall_4(true);
	bool flag_wall_5(true);
	bool flag_wall_6(false);
	bool flag_columns(false);
	
	if (!flag_horizontal_strings)
	{
		flag_wall_2 = false;
		flag_wall_3 = false;
		flag_wall_4 = false;
		flag_wall_5 = false;
		flag_wall_6 = true;
		flag_columns = true;
	}

	if (!GetCheckboxDecision("Choose elements of support", { "wall #1" , "wall #2", "wall #3", "wall #4", "wall #5", "wall #6", "columns" }, 
		{&flag_wall_1, &flag_wall_2, &flag_wall_3, &flag_wall_4, &flag_wall_5, &flag_wall_6, &flag_columns })) {}
	
	if (flag_wall_1 || flag_wall_2 || flag_wall_3 || flag_wall_4 || flag_wall_5 || flag_wall_6)
	{
		wall_thickness = GetUnsigned("wall_thickness (px)", wall_thickness, 1, 15 * wall_thickness);
		printf("wall thickness (px) = %zu\n", wall_thickness);
	}

	if(flag_wall_1)
	{
		for (size_t z = 0; z < wall_thickness; ++z)
		{
			range2_F32 wall(point2_F32(start.x(), start.y()), point2_F32(end.x(), end.y()));
			AddWall( volume, z, wall, address);
		}
	}

	if (flag_wall_2)
	{
	//	for (size_t z = 0; z < volume.sizes(0); ++z)
		for (size_t z = 0; z < n_slices; ++z)
		{
			range2_F32 wall(point2_F32(start.x(), start.y()), point2_F32(start.x() + wall_thickness, end.y()));
			//auto slice = volume.GetSlice({ z, slice_mask(1), slice_mask(0) });
			//DrawWall(slice, wall);
			AddWall(volume, z, wall, address);
		}
	}

	if (flag_wall_3)
	{
		//for (size_t z = 0; z < volume.sizes(0); ++z)
		for (size_t z = 0; z < n_slices; ++z)
		{
			range2_F32 wall(point2_F32(start.x(), start.y()), point2_F32(end.x(), start.y() + wall_thickness));
			//auto slice = volume.GetSlice({ z, slice_mask(1), slice_mask(0) });
			//DrawWall(slice, wall);
			AddWall(volume, z, wall, address);
		}
	}

	if (flag_wall_4)
	{
		//for (size_t z = 0; z < volume.sizes(0); ++z)
		for (size_t z = 0; z < n_slices; ++z)
		{
			range2_F32 wall(point2_F32(end.x() - wall_thickness, start.y()), point2_F32(end.x(), end.y()));
			//auto slice = volume.GetSlice({ z, slice_mask(1), slice_mask(0) });
			//DrawWall(slice, wall);
			AddWall(volume, z, wall, address);
		}
	}

	if (flag_wall_5)
	{
//		for (size_t z = 0; z < volume.sizes(0); ++z)
		for (size_t z = 0; z < n_slices; ++z)
		{
			range2_F32 wall(point2_F32(start.x(), end.y() - wall_thickness), point2_F32(end.x(), end.y()));
			//auto slice = volume.GetSlice({ z, slice_mask(1), slice_mask(0) });
			//DrawWall(slice, wall);
			AddWall(volume, z, wall, address);
		}
	}

	if (flag_wall_6)
	{
		//for (size_t z = volume.sizes(0)- wall_thickness; z < volume.sizes(0); ++z)
		for (size_t z = n_slices - wall_thickness; z < n_slices; ++z)
		{
			range2_F32 wall(point2_F32(start.x(), start.y()), point2_F32(end.x(), end.y()));
			//auto slice = volume.GetSlice({ z, slice_mask(1), slice_mask(0) });
			//DrawWall(slice, wall);
			AddWall(volume, z, wall, address);
		}
	}

	if (flag_columns)
	{
		size_t column_thickness = GetUnsigned("column_thickness (px)", wall_thickness, 1, 15 * wall_thickness);
		printf("column thickness (px) = %zu\n\n", column_thickness);

		//for (size_t z = 0; z < volume.sizes(0); ++z)
		for (size_t z = 0; z < n_slices; ++z)
		{
			//auto slice = volume.GetSlice({ z, slice_mask(1), slice_mask(0) });
			range2_F32 colss(point2_F32(start.x(), start.y()), point2_F32(start.x() + column_thickness, start.y() + column_thickness));
			//DrawWall(slice, colss);
			AddWall(volume, z, colss, address);
			range2_F32 coles(point2_F32(end.x() - column_thickness, start.y()), point2_F32(end.x(), start.y() + column_thickness));
			//DrawWall(slice, coles);
			AddWall(volume, z, coles, address);
			range2_F32 colse(point2_F32(end.x() - column_thickness, end.y() - column_thickness), point2_F32(end.x(), end.y()));
			//DrawWall(slice, colse);
			AddWall(volume, z, colse, address);
			range2_F32 colee(point2_F32(start.x(), end.y() - column_thickness), point2_F32(start.x() + column_thickness, end.y()));
			//DrawWall(slice, colee);
			AddWall(volume, z, colee, address);
		}
	}
}


XRAD_END
