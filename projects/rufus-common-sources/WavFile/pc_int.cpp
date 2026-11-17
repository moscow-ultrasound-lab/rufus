#include "pre.h"
#include <XRAD.h>
#include "pc_int.h"

#if defined (__MWERKS__)


#error obsolete

void	pc_short :: operator =(short x)
	{
	*((char *)&value) = *(((char *)&x)+1);
	*(((char *)&value)+1) = *((char *)&x);
	}

pc_short :: operator short()
	{
	short	buffer;

	*((char *)&buffer) = *(((char *)&value)+1);
	*(((char *)&buffer)+1) = *((char *)&value);
	return buffer;
	};


void	pc_long :: operator =(long x)
	{
	*(((char *)&value) + 0) = *(((char *)&x)+3);
	*(((char *)&value) + 1) = *(((char *)&x)+2);
	*(((char *)&value) + 2) = *(((char *)&x)+1);
	*(((char *)&value) + 3) = *(((char *)&x)+0);
	}

pc_long :: operator long()
	{
	long	buffer;

	*(((char *)&buffer) + 0) = *(((char *)&value)+3);
	*(((char *)&buffer) + 1) = *(((char *)&value)+2);
	*(((char *)&buffer) + 2) = *(((char *)&value)+1);
	*(((char *)&buffer) + 3) = *(((char *)&value)+0);
	return buffer;
	}

#endif
