
#include "Q:\projects\StringsPhantom\StringsPhantom\pre.h"
#include "CreateVolumes.h"
#include "GenerateFigures.h"

//#include "pre.h"

#include "Q:\XRAD\XRADGUI\Sources\RasterImageFile\RasterImageFile.h"
#include <XRADDicom/XRADDicom.h>
#include <XRADDicomGUI/XRADDicomGUI.h>
#include "Q:\XRAD\XRADSystem\Sources\System\xrad_fopen.h"

XRAD_BEGIN

inline double cycle(const double& x, const double& min_val, const double& max_val)
{
	if (x <= max_val && x >= min_val)
	{
		return x;
	}
	else
	{
		double div = max_val - min_val;
		double	n = (x - min_val) / div;
		return min_val + (n - floor(n)) * div;
	}
}

void GenerateStringsInSlice(RealFunction2D_UI8& slice, size_t n_points, double agility, double average_speed, double string_radius,
	MathFunction<point2_F32, double, number_complexity::scalar>	positions, MathFunction<point2_F32, double, number_complexity::scalar>	speeds, bool flag_wiggle)
{
	for (size_t i = 0; i < n_points; ++i)
	{
		point2_F32	speed_increment(RandomUniformF64(-agility, agility), RandomUniformF64(-agility, agility));
		speeds[i] += speed_increment;
		speeds[i].x() = cycle(speeds[i].x(), -average_speed, average_speed);
		speeds[i].y() = cycle(speeds[i].y(), -average_speed, average_speed);
		if (!in_range(positions[i].y() + speed_increment.y(), 0, slice.vsize() - 1)) speeds[i].y() *= -1;
		if (!in_range(positions[i].x() + speed_increment.x(), 0, slice.hsize() - 1)) speeds[i].x() *= -1;
		if (flag_wiggle)
		{
			positions[i] += speeds[i];
		}
		PutCircle(slice, positions[i], string_radius);
	}
}

void GenerateLinesBetweenStrings(size_t z, size_t n_points, double string_radius, vector<range2_F32>& strings, MathFunction<point2_F32, double, number_complexity::scalar> positions)
{
	size_t	n1, n2;
	double delta_x, delta_y;
	if (!(z % int(string_radius * 2)))
	{
		for (auto& string : strings)
		{
			n1 = RandomUniformF64(0, n_points - 1);
			n2 = RandomUniformF64(0, n_points - 1);
			string = range2_F32(positions[n1], positions[n2]);
		}
	}
}


void DontGenerateLines(size_t z, double string_radius, vector<range2_F32>& strings, MathFunction<point2_F32, double, number_complexity::scalar> positions)
{
	size_t	n1(0), n2(0);
	if (!(z % int(string_radius * 2)))
	{
		for (auto& string : strings)
		{
			string = range2_F32(positions[n1], positions[n2]);
		}
	}
}


