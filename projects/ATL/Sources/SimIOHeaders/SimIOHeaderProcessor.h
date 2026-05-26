#ifndef __header_writer_h
#define __header_writer_h

#include "SignalProcessing/Options.h"

class	SimIOHeaderProcessor : public Options
	{
	public:
	void	WriteGenericDataDescriptor(FILE *file_ptr_out);
	void	WriteSubapertDataDescriptor(FILE *file_ptr_out);
	void	AppendHistoryRecord(FILE *file_ptr_out,
			const char *input_file_name,
			const char *output_file_name);
	};



/*void	WriteGenericDataDescriptor(FILE *);
void	WriteSubapertDataDescriptor(FILE *);
void	AppendHistoryRecord(FILE *file_ptr_out,
				char *input_file_name,
				char *output_file_name);
*/

#define	SUBAPERT_DATA_SPECIES "Summed subapertures data (complex)"

#endif //__header_writer_h
