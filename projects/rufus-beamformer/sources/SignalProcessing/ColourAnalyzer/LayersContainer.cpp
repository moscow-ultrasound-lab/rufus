#include "Pre.h"
#include "LayersContainer.h"

XRAD_BEGIN

void	LayersContainer :: realloc(size_t nl_axial, size_t nl_lateral, size_t nr, size_t ns)
	{
	n_rays = nr;
	n_samples = ns;
	nL_ax = nl_axial;
	nL_lat = nl_lateral;

	size_t	megabyte = 1024*1024;
	size_t	layer_count = n_rays*n_samples;
	size_t	statistics_count = nL_ax*nL_lat;
	size_t	data_count = statistics_count*layer_count;

	if(base_pointer) dispose();

	inherited :: realloc(nL_ax, nL_lat);


	mean_values.realloc(nL_ax, nL_lat);
	deviations.realloc(nL_ax, nL_lat);
	correlations.realloc(nL_ax, nL_lat);

	if(CapsLock())ShowSigned("Data size will be (MBytes)", (data_count*sizeof(complexF32))/megabyte);
	try
		{
		base_pointer = new complexF32[data_count];
		}
	catch(exception &ex)
		{
		ShowString("An error while allocating LayersContainer:", ex.what());
		throw;
		}
	for(size_t i = 0; i < nL_ax; i++)
		{
		for(size_t j = 0; j < nL_lat; j++)
			{
			size_t	offset = layer_count*(i*nL_lat + j);
			at(i,j).UseData(base_pointer + offset, n_rays, n_samples, n_samples, 1);
			}
		}

	statistics = new ComplexFunctionF32* [n_rays];
	for(size_t i = 0; i < n_rays; i++)
		{
		statistics[i] = new ComplexFunctionF32[n_samples];
		for(size_t j = 0; j < n_samples; j++)
			{
			size_t	offset = i*n_samples + j;
			statistics[i][i].UseData(base_pointer + offset, statistics_count, layer_count);
			}
		}
	}

void	LayersContainer :: dispose(){


	mean_values.realloc(0,0);
	deviations.realloc(0,0);
	correlations.realloc(0,0);

	for(size_t i = 0; i < n_rays; i++) DestroyArray(statistics[i]);
	DestroyArray(statistics);
	//DestroyPointer(base_pointer);
	DestroyArray(base_pointer);
	}

LayersContainer :: ~LayersContainer()
	{
	dispose();
	}

void	LayersContainer :: AnalyzeLayers()
	{
	size_t	p2 = nL_ax/2;
	size_t	q2 = nL_lat/2;

	for(size_t p = 0; p < nL_ax; p++)
		{
		for(size_t q = 0; q < nL_lat; q++)
			{
			mean_values.at(p,q) = AverageValue(at(p,q));
			double	sigma = 0;
			for(size_t i = 0; i < n_rays; i++)
				{
				for(size_t j = 0; j < n_samples; j++)
					{
					sigma += cabs2(mean_values.at(p,q) - at(p,q).at(i,j));
					}
				deviations.at(p,q) = sqrt(sigma/(n_rays*n_samples));
				}
			}
		}
	float	normalizer = n_rays*n_samples;
	float	divisor = cabs(MaxValue(at(p2,q2)));
	
	for(size_t i = 0; i < n_rays; i++)
		{
		for(size_t j = 0; j < n_samples; j++)
			{
			complexF32	c0 = at(p2,q2).at(i,j);
			for(size_t p = 0; p < nL_ax; p++)
				{
				for(size_t q = 0; q < nL_lat; q++)
					{
					complexF32	up = (c0%at(p,q).at(i,j))/divisor;
					complexF32	down = (deviations.at(p2,q2)*deviations.at(p,q))/divisor;
					complexF32	increment = up / down;
					correlations.at(p,q) += increment;
//					correlations.at(p,q) += (c0%at(p,q).at(i,j)) / (deviations[p2][q2]*deviations.at(p,q));
					}
				}
			}
		}
	correlations /= normalizer;

	for(size_t p = 0; p < nL_ax; p++)
		{
		for(size_t q = 0; q < nL_lat; q++)
			{
			at(p,q) /= deviations.at(p,q);
			}
		}

	}

void	LayersContainer :: OrderStatistics()
	{
	GUIProgressBar	progress;
	progress.start("Computing ordered layers", n_rays);
	for(size_t i = 0; i < n_rays; i++)
		{
		for(size_t j = 0; j < n_samples; j++)
			{
			reorder_ascent(statistics[i][i]);
			}
		++progress;
		}
//	EndProgress();
	}


void	LayersContainer :: DisplayLayers()
	{
	size_t	answer = 0;
	int	p = 0, q = 0;
	while(answer != 4)
		{
		answer = GetButtonDecision("Layer display options",
//			5,
		{"Layers",
		"Mean values",
		"Deviations",
		"Correlation (center)",
		"Exit"});

		switch(answer)
			{
			case 0:
				if(nL_ax > 1) p = GetSigned("Axial layer to display", p, 0, nL_ax-1);
				else p = 0;
				if(nL_lat > 1) q = GetSigned("Lateral layer to display", q, 0, nL_lat-1);
				else q = 0;
				DisplayMathFunction2D(at(p,q), "Layer");
				break;
			case 1:
				DisplayMathFunction2D(mean_values, "mean values");
				break;
			case 2:
				DisplayMathFunction2D(deviations, "Deviations");
				break;
			case 3:
				DisplayMathFunction2D(correlations, "Correlation with central element");
				break;
			case 4:
				break;
			};
		}
	}

XRAD_END
