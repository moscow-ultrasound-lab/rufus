/*------------------------------------------------------------------*/
/*	Copyright (C)	*/
/*	Advanced Technology Laboratories, 1989.	*/
/*--------------------------------------------------------------------
 File Name:
	SimIO.h
	
Description:
	This include file contains the definitions of the structures 
	and functions which are common to all the simulation programs
	in the end to end sytem simulation package.

Modifications:
	Ver	Date		By				Reason
	--- -------- 	---------- 		------------------------------------------
	1.0 08/04/89 	J. Bono			Original version.
	1.1 08/18/89 	P. Keller		Revised to match document.
	1.2 09/06/89 	P. Keller		Modified for version 0.2 of SimIO
	1.3 09/28/89 	P. Keller		Added defines for RETURN, EXIT and SUPPRESS
	1.4 12/01/89	P. Keller		Added prototype for SimSkipHistory
	
	1.5 02/19/91	B. Robinson 	Add AULONG KeyType
	1.6 02/21/91	B. Robinson		Add prototype for SimCheckSumCheckEnable()
	1.7 02/27/91	B. Robinson		Add prototype for SimPutMuchData()
	1.8 03/28/91	J. Quistgaard	Added old-style function declarations inside
									#ifndef construct for Sun4.
	2.0 04/17/91	J. Quistgaard	New World Standard.  Put in the ifndef _H_SimIO
									construct.
*/

/*------------------------------------------------------------------*/

#ifndef _H_SimIO
#define _H_SimIO

#include <cstdio>
#include <vector>

