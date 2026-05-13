//	file LayersContainer.h
//	Created by ACS on 27.08.07
//---------------------------------------------------------------------------
#ifndef __LayersContainer_h
#define __LayersContainer_h
//---------------------------------------------------------------------------

XRAD_BEGIN

class	LayersContainer : public DataArray2D<DataArray<ComplexFunction2D_F32> >
{
	typedef DataArray2D<DataArray<ComplexFunction2D_F32> > inherited;
private:

	complexF32	*base_pointer;
	size_t	nL_ax, nL_lat;
	size_t	n_rays, n_samples;
	void	dispose();

	ComplexFunction2D_F32	mean_values;
	ComplexFunction2D_F32	deviations;
	ComplexFunction2D_F32	correlations;

public:
	LayersContainer():n_rays(0),n_samples(0), base_pointer(NULL){};
	virtual ~LayersContainer();
	void	realloc(size_t, size_t, size_t, size_t);

	void	AnalyzeLayers();
	void	DisplayLayers();
	void	OrderStatistics();

	ComplexFunctionF32	**statistics;
	};

XRAD_END

//---------------------------------------------------------------------------
#endif // __LayersContainer_h
