#ifndef __eigen_vector_tools
#define __eigen_vector_tools

#include <XRADBasic/MathMatrixTypes.h>
#include <XRADBasic/MathFunctionTypes2D.h>



XRAD_BEGIN

//	для симметричных и несимметричных действительных матриц
bool	eigenvectors_non_symmetrical(const RealMatrixF32 &m, ComplexMatrixF32 &v_left, ComplexMatrixF32 &v_right, ComplexVectorF32 &eig_val);
bool	eigenvectors_non_symmetrical_c(const ComplexMatrixF32 &m, ComplexMatrixF32 &v_left, ComplexMatrixF32 &v_right, ComplexVectorF32 &eig_val);

//	для эрмитовой матрицы
bool	eigenvectors_hermit(const ComplexMatrixF32 &m, ComplexMatrixF32 &vec, RealVectorF32 &eig_val);


XRAD_END

#endif //__eigen_vector_tools