#ifdef __cplusplus
extern "C"{
#endif

/* Error codes returned by the SimIO functions.	*/
typedef char *SimErrCode;

extern SimErrCode
	BAD_HEADER,	/* Bad record header	*/
	BAD_RECORD_NUMBER,	/* Record number out of sequence	*/
	BAD_PHRASE,	/* Bad keyword phrase	*/
	CANT_CLOSE,	/* Cannot close file	*/
	CANT_OPEN,	/* Cannot open file	*/
	CHECKSUM_ERROR,	/* Checksum error	*/
	END_OF_RECORD,	/* End of record	*/
	INCONSISTENT_HISTORY,	/* Multiple keyword definitions with
	different values	*/
	IO_ERROR,	/* File I/O error	*/
	LEGAL_EOF,	/* Legitimate EOF	*/
	MISSING_DESC_KEY,	/* Missing descriptor keyword	*/
	MISSING_HIST_KEY,	/* Missing history keyword	*/
	MISSING_STD_DESC,	/* Missing standard descriptor keyword	*/
	MISSING_STD_HIST,	/* Missing standard history keyword	*/
	NO_DATA_,	/* No data record	*/
	NO_DESCRIPTOR,	/* No descriptor record	*/
	OUT_OF_MEMORY,	/* Memory allocation failed	*/
	SUCCESS,	/* Normal completion	*/
	TOO_MANY_FILES,	/* Too many files open	*/
	TOO_MUCH_DATA,	/* Data record bigger than buffer	*/
	UNEXPECTED_EOF	/* EOF within a record	*/
	;

/*------------------------------------------------------------------*/
/* Maximum sizes (string sizes include one character for '\0').	*/
#define MAXNAMESIZE	32
#define MAXSTRINGSIZE	256
#define MAXCOMMENTSIZE	256
#define MAXFILES	20

/*------------------------------------------------------------------*/
/* #defines which help make calls to SimError more legible.	*/
#define EXIT	true
#define RETURN	false
#define SUPPRESS	NULL

/*------------------------------------------------------------------*/
// Legal keyword value types.	
typedef enum
	{
	ASTRING,	// String (delimited by "")	
	ADOUBLE,	// Double (double-precision float)	
	AINT,	// Int (16-bit signed integer)	
	ALONG,	// Long (32-bit signed integer)	
	AULONG,	// Unsigned long (32-bit unsigned int)	
	NUM_OF_KEY_TYPES
	}	KeyType;

/*------------------------------------------------------------------*/
/* Parameter structure for keyword lists	*/
typedef struct
	{
	char	*name;	/* Pointer to keyword name	*/
	void	*value;	/* Pointer to keyword value	*/
	char	*comment;	/* Pointer to comment buffer	*/
	KeyType type;	/* Type of keyword value	*/
	int	timesFound;	/* How often keyword was found by	*/
	/* SimGetHistory	*/
	}	param;

/*------------------------------------------------------------------*/
/* Define simulation input / output function prototypes	*/

#ifndef Sun4

/* Opens a simulation input data file.	*/
SimErrCode	SimOpen (
	const char	*name,	/* Name of file to open	*/
	const char	*format,	/* Mode for opening file	*/
	FILE	**filePtr); /* Pointer to stream ptr	*/

/* Closes an open simulation data file.	*/
SimErrCode	SimClose (
	FILE	*filePtr);	/* Stream pointer	*/

/* Gets requested parameters from history portion of file.	*/
SimErrCode	SimGetHistory (
	int	numParams,	/* Number of parameters	*/
	std::vector<param>	&paramList, /* Parm names and addr.	*/
	FILE	*filePtr);	/* Stream pointer	*/

/* Writes a new simulation history record.	*/
SimErrCode	SimPutHistory (
	int	numParams,	/* Number of parameters	*/
	std::vector<param>	&paramList, /* Parm names and addr.	*/
	FILE	*filePtr);	/* Stream pointer	*/

/* Copies history portion of input files to output files.	*/
SimErrCode	SimCopyHistory (
	int	nInFiles,	/* # input streams	*/
	FILE	*inFile[],	/* Input stream ptrs	*/
	int	nOutFiles,	/* # output streams	*/
	FILE	*outFile[]);/* Output stream ptrs	*/

/* Gets parameters of current descriptor record.	*/
SimErrCode	SimGetDescriptor (
	int	numParams,	/* Number of parameters	*/
	std::vector<param>	&paramList, /* Parm names and addr.	*/
	FILE	*filePtr);	/* Stream pointer	*/

/* Writes new descriptor record.	*/
SimErrCode	SimPutDescriptor (
	int	numParams,	/* Number of parameters	*/
	std::vector<param>	&paramList, /* Parm names and addr.	*/
	FILE	*filePtr);	/* Stream pointer	*/
 
/* Skips descriptor record, reads the binary data record.	*/
SimErrCode	SimGetData (
	size_t	maxBytes,	/* Max number of bytes	*/
	void	*bufferPtr, /* Buffer pointer	*/
	FILE	*filePtr);	/* Stream pointer	*/

/* Writes the binary data record.	*/
SimErrCode	SimPutData (
	size_t	nBytes,	/* Number of data bytes	*/
	void	*dataPtr,	/* Pointer to data	*/
	FILE	*filePtr);	/* Stream pointer	*/

/* Writes the binary data record in blocks.	*/
SimErrCode	SimPutMuchData (
	void	*dataPtrArray[],/* Array of pointers to data		*/
	int		nBlocks,		/* Num blocks of data (# pointers)	*/
	int	nBytesPerBlock,		/* size of each array pointed to	*/
	FILE	*filePtr);		/* Stream pointer					*/

/* Skips descriptor record, skips data record if present.	*/
SimErrCode	SimSkipData (
	FILE	*filePtr);	/* Stream pointer	*/

/* Skips to first descriptor record.	*/
SimErrCode	SimSkipHistory (
	FILE	*filePtr);	/* Stream pointer	*/

/* Handles error reporting of a given error type.	*/
void	SimError (
	SimErrCode errorCode,	/* Error code	*/
	const char	*message,	/* Supplemental message	*/
	bool exitFlag,	/* Exit flag: EXIT or RETURN	*/
	FILE	*outPtr);	/* Stream ptr for msg	*/

/* Sets the default error handling method.	*/
void	SimErrorMode (
	bool exitFlag,	/* Exit flag: TRUE or FALSE	*/
	FILE	*outPtr);	/* Stream ptr for msg	*/

/* Produces the standard error message.	*/
SimErrCode	SimStandardError (
	SimErrCode errorCode,/* Error code	*/
	FILE	*filePtr);	/* Ptr to stream with error	*/
	
/*	Sets/resets the local check sum check control flag	*/
void	SimCheckSumCheckEnable (
	bool checksumFlag);	/*	checking on	<-- TRUE	*/

#else

extern SimErrCode		SimOpen (); 
extern SimErrCode		SimClose ();
extern SimErrCode		SimGetHistory ();
extern SimErrCode		SimPutHistory ();
extern SimErrCode		SimCopyHistory ();
extern SimErrCode		SimGetDescriptor ();
extern SimErrCode		SimPutDescriptor ();
extern SimErrCode		SimGetData ();
extern SimErrCode		SimPutData ();
extern SimErrCode		SimPutMuchData ();
extern SimErrCode		SimSkipData ();
extern SimErrCode		SimSkipHistory ();
extern void				SimError ();
extern void				SimErrorMode ();
extern SimErrCode		SimStandardError ();
extern void				SimCheckSumEnable ();

#endif

#ifdef __cplusplus
}
#endif


/* Don't add anything below this line! */
#endif