void GenerateLinesBetweenWalls(size_t z, double string_radius, vector<range2_F32>& strings, point2_F32 start, point2_F32 end)
{
	point2_F32 p1, p2;
	p1.x();
	p1.y();
	size_t dice;
	size_t	n1, n2;
	double delta_x, delta_y;
	if (!(z % int(string_radius * 2)))
	{
		for (auto& string : strings)
		{
			size_t max(100);
			dice = RandomUniformF64(0, max);
			
			if (dice < max/6)
			{
				p1.x() = start.y();                              //от верхней к нижней
				p1.y() = RandomUniformF64(start.x(), end.x());
				p2.x() = end.y();
				p2.y() = RandomUniformF64(start.x(), end.x());
			}
			else if (dice < 2*max/6)
			{
				p1.x() = RandomUniformF64(start.y(), end.y());  //от левой к правой
				p1.y() = start.x();
				p2.x() = RandomUniformF64(start.y(), end.y());
				p2.y() = end.x();
			}
			else if (dice < 3*max / 6)   
			{
				p1.x() = start.y();                              //от верхней к правой
				p1.y() = RandomUniformF64(start.x(), end.x());
				p2.x() = RandomUniformF64(start.y(), end.y());
				p2.y() = end.x();
			}
			else if (dice < 4*max / 6)
			{
				p1.x() = RandomUniformF64(start.y(), end.y());   //от левой к нижней
				p1.y() = start.x();
				p2.x() = end.y();
				p2.y() = RandomUniformF64(start.x(), end.x());
			}
			else if (dice < 5*max / 6)
			{
				p1.x() = start.y();                               //от левой к верхней
				p1.y() = RandomUniformF64(start.x(), end.x());
				p2.x() = RandomUniformF64(start.y(), end.y());
				p2.y() = start.x();
			}
			else 
			{
				p1.x() = end.y();                                 //от нижней к правой
				p1.y() = RandomUniformF64(start.x(), end.x());
				p2.x() = RandomUniformF64(start.y(), end.y());
				p2.y() = end.x();
			}

			
			string = range2_F32(p1, p2);
		}
	}


}
/*
void SaveToHDD(RealFunctionMD_UI8& volume, wstring directory_address, wstring folder_name, bool flag_save)
{
	if (flag_save)
	{
		//wstring adress = GetString(L"directory", directory_address);
		//wstring folder = GetString(L"folder for images of the phantom", folder_name);
		//Save(volume, folder, adress);
		Save(volume, directory_address + folder_name);
	}
}
*/
//void SimulateSponge(RealFunctionMD_UI8& volume, size_t n_points, double agility, double average_speed, double string_radius, vector<range2_F32>& strings,
//					MathFunction<point2_F32, double, number_complexity::scalar>	positions, MathFunction<point2_F32, double, number_complexity::scalar>	speeds, point2_F32 start, point2_F32 end, bool flag_horizontal_strings)
void SimulateSponge(RealFunctionMD_UI8& volume, size_t n_points, double agility, double average_speed, double string_radius, vector<range2_F32>& strings,
					MathFunction<point2_F32, double, number_complexity::scalar>	positions, MathFunction<point2_F32, double, number_complexity::scalar>	speeds, point2_F32 start, point2_F32 end, bool flag_horizontal_strings, size_t n_slices, wstring address)
{
	size_t answer(2);
	bool flag_wiggle(false);
	//Question(answer);
	if (flag_horizontal_strings)
	{
		answer = Decide("Choose mesh generation method", { "Connect walls", "Connest strings", "Exit" });
		flag_wiggle = true;
	}
	size_t local_z(0);
	flag_wiggle = YesOrNo("Should the strings wiggle?", flag_wiggle);
	GUIProgressBar progress;
	progress.start("Sponge simulation", volume.sizes(0));
	//for (size_t z = 0; z < volume.sizes(0); ++z)
	for (size_t z = 0; z < n_slices; ++z)
	{
		if (answer == 0) GenerateLinesBetweenWalls(z, string_radius, strings, start, end);
		if (answer==1) GenerateLinesBetweenStrings(z, n_points, string_radius, strings, positions);
		if (answer == 2) DontGenerateLines(z, string_radius, strings, positions);
		//auto slice = volume.GetSlice({ z, slice_mask(1), slice_mask(0) });
		if (volume.sizes(0) > 1) { local_z = z; }
		auto slice = volume.GetSlice({ local_z, slice_mask(1), slice_mask(0) });
		
		//RealFunction2D_F32 slice(volume.sizes(1), volume.sizes(2));
		//if (volume.sizes(0) == 1) { slice = volume.GetSlice({ z, slice_mask(1), slice_mask(0) }); }
		for (auto& string : strings) DrawLine(slice, string, string_radius);
		GenerateStringsInSlice(slice, n_points, agility, average_speed, string_radius, positions, speeds, flag_wiggle);

		if (volume.sizes(0) == 1)
		{
			//wstring	filename = ssprintf(adress + folder + L"//%d.png", z);
			wstring	filename = ssprintf(address + L"//%d.png", z+1);
			SaveRasterImage(filename, slice, 0, 255);

			slice.fill(0);
		}

		progress++;
	}
	progress.end();
}

