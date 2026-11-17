#ifndef AlternativeAnisotropicFilter1D_h__
#define AlternativeAnisotropicFilter1D_h__
/*
created:	14:11:2016   14:34
author:		kns
*/

#include "AnisotropicFilter1D.h"

XRAD_BEGIN

class AlternativeAnisotropicFilter1D : public AnisotropicFilter1D
{
	AlternativeAnisotropicFilter1D();
private:
	double condition_01;
	double condition_02;
	double condition_03;
	double condition_04;
	double step_der;
	double step_diff;
	double large_step;
	virtual void	ComputeClearedSignal();
};
class CrossAnisotropicFilter1D : public AnisotropicFilter1D
{
private:
	virtual void	ComputeClearedSignal();
};
class HyperbolaAnisotropicFilter1D : public AnisotropicFilter1D
{
private:
	virtual void ComputeClearedSignal();
};

XRAD_END

#endif // AlternativeAnisotropicFilter1D_h__
