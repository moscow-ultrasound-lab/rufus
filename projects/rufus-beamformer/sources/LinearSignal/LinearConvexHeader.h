#ifndef __linear_convex_header_h
#define __linear_convex_header_h

// ‚ерсиЯ 20 марта 1996 года = 2.1

// ‚ерсиЯ 14 декабрЯ 2001 года = 2.2
// ‡амена short на bool длЯ TX/RX_Focusing
// добавлена структура areaHeader

#define currentLSHeaderVersion 2.2

struct	LinearSignalHeader
	{
	double	version;
	string	fileName;
	double	soundSpeed; //cm/sec
// 	double	omega0;
// 	double	halfWidth;
	double	sampleRate;
	double	dZ;
	
 	int	nElements;
	int	nApertElements;
	double	arrayPitch;
	
	bool	TX_Focusing;
	bool	RX_Focusing;
	
	int	n_frames;
	int	nRays, nSamples;

	double	rMin, rMax;
	double	TX_Focus, RX_Focus;
	double	convexRadius;

	void	scan(FILE *textFile);
	void	print(FILE *stream = NULL);
	void	refresh_data();
	};

struct	areaHeader
	{
	string	comment;
	int	firstRay, nRays;
	int	firstSample, nSamples;

	void	scan(FILE *textFile);
	void	print(FILE *stream = NULL);
	void	refresh_data();
	};



#endif //__linear_convex_header_h