void GenerateSpeedsAndPositions(size_t n_points, point2_F32	start, point2_F32 end, double average_speed,
	MathFunction<point2_F32, double, number_complexity::scalar>&	positions, MathFunction<point2_F32, double, number_complexity::scalar>&	speeds, double string_radius, bool flag_random)
{
	
	if (flag_random) 
	{
		for (size_t i = 0; i < n_points; ++i)
		{
			positions[i].y() = RandomUniformF64(start.x() + string_radius, end.x() - string_radius);
			positions[i].x() = RandomUniformF64(start.y() + string_radius, end.y() - string_radius);
			speeds[i].x() = RandomUniformF64(-average_speed, average_speed);
			speeds[i].y() = RandomUniformF64(-average_speed, average_speed);
		}
	}
	else
	{
		size_t width(end.x() - start.x());
		size_t hight(end.y() - start.y());
		size_t s(width * hight);
		double density(double(s) / double(n_points));
		double step(sqrt(density));
		printf("pixels in phantom = %zu\n", s);
		printf("pixels per string = %lf\n", density);
		printf("step = %lf\n", step);
		
		printf("start.x() = %lf\n", start.x());
		printf("end.x() = %lf\n", end.x());
		printf("start.y() = %lf\n", start.y());
		printf("end.y() = %lf\n", end.y());
		for (size_t i = 0, n = 0, j = 0; i < n_points; ++i)
		{
			positions[i].y() = start.x() + string_radius + j * step;
			positions[i].x() = start.y() + string_radius + n * step;
			//printf("positions[%zu].x() = %lf\n", i, positions[i].x());
			
			if (positions[i].x() > (end.y() - string_radius - step))
			{
				j++;
				n = 0;
			}
			else n++;
		}
	}
}

//void	SimulateOnePhantom(RealFunctionMD_UI8& volume, size_t n_points, double agility, double average_speed, double string_radius, size_t wall_thickness, point2_F32 start, point2_F32 end)
void	SimulateOnePhantom(RealFunctionMD_UI8& volume, size_t n_points, double agility, double average_speed, double string_radius, size_t wall_thickness, point2_F32 start, point2_F32 end, size_t n_slices, wstring address)
{
	MathFunction<point2_F32, double, number_complexity::scalar>	positions(n_points, point2_F32(0));//использование неуместного number_complexity
	MathFunction<point2_F32, double, number_complexity::scalar>	speeds(n_points, point2_F32(0));

	bool flag_random = YesOrNo("Position strings randomly?", true);
	GenerateSpeedsAndPositions(n_points, start, end, average_speed, positions, speeds, string_radius, flag_random);
	bool flag_horizontal_strings(false);
	if (flag_random)
	{
		flag_random = true;
	}
	flag_horizontal_strings = YesOrNo("Add horizontal strings?", flag_horizontal_strings);

	size_t n_lines(30);
	if (flag_horizontal_strings)
	{
		n_lines = GetUnsigned("number of horizontal strings", 30, 0, n_points);
	}
	vector<range2_F32> strings(n_lines);

	//SimulateSponge(	volume, n_points, agility, average_speed, string_radius, strings, positions, speeds, start, end, flag_horizontal_strings);
	SimulateSponge(volume, n_points, agility, average_speed, string_radius, strings, positions, speeds, start, end, flag_horizontal_strings, n_slices, address);
	//CreateElementsOfSupport(volume, wall_thickness, start, end, flag_horizontal_strings);
	//CreateElementsOfSupport(volume, wall_thickness, start, end, flag_horizontal_strings, n_slices);
	CreateElementsOfSupport(volume, wall_thickness, start, end, flag_horizontal_strings, n_slices, address);
}

//void	SimulateManyPhantoms(RealFunctionMD_UI8	&volume, size_t n_points_min, size_t n_strings_max, double agility, double average_speed, double string_radius, size_t wall_thickness, point2_F32	start, point2_F32	end, size_t n_phantoms, size_t h_1sm, size_t h_7mm, size_t w_7mm)
void	SimulateManyPhantoms(RealFunctionMD_UI8& volume, size_t n_points_min, size_t n_strings_max, double agility, double average_speed, double string_radius, 
							size_t wall_thickness, point2_F32	start, point2_F32	end, size_t n_phantoms, size_t h_1sm, size_t h_7mm, size_t w_7mm, size_t n_slices, wstring address)
{
	RealFunctionMD_UI8	volume_local(volume);
	point2_F32	start_0(start);
	point2_F32	end_0(end);
	size_t width_x = end.x() - start.x() + h_7mm + wall_thickness;
	size_t width_y = end.y() - start.y() + h_7mm + wall_thickness;
	size_t n_points(n_points_min);
	size_t step = (n_strings_max - n_points_min) / n_phantoms;
	start.x() = start_0.x();
	end.x() = end_0.x();
	start.y() = start_0.y();
	end.y() = end_0.y();
	GUIProgressBar progress_phantoms;
	progress_phantoms.start("SimulateManyPhantoms", n_phantoms);
	for (size_t i = 0; i < n_phantoms; ++i)	
	{
		if (end.y() > volume_local.sizes(1))
		{
			start.y() = start_0.y();
			end.y() = end_0.y();
			start.x() += width_x;
			end.x() += width_x;
		}
		//SimulateOnePhantom(volume_local, n_points, agility, average_speed, string_radius, wall_thickness, start, end);
		SimulateOnePhantom(volume_local, n_points, agility, average_speed, string_radius, wall_thickness, start, end, n_slices, address);
		volume += volume_local;
		start.y() += width_y;
		end.y() += width_y;
		n_points += step;
		progress_phantoms++;
	}
}

