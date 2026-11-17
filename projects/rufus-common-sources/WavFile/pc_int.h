#ifndef __PC_INTEGERS_H
#define __PC_INTEGERS_H

#if defined(THINK_CPLUS) || defined(__powerc) || defined (__SC__)


#error obsolete

struct	pc_short{
	short	value;
void	operator = (short x);
	operator short();
	};


struct	pc_long{
	long	value;
void	operator = (long x);
	operator long();
	};
#else
#define pc_short short
#define pc_long long
#endif

#endif //__PC_INTEGERS_H
