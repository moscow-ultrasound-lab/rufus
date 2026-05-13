
/*
struct tagDryDataInfo_18_may{
	UINT uProbeID; 
	UINT uProbeuID;
	UINT uDepthFrom; //Начальная глубина сканирования (мм).
	UINT uDepthTo; //Глубина сканирования (мм).
	enum EType
	{
		eConvex=2,
		eLinear=4
	};
	
	
	EType   Type;
	UINT uNumOfBeams;
	UINT uLineSize;
	float	Angle;		//Угол раскрыва датчика [radians] Convex only. Угол Стиринга для линейки [radians]
	float	Radius;		//Радиус внешней поверхности [mm] Convex only
	float	Width;		//Ширина линейки [mm]			  Linear only
	float Frequency;

private:
	void	*dummy_sizeof_tester()
		{
		void	*p = sizeof(EType) - 4;
		return p;
		}

};
*/
struct tagDryDataInfo {
	UINT uProbeID; 
	UINT uProbeuID;
	UINT uDepthFrom; //Начальная глубина сканирования (мм).
	UINT uDepthTo; //Глубина сканирования (мм).
	enum EType
	{
		eConvex=2,
		eLinear=4
	};
	
	
	EType   Type;
	UINT uNumOfBeams;
	UINT uLineSize;
	float	Angle;		//Угол раскрыва датчика [radians] Convex only. Угол Стиринга для линейки [radians]
	float	Radius;		//Радиус внешней поверхности [mm] Convex only
	float	Width;		//Ширина линейки [mm]			  Linear only
	float   Frequency;
	float	FocusDepth;

private:
	void	*dummy_sizeof_tester()
		{
		void	*p = sizeof(EType) - 4;
		return p;
		}

};