void	CreateSphere(size_t	n_slices, size_t w, size_t h)
{
	size_t x_point = GetUnsigned("x_point (center of a sphere)", h/2, 0, h);
	size_t y_point = GetUnsigned("y_point (center of a sphere)",w/2, 0, w);
	size_t z_point = GetUnsigned("z_point (center of a sphere)", n_slices/2, 0, n_slices);
	size_t radius = GetUnsigned("radius of a sphere", 70, 0, 300);
	RealFunctionMD_UI8	volume({ n_slices, h, w }, 0);
	for (size_t z = 1; z < n_slices; ++z)
	{
		auto slice = volume.GetSlice({ z, slice_mask(1), slice_mask(0) });

		for (size_t i = 1; i < h; ++i)
		{
			for (size_t j = 1; j < w; ++j)
			{
				if (((i- x_point)*(i - x_point) + (j - y_point)*(j - y_point) + (z - z_point)*(z - z_point)) < (radius*radius))
				{
					slice.at(i, j) = 255;
				}
			}
		}
	}
	DisplayMathFunction3D(volume, "Sphere");
	bool flag_save = YesOrNo("Save sphere as images?", true);
	//wstring adress = GetString(L"directory", L"c://temp//");
	//wstring folder = GetString(L"folder for sphere", L"sphere");
	//if (flag_save) Save(volume, folder, adress);
	wstring address = GetFolderNameRead(L"Choose folder for sphere", saved_default_value);
	if (flag_save) Save(volume, address);
}

void CreateEmptyVolume(size_t	n_slices, size_t w, size_t h)
{
	bool flag_save = YesOrNo("Save empty volume as images?", true);
	wstring address = GetFolderNameRead(L"Choose folder for empty volume", saved_default_value);

	//RealFunctionMD_UI8	volume({ n_slices, h, w }, 0); 
	
	RealFunctionMD_UI8	volume({ 1, h, w }, 0);
	auto slice = volume.GetSlice({ 0, slice_mask(1), slice_mask(0) });

	/*
	if (!flag_save_to_hdd)
	{
		volume.realloc({ n_slices, h, w }, 0);
		DisplayMathFunction3D(volume, "empty volume");
	}
	*/
	wstring filename;
	GUIProgressBar prog;
	prog.start(L"Saving empty volume to HDD", n_slices);
	//size_t j = 0;
	for (size_t i = 0; i < n_slices; ++i)
	{
		filename = ssprintf(address + L"//%d.png", i+1);
		SaveRasterImage(filename, slice, 0, 255);
		++prog;
	}
	prog.end();

	
	
	//if (flag_save) Save(volume, address);
}

void Combine(RealFunctionMD_UI8	&volume1, RealFunctionMD_UI8 volume2, RealFunctionMD_UI8 volume3, bool flag_inverse_mask)
{
	for (size_t z = 0; z < volume1.sizes(0); ++z)
	{
		auto slice1 = volume1.GetSlice({ z, slice_mask(1), slice_mask(0) });
		auto slice2 = volume2.GetSlice({ z, slice_mask(1), slice_mask(0) });
		auto slice3 = volume3.GetSlice({ z, slice_mask(1), slice_mask(0) });	

		for (size_t i = 0; i < slice1.vsize(); ++i)
		{
			for (size_t j = 0; j < slice1.hsize(); ++j)
			{
				if (!flag_inverse_mask) 
				{
					if (slice2.at(i, j) > 0) 
					{ 
						slice1.at(i, j) = slice3.at(i, j);
					} 
				}
				if (flag_inverse_mask) 
				{
					if (slice2.at(i, j) < 1) 
					{ 
						slice1.at(i, j) = slice3.at(i, j); 
					} 
				}
			}
		}
	}
}

