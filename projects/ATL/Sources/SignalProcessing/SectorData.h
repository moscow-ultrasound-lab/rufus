#ifndef	__sector_data_h
#define	__sector_data_h

#include "Options.h"
#include "SignalOptions.h"
//#include "AcousticFrameDisplayer.h"

XRAD_BEGIN

//-------------------------------------------------------------
//	вспомогательный класс ввода-вывода, на деле, стараЯ 
//	неуклюжаЯ конструкциЯ, собираю вместе, чтобы потом
//	уничтожить
//-------------------------------------------------------------


class	SectorDataIO_temp
{
protected:
	string DefaultOPFileName;
//	string OutputFileNameAppendix;

	FILE	*Signal_IP_File;
	FILE	*Signal_OP_File;
	string Signal_IP_File_Name;
	string Signal_OP_File_Name;

	SectorDataIO_temp();
	virtual ~SectorDataIO_temp();


	void	SetOutputFileName(string name_base, string name_appendix);
};



//-------------------------------------------------------------
//	контейнер-обработчик, так себе
//-------------------------------------------------------------

class	SectorData : public Options, public SectorDataIO_temp
{

public:
	ComplexFunctionMD_F32 focused_data;

protected:
	void	ISignal_Read();
	void	ISignal_Write();
	void	Read_Data();
	bool	Write_Data();
	virtual	void	Append_Data();
public:

	// Display and analyze tools

	enum	FramesCombineAlgorithm
	{
		coherent_add = 0,
		non_coherent_add = 1,
		single_subaperture = 2,
		single_quantile = 3
	};

	struct	FramesCombineParams
	{
		FramesCombineAlgorithm alg;
		size_t	frame;
		size_t	quantile;

		FramesCombineParams()
		{
			alg = coherent_add;
			frame = quantile = 0;
		}
	};


	bool	GetSubaperturesMixParams(FramesCombineParams &p);
	string	GenerateImageTitle(const FramesCombineParams &p);

	void	ComputeDetectedSignal(GrayScanConverter &result, const FramesCombineParams &smp);

	virtual void	ProcessInitDialog();
	SectorData();
	virtual ~SectorData();


//------------------------------------------

	virtual void	Display(const char *);

	physical_angle	CentreAngle();
	physical_angle	CurrentRayAngle(size_t ray);

	void	Work();
	virtual void	Batch();
	virtual void	InitWork();
	virtual void	EndWork();

};


XRAD_END

#endif //__sector_data_h