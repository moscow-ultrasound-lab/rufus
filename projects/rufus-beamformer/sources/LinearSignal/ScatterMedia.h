#ifndef __scatter_media_h
#define __scatter_media_h

XRAD_BEGIN

class	scatterMedia
	{
	public:
	virtual int	start() = 0;
	virtual void	operator ++() = 0;
	void	operator ++(int){operator++();};
	virtual bool	end() = 0;
	virtual int	nPoints() = 0;
	virtual void	Move(double delay) = 0;


	protected:
		int	n_points, no;
		float	xp, zp;
		complexF32 value;

	public:

	virtual void	Init(double s, double d) = 0;

		const complexF32& operator*() const{return value;};
		float	x() const{return xp;}; //in elements
		float	z() const{return zp;}; //in samples
	};

XRAD_END

#endif //__scatter_media_h