void	CombineVolumes(size_t	n_slices, size_t w, size_t h)
{
	bool store_in_RAM = YesOrNo("Load full volumes in RAM?", false);

	//RealFunctionMD_UI8	volume1({ n_slices, h, w }, 0);  //для губки
	RealFunctionMD_UI8	volume1;  //для губки
	//RealFunctionMD_UI8	volume2({ n_slices, h, w }, 0);  //для сферы
	RealFunctionMD_UI8	volume2;  //для сферы
	//RealFunctionMD_UI8	volume3({ n_slices, h, w }, 0);  //для пустого массива
	RealFunctionMD_UI8	volume3;  //для пустого массива
	
	wstring folder1 = GetFolderNameRead(L"Choose folder for Volume 1", saved_default_value); 
	wstring folder2 = GetFolderNameRead(L"Choose folder for Volume 2", saved_default_value);
	wstring folder3 = GetFolderNameRead(L"Choose folder for Volume 3", saved_default_value);
	wstring folder4 = GetFolderNameRead(L"Choose folder for Result", saved_default_value); 

	bool flag_inverse_mask = YesOrNo("Inverse mask?", false);

	if (store_in_RAM)
	{
		
		volume1.realloc({ n_slices, h, w }, 0);
		volume2.realloc({ n_slices, h, w }, 0);
		volume3.realloc({ n_slices, h, w }, 0);
		
		bool display_loaded_images = YesOrNo("Display loaded images?", false);

		LoadFullVolume(volume1, folder1);
		if (display_loaded_images) { DisplayMathFunction3D(volume1, folder1); }

		LoadFullVolume(volume2, folder2);
		if (display_loaded_images) { DisplayMathFunction3D(volume2, folder2); }

		LoadFullVolume(volume3, folder3);
		if (display_loaded_images) { DisplayMathFunction3D(volume3, folder3); }

		Combine(volume1, volume2, volume3, flag_inverse_mask);
		if (display_loaded_images)
		{
			DisplayMathFunction3D(volume1, "after combining");
			bool flag_save = YesOrNo("Save result as images?", true);
			if (flag_save) Save(volume1, folder4);
		}

		if (!display_loaded_images) Save(volume1, folder4);
	}	
	else
	{

		volume1.realloc({ 1, h, w }, 0);
		volume2.realloc({ 1, h, w }, 0);
		volume3.realloc({ 1, h, w }, 0);


		wstring filename;



		GUIProgressBar prog;
		prog.start(L"Combining volumes", n_slices);
		for (size_t z = 0; z < n_slices; ++z)
		{
			filename = ssprintf(folder1 + L"//%d.png", z + 1);
			RasterImageFile my_file1(filename);
			auto slice1 = volume1.GetSlice({ 0, slice_mask(1), slice_mask(0) });
			slice1.CopyData(my_file1.channel(xrad::color_channel::lightness));

			filename = ssprintf(folder2 + L"//%d.png", z + 1);
			RasterImageFile my_file2(filename);
			auto slice2 = volume2.GetSlice({ 0, slice_mask(1), slice_mask(0) });
			slice2.CopyData(my_file2.channel(xrad::color_channel::lightness));

			filename = ssprintf(folder3 + L"//%d.png", z + 1);
			RasterImageFile my_file3(filename);
			auto slice3 = volume3.GetSlice({ 0, slice_mask(1), slice_mask(0) });
			slice3.CopyData(my_file3.channel(xrad::color_channel::lightness));

			Combine(volume1, volume2, volume3, flag_inverse_mask);
			//Save(volume1, folder4);

			filename = ssprintf(folder4 + L"//%d.png", z);
			auto slice4 = volume1.GetSlice({ 0, slice_mask(1), slice_mask(0) });
			SaveRasterImage(filename, slice4, 0, 255);

			++prog;
		}
		prog.end();
	}
}

