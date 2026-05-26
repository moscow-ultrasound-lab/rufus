
#if 0
#include "pre.h"

//	эти #include ниже -- не ради включения файлов,
//	а для быстрого их открывания, поскольку в проект их
//	добавлять не следует, а открывать через Open File очень уж
//	неудобно

#include <BlurAlgorithms.cc>

#include "rasp.h"
#include "XRADBasic.h"
#include "MathAcc.h"

#include "DataOwner.h"
#include "DataArray2D.cc"
#include "MathAlgorithms.h"
#include "MathAlgorithms.cc"


#include "MathFunction.h"
//#include "QMFunctors.h"

#include "BasicArraysInteractions.h"
#include "BasicArraysInteractions.cc"
#include "TI_BasicArraysInteractions.h"
#include "TI_BasicArraysInteractions.cc"


#include "RaspMathFunction2D.h"
#include "RaspMathFunction2D.cc"


#include "TI_RaspMathFunction2D.h"
#include "TI_RaspMathFunction2D.cc"
#include "TI_OptimizedFunctions.h"
#include "TI_OptimizedFunctions.cpp"



#include "ImportExportSample.h"
#include "ImportExportSample.cc"

#include "RASPFunctors.h"
#include "RASPBasics.h"
#include "BlurAlgorithms.cc"

#include "RASP_Processor.h"
#include "RASP_Processor.cc"
#include "FixedPoint.h"
#include "TextureProcessor.h"
#include "TextureProcessor.cc"
#include "TextureProcessorSettings.cpp"
#include "RASPMathFunction2D.cc"

#include "ComplexFunction.h"

#include "MathFunction2D.cc"


#include "RaspTableFunction.h"
#include "RaspTableFunction.cc"

#include <limits>


#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "FixedPoint.h"

XRAD_BEGIN

#define CLC_CPP 1
#define CLC_ASM 2
#define CLC_MMX 3

 
#ifdef __BORLANDC__

//const int	ASM_ONE_SHIFT = fixed_point_position(int(0));

//#define	ASSEMBLER_BLUR_ALGORITHM CLC_MMX
//#define	ASSEMBLER_BLUR_ALGORITHM CLC_ASM
#define	ASSEMBLER_BLUR_ALGORITHM CLC_CPP


#else

#define	ASSEMBLER_BLUR_ALGORITHM CLC_CPP

#endif



//--------------------------------------------------------------------------
//
//	вычисление бих-фильтра
//


void	BIXBlur1D_fast(int *base,
	int st, int sst,
	int a0, int a1)
	{

#if ASSEMBLER_BLUR_ALGORITHM==CLC_CPP

	BIXBlur1D_fast_cpp(base, st, sst, a0, a1);

#endif


#if ASSEMBLER_BLUR_ALGORITHM == CLC_MMX

  int ins_st,ins_sst;

  ins_st=4*st;
  ins_sst=4*sst;

  asm {

	MOVD    MM7,a0
	PSLLD   MM7,16
	MOVD    MM1,a1
	POR     MM7,MM1

	mov     ecx,ins_st		// Шаг изменения индекса
	mov     edx,ins_sst

	mov     esi,base

	mov     ebx,esi
	add     ebx,edx		   // &base[sst]

	movzx   eax,WORD PTR [esi]
	add     esi,ecx		   // Указатель на текущее значение base

LP10:
	cmp     esi,ebx
	jge     LP20

	shl     eax,16
	movzx   edi,WORD PTR [esi]
	or	eax,edi

	MOVD    MM0,eax
	PMADDWD MM0,MM7
	MOVD    eax,MM0
	sar     eax,ASM_ONE_SHIFT
	mov     [esi],eax

	add     esi,ecx
	jmp     LP10

LP20:
	mov     esi,base

	mov     ebx,esi

	add     esi,edx
	sub     esi,ecx

	movzx   eax,WORD PTR [esi]
	sub     esi,ecx

LP30:
	cmp     esi,ebx
	jl	LP40

	shl     eax,16
	movzx   edi,WORD PTR [esi]
	or	eax,edi

	MOVD    MM0,eax
	PMADDWD MM0,MM7
	MOVD    eax,MM0
	sar     eax,ASM_ONE_SHIFT
	mov     [esi],eax

	sub     esi,ecx
	jmp     LP30

LP40:
	EMMS
	};

#endif


#if ASSEMBLER_BLUR_ALGORITHM == CLC_ASM

  int ins_st,ins_sst,ins_a1;

  // Эти присвоения потому что mov почему то не работает
  // с переменыыми-параметрами функций.
  ins_st=4*st;
  ins_sst=4*sst;
  ins_a1=a1;

  int mem;

  asm {
	mov   mem,esp
	mov   esp,ins_a1

	mov   ecx,ins_st

	mov   esi,base
	mov   edi,[esi]

	mov   ebx,esi
	add   ebx,edx		   // &base[sst]

	add   esi,ecx

LP10:
	cmp   esi,ebx
	jge   LP20

	mov   eax,[esi]

	sub   eax,edi
	imul  esp
	sar   eax,ASM_ONE_SHIFT
	add   eax,edi

	mov   [esi],eax
	mov   edi,eax

	add   esi,ecx
	jmp   LP10

LP20:
	mov   esi,base

	mov   ebx,esi

	add   esi,ins_sst
	sub   esi,ecx
	mov   edi,[esi]
	sub   esi,ecx

LP30:
	cmp   esi,ebx
	jl    LP40

	mov   eax,[esi]
	sub   eax,edi

	imul  esp
	sar   eax,ASM_ONE_SHIFT
	add   eax,edi
	mov   [esi],eax

	mov   edi,eax

	sub   esi,ecx
	jmp   LP30

LP40:
	mov esp,mem
	};
#endif
	}

XRAD_END

#endif