void	CreateTestObjects(size_t	n_slices, size_t w, size_t h)
{
	size_t x_point = GetUnsigned("x_point", 1, 0, h);
	size_t y_point = GetUnsigned("y_point", 1, 0, w);
	double	radius_start(0);
	double	radius_end(30);

	ComplexFunctionF64 SetRadius(512, complexF64(0));

	for (size_t i = 0; i < SetRadius.size() - 1; ++i)
	{
		SetRadius[i] = RandomUniformF64(radius_start, radius_end); //цикл генерации радиусов случайных величин
	}

	DisplayMathFunction(SetRadius, 0, 1, "SetRadius");
	FFT(SetRadius, ftForward); //прямое преобразование Фурье
	DisplayMathFunction(SetRadius, 0, 1, "FFT");
	for (size_t i = SetRadius.size() / 20; i < SetRadius.size() - 1; ++i) // Определяем полосу пропускания ФНЧ
	{
		SetRadius[i] = 0; //цикл для ФНЧ
	}
	DisplayMathFunction(SetRadius, 0, 1, "FFT After filtration");

	FFT(SetRadius, ftReverse); //обратное преобразование Фурье
	DisplayMathFunction(SetRadius, 0, 1, "After filtration"); //убираем вывод дисплея с графиком отфильтрованного сигнала
	RealFunction2D_UI8 CreateRandomObject(300, 300, 0); //задаем матрицу и заполнили нулями, объект должен вписываться в окружность радиусом 179
	size_t x_center(CreateRandomObject.vsize() / 2);
	size_t y_center(CreateRandomObject.hsize() / 2);

	for (size_t i = 0; i < SetRadius.size() - 1; ++i)
	{
		double r = SetRadius[i].re;
		double fi = double(i) - double(90);
		for (size_t n = 0; n < r; ++n)
		{
			double grad = (double(fi) / double(180) * double(3.14));
			size_t	x = x_center + double(sin(grad))*n;
			size_t	y = y_center + double(cos(grad))*n;
			CreateRandomObject.at(x, y) = 255;// вносим значения х у в матрицу
		}

	}

	DisplayMathFunction2D(CreateRandomObject, "CreateRandomObject");

	double string_radius = GetUnsigned("string_radius", 1, 1, 1000);
	point2_F32 position(x_point, y_point);
	double RandomRadius = RandomUniformF64(radius_start, radius_end);

	RealFunctionMD_UI8	volume_local({ n_slices, h, w }, 0);
	
	for (size_t z = 0; z < volume_local.sizes(0); ++z)
	{
		auto slice = volume_local.GetSlice({ z, slice_mask(1), slice_mask(0) });

		PutFigure(slice, position, string_radius);

	}
	DisplayMathFunction3D(volume_local, "sponge");
}

void	TestSpongeSimulation(size_t	n_slices, size_t w , size_t h)
{
	
	bool flag_save_to_hdd = YesOrNo("Save data to HDD?", true);
	wstring address;
	if (flag_save_to_hdd)
	{
		//address = GetString(L"directory", L"C:\\temp\\sponge");
		address = GetFolderNameRead(L"Choose folder", saved_default_value);
		//string GetFolderNameRead(string prompt, GUIValue<string> default_name = saved_default_value, string filter = "*.*");
		//filename = GetFileNameRead(L"Choose data files", saved_default_value, L"*.par"); //GetFileNameRead(L"Choose data files", L"*.par");

	}

	size_t w_mm = 120; //68;
	size_t h_mm = 68; //120;
	size_t z_mm = 170;
	size_t w_1sm = 10 * w / w_mm;
	size_t h_1sm = 10 * h / h_mm;
	printf("px per cm along x = %zu\n", h_1sm);
	printf("px per cm along y = %zu\n\n", w_1sm);
	size_t w_7mm = 7 * w / w_mm;
	size_t h_7mm = 7 * h / h_mm;
	size_t h_3sm = 30 * h / h_mm;
	double	string_radius = 2.5;
	string_radius = GetFloating("string radius in px", string_radius, 1, 100);
	size_t	n_strings = w*h/(pi()*square(string_radius));//половина площади, полное затухание 7.5 МГц на 1 см при koef=0.5
	fflush(stdout);

	double	agility = 0.5;
	double	average_speed = agility;
	size_t	wall_thickness = 20;
	//RealFunctionMD_UI8	volume({ n_slices, h, w }, 0);
	RealFunctionMD_UI8	volume({ 1, h, w }, 0); 
	if (!flag_save_to_hdd) 
	{ 
		volume.realloc({ n_slices, h, w }, 0);
	}


	bool flag_continious(false);
	do
	{
		
		size_t x_width = GetUnsigned("x_width (width of the sponge)", h_1sm, 0, h);
		size_t y_width = GetUnsigned("y_width (width of the sponge)", w_1sm, 0, w);

		size_t x_point = GetUnsigned("x_point (center of the sponge)", h / 2, x_width/2, h - x_width / 2);
		size_t y_point = GetUnsigned("y_point (center of the sponge)", w / 2, y_width/2, w - y_width / 2);
		
		size_t x_left = x_point - x_width /2; //0.2*h
		size_t x_right = x_point + x_width / 2;;
		//x_right = x_left + GetUnsigned("phantom width in px along x", h_1sm, h_1sm, h_1sm*5);
		//size_t x_width = x_right - x_left;
		printf("phantom width in px along x = %zu\n", x_width);
		size_t y_left = y_point - y_width / 2;// 0.2*w
		size_t y_right = y_point + y_width / 2;
		//y_right = y_left + GetUnsigned("phantom width in px along y", w_1sm, w_1sm, w_1sm*5);
		//size_t y_width = y_right - y_left;
		printf("phantom width in px along y = %zu\n\n", y_width);
		point2_F32	start(x_left, y_left);
		point2_F32	end(x_right, y_right);
		size_t n_phantoms = GetUnsigned("n_phantoms", 1, 1, 40);
		size_t strings_density;
		size_t n_strings_density_max;
		if(n_phantoms > 1)
		{
			strings_density = GetUnsigned("How many strings per cm^2? (min)", 300, 1, 7000);
			n_strings_density_max = GetUnsigned("How many strings per cm^2? (max)", strings_density, strings_density, 7000);
		} 
		else
		{
			strings_density = GetUnsigned("How many strings per cm^2?", 300, 1, 7000);
			n_strings_density_max = strings_density;
		}
		n_strings = strings_density*x_width*y_width / (h_1sm * w_1sm);
		size_t n_strings_max = n_strings_density_max*x_width*y_width /(h_1sm * w_1sm);

		if (n_phantoms > 1)
		{
			printf("number of strings per cm^2 in the first phantom = %zu\n", strings_density);
			printf("number of strings per cm^2 in the last phantom = %zu\n", n_strings_density_max);
			printf("number of strings in the first phantom = %zu\n", n_strings);
			printf("number of strings in the last phantom = %zu\n", n_strings_max);
		}
		else
		{
			printf("number of strings per cm^2 = %zu\n", strings_density);
			printf("number of strings in phantom = %zu\n", n_strings);
		}
		size_t y_void = 0.4*w;
		//RealFunctionMD_UI8	volume_local({ n_slices, h, w }, 0);
		RealFunctionMD_UI8	volume_local({ 1, h, w }, 0);
		if (!flag_save_to_hdd)
		{
			volume_local.realloc({ n_slices, h, w }, 0);
		}


		//SimulateManyPhantoms(volume_local, n_strings, n_strings_max, agility, average_speed, string_radius, wall_thickness, start, end, n_phantoms, h_1sm, h_7mm, w_7mm);
		SimulateManyPhantoms(volume_local, n_strings, n_strings_max, agility, average_speed, string_radius, wall_thickness, start, end, n_phantoms, h_1sm, h_7mm, w_7mm, n_slices, address);
		volume += volume_local;
		
		if (!flag_save_to_hdd)
		{
			DisplayMathFunction3D(volume, "sponge");
			bool flag_save = YesOrNo("Save sponge to c://temp// as images?", true);

			if (flag_save) 
			{ 
			//	SaveToHDD(volume, L"c://temp//", L"sponge", flag_save); 
				address = GetFolderNameRead(L"Choose folder to save the data", saved_default_value);
				Save(volume, address);
			}
		}

		/*
		if (flag_save)
		{

			wstring adress = GetString(L"directory", L"c://temp//");
			wstring folder = GetString(L"folder for images of the phantom", L"sponge");
			Save(volume, folder, adress);
		}
		*/

		flag_continious = YesOrNo("Create another?", true);					
	} 
	while (flag_continious);
	return;
}

XRAD_END
