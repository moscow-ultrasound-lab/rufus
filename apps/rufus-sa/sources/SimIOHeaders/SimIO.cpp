

/*------------------------------------------------------------------*/
/*							Copyright (C)							*/
/*				Advanced Technology Laboratories, 1989.				*/
/*--------------------------------------------------------------------
 File Name:
	SimIO.c

 Description:
	File access primitives for ATL simulation system.


 Modifications:
	Ver	  Date		By			Reason
	---	-------- ----------	------------------------------------------
	0.1	08/18/89 P. Keller	Stubs for compilation
	0.2	09/05/89 P. Keller	First semi-functional version
	2.1	09/27/89 P. Keller	Functional with following exceptions:
							no checksum; CopyHistory does not merge
							history; GetData does not swap bytes.
							Changed version number to match manual.
	2.2	09/28/89 P. Keller	Updated to work with Think C 4.0
	2.3	09/28/89 J. Bono/	Fix record number in SimStandardError.
			 P. Keller	Preset RecNum in SimOpen and increment it
					in SimGetHeader.
					Allow SimSkipData to skip history by
					reading first header if needed.
					Augment list of white space characters.

	2.4	10/19/89 P. Keller	Open files in binary mode on IBM PC
					Update record number when rewinding or
					fseeking the file
					Write out name of parameters in error
					Flush stdout and stderr before error msg

	2.5	10/20/89 P. Keller	Add checksumming

	2.6	10/25/89 P. Keller	Save and restore file position.  We ensure
					that the file is always positioned at the
					beginning of a record when leaving the
					SimIO routines.  Modified SimSkipData to
					not return LEGAL_EOF.
					Fixed bug in SimPutDescriptor: it wrote
					HIST headers rather than DESC records.
					Reinstated prototypes for internal
					functions in IBM PC version.  Does not
					compile with the -Za (ANSI) option (?).

	2.7	11/29/89 P. Keller	Put '\r' into list of white space chars;
				output '\r' rather than '\n' in history
				and descriptor records -- for binary files.
	2.8	12/01/89 P. Keller	Added SimSkipHistory.



	2.9	 02/19/91 B. Robinson	Modify for new AULONG KeyType as appropriate.
				Place a Pause() before exit(1), since AD lib
				doesn't allow chance to see message.
				Add \a in error message.
				Add SimCheckSumCheckEnable().

	2.10 02/27/91 B. Robinson	add SimPutMuchData() to write out in blocks.
				Because the size_t on the PC is
				unsigned int, change things that use
				dataLen, (ie: SimPutHeader & the SimFile
				structure) to use long.   This makes, for
				eg, SimPutHeader write out the correct
				datalen in the header when dataLen > 2**16 &
				SimPutMuchData() is used to output lotsa data

	2.11 03/06/91 B. Robinson	modify the scanf formats in SimGetKeyword
				to remove the    %*[ \n\r\t]   preceeding
				the comment field read.   For some reason
				this makes the comment field read work on
				the MAC, but may cause problems on other
				processors ??.

	2.12 03/07/91 B. Robinson 	Change type of DataCount from size_t to long.
								This is because its KeyType is ALONG, and
								when read in by SimGetKeyword (for example
								in a SimGetDescriptor), might overwrite the
								buffer (ie: when size_t != long)

	2.13 04/01/91 J. Quistgaard	Port to Sun4 and Stardent.  Moved all pre-
				processor directives to column 0, removed
				variables that were neither set nor used, put
				in conditional typecasting for arguments to
				fread() and fwrite(), limited checksums to 16
				bits for non Mac/PC's, put in consistent typecasting
				for arguments to SimCheckDisk and SimCheckRam, put
				in conditional inclusion of old-style function
				declarations for Sun4, changed min() references to
				_MIN().  You need to use "generic.h" version 1.16
				or later for this to work properly.


1/Jul/91	For my own private verision, I put a ADlib Pause() in the
			exit routine.			B Robinson.

																	*/

/*------------------------------------------------------------------*/
/* Include files:													*/

#include "pre.h"

#include "SimIO.h"			/* Simulation I/O						*/
#include "SimIOHeaders/SimIOHeaders.h"
/*------------------------------------------------------------------*/
/* Local #defines:													*/
/* Special characters												*/

#pragma warning (disable: 4081 4389 4018 4800 4996)
#pragma 

#ifdef __MWERKS__
#pragma optimization_level 0
#else
#pragma message("It is strongly recommended to compile this file without any optimizations. KNS")
#endif

XRAD_USING

#define RECORD_HEADER_KEY	'$'
#define	PHRASE_END_KEY		'`'
#define	STD_KEYWORD_KEY		'.'

#define	SHORT_HEADER_LENGTH	6
#define	LONG_HEADER_LENGTH	22

/*------------------------------------------------------------------*/
/* Global static variables:											*/
/* Error codes returned by the SimIO functions.						*/
SimErrCode
BAD_HEADER =		"Bad record header",
BAD_RECORD_NUMBER =	"Record number out of sequence",
BAD_PHRASE =		"Bad keyword phrase",
CANT_CLOSE =		"Cannot close file",
CANT_OPEN =			"Cannot open file",
CHECKSUM_ERROR =	"Checksum error",
END_OF_RECORD =		"End of record",
INCONSISTENT_HISTORY =
"Multiple keyword definitions with different values",
IO_ERROR =			"File I/O error",
LEGAL_EOF =			"Legitimate EOF",
MISSING_DESC_KEY =	"Missing descriptor keyword",
MISSING_HIST_KEY =	"Missing history keyword",
MISSING_STD_DESC =	"Missing standard descriptor keyword",
MISSING_STD_HIST =	"Missing standard history keyword",
NO_DATA_ =			"No data record",
NO_DESCRIPTOR =		"No descriptor record",
OUT_OF_MEMORY =		"Memory allocation failed",
SUCCESS =			"Normal completion",
TOO_MANY_FILES =	"Too many files open",
TOO_MUCH_DATA =		"Data record bigger than buffer",
UNEXPECTED_EOF =	"EOF within a record"
;

/*------------------------------------------------------------------*/
/* Local static variables:											*/
/* Record types.													*/
typedef enum{
	HISTORY,
	DESCRIPTOR,
	DATA
} RecType;

static char	*RecordTypeName[] ={
	"HIST",
	"DESC",
	"DATA"
};

/* List of open files.												*/
struct	SimFile{
	char	name[256];			/* Pointer to file name					*/
	unsigned recNum;		/* Current record number				*/
	size_t	headLen;		/* Length of record header				*/
	int	dataLen;		/* Length of record	data				*/
	unsigned checksum;		/* Checksum								*/
	RecType	recType;		/* Record type							*/
	int	recOffset;		/* Offset to beginning of record		*/

	SimFile();
};

SimFile::SimFile()
{
	*name = 0; // empty C string
	recNum = 0;
	headLen = 0;
	dataLen = 0;
	checksum = 0;
	recType = DATA;
}

class	SimFilesList
{
	int	maxfiles;
	int	numfiles;
	SimFile	*itsSimFiles;
	FILE	**Files;
public:
	int	count() const{
		return numfiles;
	};
	SimFilesList(int n);
	~SimFilesList();
	void	AddFile(FILE *);
	void	RemoveFile(FILE *);
	SimFile &FindSimFile(FILE *);

	SimFile	&operator[](int);
};

SimFilesList::SimFilesList(int n)
{
	maxfiles = n;
	numfiles = 0;
	itsSimFiles = new SimFile[maxfiles];
	Files = new FILE*[maxfiles];
}

SimFilesList :: ~SimFilesList()
{
	DestroyArray(itsSimFiles);
	DestroyArray(Files);
	maxfiles = 0;
}
SimFile	dummy_sim_file;

void	SimFilesList::AddFile(FILE *f)
{
	if(numfiles > maxfiles-1) FatalError("Too many files");
	Files[numfiles] = f;
	numfiles++;
}

void	SimFilesList::RemoveFile(FILE *f)
{
	int	i = 0;
	while(i < numfiles && fileno(Files[i]) != fileno(f)) i++;
	if(i < numfiles)
	{
		for(; i < numfiles && i < maxfiles-1; i++)
		{
			Files[i] = Files[i+1];
			itsSimFiles[i] = itsSimFiles[i+1];
		}
		itsSimFiles[i] = SimFile();
		Files[i] = 0;
	}
	numfiles--;
}

SimFile	&SimFilesList :: operator[](int n)
{
// получает в качестве аргумента юниксовый fileno(FILE *), находит, где в списке этот файл и возвращает его SimFile
	int	i = 0;
	while(i < numfiles && fileno(Files[i]) != n) i++;
	if(i <= maxfiles) if(Files[i]) return itsSimFiles[i];
	Error("Invalid SimFile");
	return	dummy_sim_file;
}

SimFilesList	SimFiles(MAXFILES);




/* DefaultErrMode error mode.												*/
static struct
{
	int		exitFlag;		/* Flag: EXIT or RETURN					*/
	FILE	*outPtr;		/* Stream ptr for msg					*/
}		DefaultErrMode ={true, stderr};

/* Standard history keywords.										*/
static char	ProcessName[MAXSTRINGSIZE];
static char	ProcessDescription[MAXSTRINGSIZE];
static double ProcessVersion;
static char	ProcessDateTime[MAXSTRINGSIZE];

static char	ProcessNameComment[MAXCOMMENTSIZE];
static char	ProcessDescriptionComment[MAXCOMMENTSIZE];
static char	ProcessVersionComment[MAXCOMMENTSIZE];
static char	ProcessDateTimeComment[MAXCOMMENTSIZE];

static param stdHistoryParams[] =
{
{"ProcessName",			ProcessName,		ProcessNameComment,			ASTRING,	0},
{"ProcessDescription",	ProcessDescription,	ProcessDescriptionComment,	ASTRING,	0},
{"ProcessVersion",		&ProcessVersion,	ProcessVersionComment,		ADOUBLE,	0},
{"ProcessDateTime",		ProcessDateTime,	ProcessDateTimeComment,		ASTRING,	0}
};

#define	nStdHistoryParams	(sizeof(stdHistoryParams)/sizeof(param))

/* Standard descriptor keywords.									*/
static char	ProcessorType[MAXSTRINGSIZE];
static char	DataType[MAXSTRINGSIZE];
static int DataCount;

static char	ProcessorTypeComment[MAXCOMMENTSIZE];
static char	DataTypeComment[MAXCOMMENTSIZE];
static char	DataCountComment[MAXCOMMENTSIZE];

static param stdDescriptorParams[] =
{
{"ProcessorType",		ProcessorType,		ProcessorTypeComment,		ASTRING,	0},
{"DataType",			DataType,			DataTypeComment,			ASTRING,	0},
{"DataCount",			&DataCount,			DataCountComment,			ALONG,		0}
};

#define	nStdDescriptorParams	(sizeof(stdDescriptorParams)/sizeof(param))

/* Flag to control checking of the checksum							*/
static bool	CheckSumCheckEnable = true;		/* default is on	*/

#ifndef Sun4
/*------------------------------------------------------------------*/
/* Prototypes for internal functions:								*/
static void		Copy(
	void	*source,		/* Pointer to source					*/
	void	*destination,	/* Pointer to destination				*/
	KeyType	type);			/* Keyword type							*/

static bool		Equal(
	void	*value1,		/* Pointer to first value				*/
	void	*value2,		/* Pointer to second value				*/
	KeyType	type);			/* Keyword type							*/

static SimErrCode	SimGetHeader(
	FILE	*filePtr);		/* Stream pointer						*/

static SimErrCode	SimPutHeader(
	unsigned checksum,		/* Checksum								*/
	int	dataLen,		/* Length of record data				*/
	RecType	recType,		/* Record type							*/
	FILE	*filePtr);		/* Stream pointer						*/

static void		SimNextRecord(
	FILE	*filePtr);		/* Stream pointer						*/

static SimErrCode SimGetKeywordName(
	char	*name,			/* Pointer to name buffer				*/
	FILE	*filePtr);		/* Stream pointer						*/

static SimErrCode SimGetKeyword(
	param	*keyword,		/* Pointer to parameter structure		*/
	FILE	*filePtr);		/* Stream pointer						*/

static SimErrCode SimSkipKeyword(
	FILE	*filePtr);		/* Stream pointer						*/

static SimErrCode SimParmError(
	SimErrCode errorCode,	/* Error code							*/
	char	*parmName,		/* Parameter name						*/
	FILE	*filePtr);		/* Stream pointer						*/

static SimErrCode SimCheckDisk(
	int	offset,			/* Starting offset of file section		*/
	int	length,			/* Length in bytes of file section		*/
	unsigned *checksum,		/* Pointer to checksum					*/
	FILE	*filePtr);		/* Stream pointer						*/

static SimErrCode SimCheckRAM(
	void	*bufPtr,		/* Pointer to buffer					*/
	int	length,			/* Length in bytes of buffer			*/
	unsigned *checksum);	/* Pointer to checksum					*/


#else

static void			Copy();
static bool		Equal();
static SimErrCode	SimGetHeader();
static SimErrCode	SimPutHeader();
static void			SimNextRecord();
static SimErrCode 	SimGetKeywordName();
static SimErrCode 	SimGetKeyword();
static SimErrCode 	SimSkipKeyword();
static SimErrCode 	SimParmError();
static SimErrCode 	SimCheckDisk();
static SimErrCode 	SimCheckRAM();

#endif


/*********************************************************************
 Function:
	Copy

 Description:
	Internal function to copy any of the legal types from one buffer
	to another.

 Returns:

 Notes:
	No copy if either of the pointers are NULL.
																	*/
static void Copy(
	void	*source,		/* Pointer to source					*/
	void	*destination,	/* Pointer to destination				*/
	KeyType	type)			/* Keyword type							*/
{
	if(source != NULL && destination != NULL)
		switch(type)
		{
			case ASTRING:
				strcpy((char *)destination, (char *)source);
				break;
			case ADOUBLE:
				*(double *)destination = *(double *)source;
				break;
			case AINT:
				*(int *)destination = *(int *)source;
				break;
			case ALONG:
				*(long *)destination = *(long *)source;
				break;
			case AULONG:
				*(unsigned long *)destination = *(unsigned long *)source;
				break;
		}
}

/*--------------------------------------------------------------------
 Function:
	Equal

 Description:
	Internal function to compare any of the legal types.

 Returns:
	true if equal, false otherwise.

 Notes:
	Returns false if either pointer is NULL.
																	*/
static bool Equal(
	void	*value1,		/* Pointer to first value				*/
	void	*value2,		/* Pointer to second value				*/
	KeyType	type)		/* Keyword type							*/
{
	int		equal(0);			/* Flag for equality					*/

	/* Fail the comparison if either of the pointers are NULL.		*/
	if(value1 == NULL || value2 == NULL)
		return (false);

	/* Compare the various types in their individual ways.			*/
	switch(type)
	{
		case ASTRING:
			equal = (strcmp((char *)value1, (char *)value2) == 0);
			break;
		case ADOUBLE:
			equal = (*(double *)value1 == *(double *)value2);
			break;
		case AINT:
			equal = (*(int *)value1 == *(int *)value2);
			break;
		case ALONG:
			equal = (*(long *)value1 == *(long *)value2);
			break;
		case AULONG:
			equal = (*(unsigned long *)value1 == *(unsigned long *)value2);
			break;
	}

/* Return the appropriate bool value.						*/
	if(equal)
		return (true);
	else
		return (false);
}

/*--------------------------------------------------------------------
 Function:
	SimCheckDisk

 Description:
	Calculates a checksum for a section of a file.

 Returns:
	SUCCESS, IO_ERROR, UNEXPECTED_EOF

 Notes:
	Leaves the file pointer in its current location.
																	*/
static SimErrCode SimCheckDisk(
	int	offset,		/* Starting offset of file section		*/
	int	length,			/* Length in bytes of file section		*/
	unsigned *checksum,		/* Pointer to checksum					*/
	FILE	*filePtr)		/* Stream pointer						*/
{
	int		c;				/* A byte read from file				*/
	int	i;				/* Loop counter							*/
	int	curPos = ftell(filePtr);
							/* Save the current file position		*/

	/* Initialization.												*/
	*checksum = 0;
	if(fseek(filePtr, offset, SEEK_SET))
		return (IO_ERROR);

	/* Loop through summation.										*/
	for(i = 0; i < length; i++)
	{
		if((c = fgetc(filePtr)) == EOF)
			if(feof(filePtr))
				return (UNEXPECTED_EOF);
			else
				return (IO_ERROR);

		*checksum += (unsigned)c;
	}

	*checksum &= 0xffff;	/* Limit it to 16 bits for non-Mac/PC	*/

	/* Clean up.													*/
	fseek(filePtr, curPos, SEEK_SET);
	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimCheckRAM

 Description:
	Calculates a checksum for a section of memory.

 Returns:
	SUCCESS

 Notes:
																	*/
static SimErrCode SimCheckRAM(
	void	*bufPtr,		/* Pointer to buffer					*/
	int	length,			/* Length in bytes of buffer			*/
	unsigned *checksum)		/* Pointer to checksum					*/
{
	unsigned cSum = 0;
							/* Running checksum						*/
	unsigned char *p;
							/* Running buffer pointer				*/
	unsigned char *pStop = (unsigned char *)bufPtr + length;
							/* Ending buffer pointer				*/

	/* Loop through summation.										*/
	for(p = (uint8_t *)bufPtr; p < pStop; p++)
		cSum += *p;

	/* Limit checksum to 16 bits for non-Mac/PC 						*/

	cSum &= 0xffff;

	/* Clean up.													*/
	*checksum = cSum;
	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimGetHeader

 Description:
	Internal function to read the header of the next record.

 Returns:
	SUCCESS, LEGAL_EOF, BAD_HEADER, BAD_RECORD_NUMBER

 Notes:
	Assumes the file pointer is at the beginning of the record header,
	and leaves it at the beginning of the record data if it does not
	encounter an error.
	For short-form headers, the record length is determined by
	scanning the record data for the next "$$".
	If all goes well, the open-file list is updated and the record
	number is checked.
	For short form headers, the check sum is always calculated (to
	update the open file list).
	For long form headers, provided CheckSumCheckEnable is true,
	the check sum contained in the header is checked against the
	calculated data check sum,
																	*/
static SimErrCode SimGetHeader(
	FILE	*filePtr)		/* Stream pointer						*/
{
	int		c;				/* Single character						*/
	char	type[5];		/* ASCII type of this record (incl '\0')*/

	unsigned recNum;		/* Record number						*/
	int	dataLen;		/* Length of record data				*/
	unsigned checksum;		/* Checksum as reported in header		*/
	RecType	recType;		/* Record type							*/
	SimErrCode status;		/* Status returned by SimCheckDisk		*/

	/* Save the current position, on the assumption that this is	*/
	/* the beginning of the record.									*/
	SimFiles[fileno(filePtr)].recOffset = ftell(filePtr);

	/* Check for a legitimate EOF by reading the first character.	*/
	clearerr(filePtr);
	c = fgetc(filePtr);
	if(feof(filePtr))
		return (LEGAL_EOF);
	ungetc(c, filePtr);

	/* Short record header.											*/
	if(SimFiles[fileno(filePtr)].headLen == SHORT_HEADER_LENGTH)
	{
	/* Read the record type.									*/
		if(fscanf(filePtr, "$$ %4s", type) != 1)
			return (BAD_HEADER);

		/* Determine the record length by scanning for "$$".		*/
		{
			int	c1, c2;			/* Two consecutive characters			*/
			for(dataLen = 0, c1 = fgetc(filePtr), c2 = fgetc(filePtr);
				(c2 != EOF) &&
				((c1 != RECORD_HEADER_KEY) || (c2 != RECORD_HEADER_KEY));
				dataLen++, c1 = c2, c2 = fgetc(filePtr))
				;
			if(c1 != EOF && c2 == EOF) dataLen++;
		}

		/* Calculate the checksum.									*/
		if(SUCCESS != (status =
		   SimCheckDisk(SimFiles[fileno(filePtr)].recOffset +
		   (int)SimFiles[fileno(filePtr)].headLen, dataLen,
		   &checksum, filePtr)))
			return (status);
	}

/* Long record header.											*/
	else
	{
		unsigned myChecksum;/* Checksum as computed by us			*/

		/* Read the record header.									*/
		if(fscanf(filePtr, "$ %4x %4x %8lx $ %4s",
		   &recNum, &checksum, &dataLen, type) != 4)
			return (BAD_HEADER);

		/* Check the record number.									*/
		if(recNum != (SimFiles[fileno(filePtr)].recNum + 1))
			return (BAD_RECORD_NUMBER);

		/* Check the checksum if requested to do so					*/
		if(CheckSumCheckEnable)
		{
			if(SUCCESS != (status =
			   SimCheckDisk(SimFiles[fileno(filePtr)].recOffset +
			   (int)SimFiles[fileno(filePtr)].headLen, dataLen,
			   &myChecksum, filePtr)))
				return (status);
			if(checksum != myChecksum)
				return (CHECKSUM_ERROR);
		}
	}

/* Reset the file pointer to beginning of record data.			*/
	fseek(filePtr, SimFiles[fileno(filePtr)].recOffset +
		(int)SimFiles[fileno(filePtr)].headLen, SEEK_SET);

	/* Identify the type of record.									*/
	if(strcmp(type, RecordTypeName[HISTORY]) == 0)
		recType = HISTORY;
	else if(strcmp(type, RecordTypeName[DESCRIPTOR]) == 0)
		recType = DESCRIPTOR;
	else if(strcmp(type, RecordTypeName[DATA]) == 0)
		recType = DATA;
	else
		return (BAD_HEADER);

	/* Update the open-file list.									*/
	SimFiles[fileno(filePtr)].recNum++;
	SimFiles[fileno(filePtr)].dataLen = dataLen;
	SimFiles[fileno(filePtr)].checksum = checksum;
	SimFiles[fileno(filePtr)].recType = recType;

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimPutHeader

 Description:
	Internal function to write a record header.

 Returns:
	SUCCESS, UNEXPECTED_EOF, IO_ERROR

 Notes:
	Assumes the file pointer is at the beginning of the record header;
	and leaves it at the beginning of the record data if it does not
	encounter an error.
	Updates the open-file list.
	You may wish to call SimWriteHeader twice, especially when writing
	parameter records: the first time to write a dummy header, and the
	second time, when the record length and checksum are known, to
	write the real header.
																	*/
static SimErrCode SimPutHeader(
	unsigned checksum,		/* Checksum								*/
	int	dataLen,		/* Length of record						*/
	RecType	recType,		/* Record type							*/
	FILE	*filePtr)		/* Stream pointer						*/
{
/* Save the current position in the file.						*/
	SimFiles[fileno(filePtr)].recOffset = ftell(filePtr);

	/* Increment the record number.									*/
	SimFiles[fileno(filePtr)].recNum++;

	/* Write the header; analyze and report any errors.				*/
	if(fprintf(filePtr, "%c%04X%04X%08lX%c%4s",
	   RECORD_HEADER_KEY, SimFiles[fileno(filePtr)].recNum,
	   checksum, dataLen, RECORD_HEADER_KEY, RecordTypeName[recType])
	   == EOF)
		if(feof(filePtr))
			return (UNEXPECTED_EOF);
		else
			return (IO_ERROR);

	/* Update the rest of the open-file list.						*/
	SimFiles[fileno(filePtr)].dataLen = dataLen;
	SimFiles[fileno(filePtr)].checksum = checksum;
	SimFiles[fileno(filePtr)].recType = recType;

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimNextRecord

 Description:
	Internal function to advance the file pointer to the next record.

 Returns:

 Notes:
	Assumes the file is positioned at the beginning of the record
	data, and uses the information in the open-file list; so be sure
	to call SimGetHeader before SimNextRecord.
																	*/
static void SimNextRecord(
	FILE	*filePtr)		/* Stream pointer						*/
{
	fseek(filePtr, SimFiles[fileno(filePtr)].recOffset +
		(int)SimFiles[fileno(filePtr)].headLen +
		  SimFiles[fileno(filePtr)].dataLen, SEEK_SET);
}

/*--------------------------------------------------------------------
 Function:
	SimGetKeywordName

 Description:
	Internal function to retrieve the name of the next keyword.

 Returns:
	SUCCESS, END_OF_RECORD, BAD_PHRASE, LEGAL_EOF, IO_ERROR

 Notes:
	Assumes the file pointer is positioned at the beginning of the
	keyword phrase.  Leaves the file pointer after the keyword name
	and before the value.
																	*/
static SimErrCode SimGetKeywordName(
	char	*name,			/* Pointer to name buffer				*/
	FILE	*filePtr)		/* Stream pointer						*/
{
	int		nRead;			/* Number of items read					*/

	/* Skip any white space.										*/
	fscanf(filePtr, "%*[ \r\n\t\f]");

	/* Check whether we are at the end of the record.				*/
	if((ftell(filePtr) - SimFiles[fileno(filePtr)].recOffset) ==
	   SimFiles[fileno(filePtr)].headLen + SimFiles[fileno(filePtr)].dataLen)
		return (END_OF_RECORD);

	/* Get the keyword name.										*/
	nRead = fscanf(filePtr, "%s", name);

	/* If we scanned past the end of the record, there's a problem.	*/
	if((ftell(filePtr) - SimFiles[fileno(filePtr)].recOffset) >
	   SimFiles[fileno(filePtr)].headLen + SimFiles[fileno(filePtr)].dataLen)
		return (BAD_PHRASE);

	/* Report EOF and other I/O errors.								*/
	if(nRead == 0)
		if(feof(filePtr))
			return (LEGAL_EOF);
		else
			return (IO_ERROR);

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimGetKeyword

 Description:
	Internal function to retrieve the value and comments of the next
	keyword.

 Returns:
	SUCCESS, UNEXPECTED_EOF, BAD_PHRASE

 Notes:
	Assumes the file pointer is positioned after the keyword name; so
	call SimGetKeywordName first. Leaves the file pointer in place so
	that it can be called more than once; use SimSkipKeyword to move
	to the end of the keyword phrase.
																	*/
SimErrCode SimGetKeyword(
	param	*keyword,		// Pointer to parameter structure
	FILE	*filePtr)		// Stream pointer
{
// All possible formats:
// formats[ASTRING]	[0 = value NULL]	 [0 = comment NULL]
//		  [ADOUBLE]	[1 = value not NULL] [1 = comment not NULL]
//		  [AINT]
//		  [ALONG]
//		  [AULONG]

	bool	cvtValue(false); 	// Flag to read value: 0 or 1
	bool	cvtComment(false);	// Flag to read comment: 0 or 1

	static char *formats[NUM_OF_KEY_TYPES][2][2] =		// 	%*[ \n\r\t]   removed
	{
	// String, read nothing
	"%*[ \n\r\t] \" %*[^\"`] \" %*[^`] `",
	// String, read comment
	"%*[ \n\r\t] \" %*[^\"`] \" %[^`] `",
	// String, read value
	"%*[ \n\r\t] \" %[^\"`] \" %*[^`] `",
	// String, read value and comment
	"%*[ \n\r\t] \" %[^\"`] \" %[^`] `",

	// Double, read nothing
	"%*lf %*[^`] `",
	// Double, read comment
	"%*lf %[^`] `",
	// Double, read value
	"%lf %*[^`] `",
	// Double, read value and comment
	"%lf %[^`] `",

	// Int, read nothing
	"%*d %*[^`] `",
	// Int, read comment
	"%*d %[^`] `",
	// Int, read value
	"%d %*[^`] `",
	// Int, read value and comment
	"%d %[^`] `",

	// Long, read nothing
	"%*ld %*[^`] `",
	// Long, read comment
	"%*ld %[^`] `",
	// Long, read value
	"%ld %*[^`] `",
	// Long, read value and comment
	"%ld %[^`] `",

	// Unsigned Long, read nothing
	"%*lu %*[^`] `",
	// Unsigned Long, read comment
	"%*lu %[^`] `",
	// Unsigned Long, read value
	"%lu %*[^`] `",
	// Unsigned Long, read value and comment
	"%lu %[^`] `"
	};



	int	curPos;			// Current file position
	int	nRead;			// Number of items read
	union
	{
		char	astring[MAXSTRINGSIZE];
		double	adouble;
		int	aint;
		long	along;
		unsigned long	aulong;
	}		oldValue;	// Previously stored value

// Remember the current position so we can return here.
	curPos = ftell(filePtr);

	// Save the old value if it was already found.
	if(keyword->timesFound > 0)
		Copy(keyword->value, (void *)(&oldValue), keyword->type);

	// Check the parameter entry for null pointers.
	cvtValue = (keyword->value != NULL) ? true:false; 	// Flag to read value: 0 or 1
	cvtComment = (keyword->comment != NULL) ? true:false;	// Flag to read comment: 0 or 1


	// Read the input with the appropriate format and pointers.
	if(cvtValue && cvtComment)
	{
		nRead = fscanf(filePtr,
					   formats[keyword->type][cvtValue][cvtComment],
					   keyword->value, keyword->comment);
	}
	else if(cvtValue)
	{
	// 
		if(keyword->value == SIMIO::string_void) nRead = 4;
		else nRead = fscanf(filePtr,
							formats[keyword->type][cvtValue][cvtComment],
							keyword->value);
	}
	else if(cvtComment)
	{
		nRead = fscanf(filePtr,
					   formats[keyword->type][cvtValue][cvtComment],
					   keyword->comment);
	}
	else
	{
		nRead = fscanf(filePtr,
					   formats[keyword->type][cvtValue][cvtComment],
					   keyword->value, keyword->comment);
	}

// If we scanned past the end of the record, there's a problem.
	if((ftell(filePtr) - SimFiles[fileno(filePtr)].recOffset) >
	   SimFiles[fileno(filePtr)].headLen + SimFiles[fileno(filePtr)].dataLen)
		return (BAD_PHRASE);

	// Check for EOF and other scanf errors.
	if(cvtValue && nRead < 1)
		if(feof(filePtr))
			return (UNEXPECTED_EOF);
		else
			return (BAD_PHRASE);

	// If there was no comment, set it to null.
	if((cvtValue && cvtComment && nRead < 2) ||
		(!cvtValue && cvtComment && nRead < 1))
		keyword->comment[0] = '\0';

	// Reset the position to where we started.
	fseek(filePtr, curPos, SEEK_SET);

	// If there are multiple definitions of the same keyword, make
	// sure that their values agree.
	if(keyword->timesFound > 0 &&
	   !Equal(keyword->value, (void *)(&oldValue), keyword->type))
		return (INCONSISTENT_HISTORY);

	// Increment the nTimesFound field.
	keyword->timesFound++;
	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimSkipKeyword

 Description:
	Internal function to skip to the end of the current keyword phrase.

 Returns:
	SUCCESS, UNEXPECTED_EOF, IO_ERROR

 Notes:
	Leaves the file pointer after the keyword delimiter ("`").
																	*/
static SimErrCode SimSkipKeyword(
	FILE	*filePtr)		/* Stream pointer						*/
{
	int		c;				/* Character read						*/

	/* Loop to flush until the next phrase delimiter.				*/
	while((c = fgetc(filePtr)) != EOF && c != PHRASE_END_KEY)
		;

	/* If we read past the end of the record, there's a problem.	*/
	if((ftell(filePtr) - SimFiles[fileno(filePtr)].recOffset) >
	   SimFiles[fileno(filePtr)].headLen + SimFiles[fileno(filePtr)].dataLen)
		return (BAD_PHRASE);

	/* Check for unexpected EOF or file read error.					*/
	if(c == EOF)
		if(feof(filePtr))
			return (UNEXPECTED_EOF);
		else
			return (IO_ERROR);

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimParmError

 Description:
	Produces parameter error message using the default error handling
	method.

 Returns:
	The input error code.

 Notes:
																	*/
static SimErrCode SimParmError(
	SimErrCode errorCode,	/* Error code							*/
	char	*parmName,		/* Parameter name						*/
	FILE	*filePtr)		/* Ptr to stream with error				*/
{
	char	*msg;			/* Pointer to buffer with message		*/
	static char format[] = "Parameter '%s' of record # %d of file '%s'";
							/* Format for error message				*/
	int		fudge = 10;		/* Fudge factor for msg buffer length	*/
	int		nBytes = int(strlen(format) + strlen(parmName) + strlen(SimFiles[fileno(filePtr)].name)) +
		fudge;	/* Number of bytes in message buffer	*/

/* Restore the file pointer to the beginning of the record.		*/
	fseek(filePtr, SimFiles[fileno(filePtr)].recOffset, SEEK_SET);

	/* If we can allocate memory for the message, build the standard*/
	/* message and send it out.										*/
	if((msg = (char *)malloc(nBytes)) != NULL)
	{
		sprintf(msg, format,
				parmName,
				SimFiles[fileno(filePtr)].recNum,
				SimFiles[fileno(filePtr)].name);
		SimError(errorCode, msg, DefaultErrMode.exitFlag, DefaultErrMode.outPtr);
		free(msg);
	}

/* Otherwise send out the error with a null message.			*/
	else
		SimError(errorCode, "", DefaultErrMode.exitFlag, DefaultErrMode.outPtr);

	return (errorCode);
}

/*********************************************************************
 Function:
	SimOpen

 Description:
	Opens a simulation data file.

 Returns:
	SUCCESS, CANT_OPEN, TOO_MANY_FILES, OUT_OF_MEMORY

 Notes:
	"fseek" does not work properly for text files in Microsoft C 5.1.
	Therefore, in Microsoft C, SimOpen sets binary mode for all files.
	Files opened with "w" access rights cannot be read in Think C 3.0.
	Therefore, write-access files should be opened with "w+".
																	*/

SimErrCode	SimOpen(
	const char	*name,			/* Name of the file to open				*/
	const char	*format,		/* Mode for opening the file			*/
	FILE	**filePtr)		/* Pointer to a stream pointer			*/
{
	int		fileNum;		/* File number of current stream		*/

//<<<<<<< HEAD
	/* Open the file.	
	//! экстренная замена*/
//	if((*filePtr = fopen(name, format)) == NULL)
//	{
//		SimError(CANT_OPEN, name, DefaultErrMode.exitFlag, DefaultErrMode.outPtr);
//		return (CANT_OPEN);
//	}
//=======
	/* Open the file.												*/
	if((*filePtr = std::fopen(name, format)) == NULL)
	{
		SimError(CANT_OPEN, name, DefaultErrMode.exitFlag, DefaultErrMode.outPtr);
		return (CANT_OPEN);
	}
//>>>>>>> 0007c084a37bb91106e314ed1757cbe765a03039
/* Get file number of the new stream.							*/
	if((fileNum = fileno(*filePtr)) == EOF || SimFiles.count() >= MAXFILES)
	{
		SimError(TOO_MANY_FILES, name, DefaultErrMode.exitFlag, DefaultErrMode.outPtr);
		fclose(*filePtr);
		return (TOO_MANY_FILES);
	}
	SimFiles.AddFile(*filePtr);

	/* Change the mode to binary (for IBM PC, so fseek works).      */
#ifdef IBMPC
	setmode(fileNum, O_BINARY);
#endif

	/* Initialize the file position (byte offset from beginning).	*/
	SimFiles[fileNum].recOffset = 0;

	/* Initialize the file name.									*/
//	SimFiles[fileNum].name = nameBuffer;
//	strcpy (nameBuffer, name);
	strcpy(SimFiles[fileNum].name, name);

	/* Initialize the record header length.							*/
	if(fgetc(*filePtr) == RECORD_HEADER_KEY &&
		(fgetc(*filePtr) == RECORD_HEADER_KEY))
		SimFiles[fileNum].headLen = SHORT_HEADER_LENGTH;
	else
		SimFiles[fileNum].headLen = LONG_HEADER_LENGTH;

	/* Zero the record number to signal no records read yet.		*/
	rewind(*filePtr);
	SimFiles[fileNum].recNum = 0;

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimClose

 Description:
	Closes an open simulation data file.

 Returns:
	 SUCCESS, CANT_CLOSE

 Notes:
																	*/

SimErrCode	SimClose(FILE	*filePtr)
{

/* Deallocate the name buffer.									*/
	SimFiles.RemoveFile(filePtr);
	//free (SimFiles[fileno(filePtr)].name);

	/* Close the file.												*/
	if(fclose(filePtr) == EOF)
		return (SimStandardError(CANT_CLOSE, filePtr));

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimGetHistory

 Description:
	Gets requested parameters from history portion of file.

 Returns:
	SUCCESS, LEGAL_EOF, BAD_HEADER, BAD_RECORD_NUMBER,
	BAD_PHRASE, IO_ERROR, UNEXPECTED_EOF, MISSING_HIST_KEY,
	MISSING_STD_HIST

 Notes:
	Leaves the file pointer after the last history record.
	There is a problem retrieving the standard reserved keywords:
	SimGetHistory would not know which history record you intended
	to read, because all history records are required to have all
	the reserved keywords.  To resolve this ambiguity, you must
	prefix the the process name to the reserved keyword name;
	for example, to retrieve the description of a file created by
	Editor, specify the keyword name as REditor.ProcessDescriptionS.

																	*/
SimErrCode	SimGetHistory(
	int		numParams,		/* Number of parameters					*/
	std::vector<param>	&paramList,		/* Parameter names and addr.			*/
	FILE	*filePtr)		/* Stream pointer 						*/
{
	//if(numParams != paramList.size()) Error(ssprintf("SimGetHistory, numParams(%d)!=paramList.size(%d)", numParams, int(paramList.size())));

	SimErrCode status;		/* Error return code					*/
	char	name[MAXNAMESIZE];	/* Keyword name						*/
	int		usrKey;			/* Keyword counter for user keywords	*/
	int		stdKey;			/* Keyword counter for standard keywords*/
	int		nameLength;		/* Length of ProcessName				*/

	/* Rewind the input file.										*/
	rewind(filePtr);
	SimFiles[fileno(filePtr)].recNum = 0;

	/* Zero the "found count" for the user-requested keywords.		*/
	for(usrKey = 0; usrKey < numParams; usrKey++)
		paramList[usrKey].timesFound = 0;

	/* Loop through all the history records.						*/
	for(;;)
	{

	/* Read the next record header.  Exit the loop when we hit	*/
	/* a legal end-of-file.  Report all other errors.			*/
		if((status = SimGetHeader(filePtr)) != SUCCESS)
			if(status == LEGAL_EOF)
				break;
			else
				return (SimStandardError(status, filePtr));

		/* If we hit a non-history record, back up the file pointer	*/
		/* to the beginning of the header; then exit the loop.		*/
		if(SimFiles[fileno(filePtr)].recType != HISTORY)
		{
			fseek(filePtr, SimFiles[fileno(filePtr)].recOffset, SEEK_SET);
			SimFiles[fileno(filePtr)].recNum--;
			break;
		}

	/* Initialize standard keyword structure.					*/
		for(stdKey = 0; stdKey < nStdHistoryParams; stdKey++)
		{
			ProcessNameComment[0] = '\0';
			ProcessDescriptionComment[0] = '\0';
			ProcessVersionComment[0] = '\0';
			ProcessDateTimeComment[0] = '\0';
			stdHistoryParams[stdKey].timesFound = 0;
		}

	/* Loop to extract the requested keywords from the history	*/
	/* record.													*/
		for(;;)
		{

		/* Read the next keyword name.  Exit the loop when we	*/
		/* hit a legal end-of-file or the end of the record.	*/
		/* Report all other errors.								*/
			if((status = SimGetKeywordName(name, filePtr)) != SUCCESS)
				if(status == LEGAL_EOF || status == END_OF_RECORD)
					break;
				else
					return (SimStandardError(status, filePtr));

			/* Search the user-requested keywords for a match.		*/
			for(usrKey = 0; usrKey < numParams; usrKey++)
				if(strcmp(name, paramList[usrKey].name) == 0)
					if(SUCCESS != (status =
					   SimGetKeyword(&paramList[usrKey], filePtr)))
						return (SimParmError(status, name, filePtr));

			/* Search the standard keywords for a match.			*/
			for(stdKey = 0; stdKey < nStdHistoryParams; stdKey++)
				if(strcmp(name, stdHistoryParams[stdKey].name) == 0)
					if(SUCCESS != (status =
					   SimGetKeyword(&stdHistoryParams[stdKey], filePtr)))
						return (SimParmError(status, name, filePtr));

			/* Skip to the end of this keyword.						*/
			if((status = SimSkipKeyword(filePtr)) != SUCCESS)
				return (SimParmError(status, name, filePtr));

		}				/* Loop over keywords					*/

	/* Loop to make sure the history record has all the			*/
	/* required standard keywords.								*/
		for(stdKey = 0; stdKey < nStdHistoryParams; stdKey++)
			if(stdHistoryParams[stdKey].timesFound == 0)
				return (SimParmError(MISSING_STD_HIST,
						stdHistoryParams[stdKey].name, filePtr));

	/* Check to see whether any of the requested keywords are	*/
	/* of the ProcessName.Keyword form which matches the		*/
	/* current process name.									*/
		nameLength = int(strlen(ProcessName));

		for(usrKey = 0; usrKey < numParams; usrKey++)
			if(strncmp(ProcessName,
			   paramList[usrKey].name, nameLength) == 0 &&
			   paramList[usrKey].name[nameLength] == STD_KEYWORD_KEY)
				for(stdKey = 0; stdKey < nStdHistoryParams; stdKey++)
					if(strcmp(stdHistoryParams[stdKey].name,
					   &paramList[usrKey].name[nameLength+1])
					   == 0)
					{
						Copy(stdHistoryParams[stdKey].value,
							 paramList[usrKey].value,
							 stdHistoryParams[stdKey].type);
						Copy(stdHistoryParams[stdKey].comment,
							 paramList[usrKey].comment,
							 ASTRING);
						paramList[usrKey].timesFound++;
					}


	}					/* End of loop over history records		*/

/* Loop to make sure all the user-requested keywords were found.*/
	for(usrKey = 0; usrKey < numParams; usrKey++)
		if(paramList[usrKey].timesFound == 0)
			return (SimParmError(MISSING_HIST_KEY,
					paramList[usrKey].name, filePtr));

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimPutHistory

 Description:
	Writes a new simulation history record.

 Returns:
	SUCCESS, MISSING_STD_HIST, IO_ERROR

 Notes:
	Each history record must include all the standard history
	parameters.
	For the sake of legibility, SimPutHistory will put a line
	terminator before each keyword phrase, and two line terminators
	at the end of the record.
																	*/
SimErrCode	SimPutHistory(
	int		numParams,		/* Number of parameters					*/
	std::vector<param>	&paramList,		/* Parameter names and addr.			*/
	FILE	*filePtr)		/* Stream pointer						*/
{
	int		stdKey;			/* Counts standard keywords				*/
	int		usrKey;			/* Counts user's keywords				*/
	bool	found;			/* Flag that we have found the keyword	*/
	unsigned checksum;		/* Computed checksum					*/
	SimErrCode status;		/* Status returned by checksum routine	*/

	//if(numParams != paramList.size()) Error(ssprintf("SimPutHistory, numParams(%d)!=paramList.size(%d)", numParams, int(paramList.size())));

	/* Check whether we have all the standard parameters.			*/
	for(stdKey = 0; stdKey < nStdHistoryParams; stdKey++)
	{
		found = false;
		for(usrKey = 0; usrKey < numParams; usrKey++)
			if(strcmp(stdHistoryParams[stdKey].name,
			   paramList[usrKey].name) == 0)
			{
				found = true;
				break;
			}
		if(!found)
			return (SimParmError(MISSING_STD_HIST,
					stdHistoryParams[stdKey].name, filePtr));
	}

/* Clear any error flags on the output stream.					*/
	clearerr(filePtr);

	/* Write a dummy record header.									*/
	SimPutHeader(0, 0, HISTORY, filePtr);

	/* Loop through user-supplied keywords.							*/
	for(usrKey = 0; usrKey < numParams; usrKey++)
	{

	/* Write a line feed and the keyword name.					*/
		fprintf(filePtr, "\r%s ", paramList[usrKey].name);

		/* Write the keyword value.									*/
		switch(paramList[usrKey].type)
		{
			case ASTRING:
				fprintf(filePtr, "\"%s\"", (char*)(paramList[usrKey].value));
				break;
			case ADOUBLE:
				fprintf(filePtr, "%f", *(double *)paramList[usrKey].value);
				break;
			case AINT:
				fprintf(filePtr, "%d", *(int *)paramList[usrKey].value);
				break;
			case ALONG:
				fprintf(filePtr, "%ld", *(long *)paramList[usrKey].value);
				break;
			case AULONG:
				fprintf(filePtr, "%lu", *(unsigned long *)paramList[usrKey].value);
				break;
		}

	/* Write the comment, if any.								*/
		if(paramList[usrKey].comment != NULL)
			fprintf(filePtr, " %s", paramList[usrKey].comment);

		/* Write the phrase terminator.								*/
		fprintf(filePtr, "%c", PHRASE_END_KEY);
	}

/* Write another two line feeds.								*/
	fprintf(filePtr, "\r\r");

	/* Calculate the actual record length.							*/
	SimFiles[fileno(filePtr)].dataLen = 
		int(ftell(filePtr) -SimFiles[fileno(filePtr)].recOffset - SimFiles[fileno(filePtr)].headLen);

	/* Calculate the checksum.										*/
	if(SUCCESS != (status = SimCheckDisk(
		SimFiles[fileno(filePtr)].recOffset +
		(int)SimFiles[fileno(filePtr)].headLen,
		SimFiles[fileno(filePtr)].dataLen,
		&checksum, filePtr)))
		return (SimStandardError(status, filePtr));

	/* Put file pointer back to the beginning of the record header.	*/
	fseek(filePtr, SimFiles[fileno(filePtr)].recOffset, SEEK_SET);
	SimFiles[fileno(filePtr)].recNum--;

	/* Write the real record header.								*/
	SimPutHeader(checksum, SimFiles[fileno(filePtr)].dataLen, HISTORY,
				 filePtr);

		 /* Skip to the end of this record.								*/
	SimNextRecord(filePtr);

	/* Check whether any of this has caused an I/O error.			*/
	if(ferror(filePtr))
		return (SimStandardError(IO_ERROR, filePtr));

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimCopyHistory

 Description:
	Copies history portion of input files to output files.

 Returns:
	SUCCESS, LEGAL_EOF, UNEXPECTED_EOF, IO_ERROR, BAD_HEADER,
	BAD_RECORD_NUMBER, OUT_OF_MEMORY

 Notes:
	Rewinds all files before starting, and leaves all file pointers
	after the last history record.
	Merges the input history files by eliminating identical history
	records and by verifying that duplicate parameters have the same
	value.

																	*/
SimErrCode	SimCopyHistory(
	int		nInFiles,		/* Number of input streams				*/
	FILE	*inFile[],		/* Input stream pointers				*/
	int		nOutFiles,		/* Number of output streams				*/
	FILE	*outFile[])		/* Output stream pointers				*/
{
	int		in, out;		/* Input and output file counters		*/
	FILE	*input;			/* Current input file stream pointer	*/
	FILE	*output;		/* Current output file stream pointer	*/
	struct SimFile *inInfo;	/* Pointer to input file info			*/
	SimErrCode status;		/* Status returned by Sim... routines	*/
	char	*buffer;		/* Pointer to buffer for record data	*/

	/* Loop through all the output files.							*/
	for(out = 0; out < nOutFiles; out++)
	{
		output = outFile[out];
		rewind(output);
		SimFiles[fileno(output)].recNum = 0;

		/* Loop through the input files.								*/
		for(in = 0; in < nInFiles; in++)
		{
			input = inFile[in];
			inInfo = &SimFiles[fileno(input)];
			rewind(input);
			SimFiles[fileno(input)].recNum = 0;

			/* Loop through the history records in the input.		*/
			for(;;)
			{

			/* Read the next record header.  Exit the loop when	*/
			/* we hit a legal end-of-file.  Report all other	*/
			/* errors.											*/
				if((status = SimGetHeader(input)) != SUCCESS)
					if(status == LEGAL_EOF)
						break;
					else
						return (SimStandardError(status, input));

				/* If we hit a non-history record, back up the file	*/
				/* pointer to the beginning of the header; then		*/
				/* exit the loop.									*/
				if(SimFiles[fileno(input)].recType != HISTORY)
				{
					fseek(input, SimFiles[fileno(input)].recOffset, SEEK_SET);
					SimFiles[fileno(input)].recNum--;
					break;
				}


			/* Write the header to the output file.				*/
				if(SUCCESS != (status =
				   SimPutHeader(inInfo->checksum, inInfo->dataLen,
				   inInfo->recType, output)))
					return (SimStandardError(status, output));

				/* Copy the history record data to the output.		*/
				if((buffer = (char *)malloc((unsigned)inInfo->dataLen)) == NULL)
					return (SimStandardError(OUT_OF_MEMORY, input));

			#ifdef Stardent
				if(fread(buffer, (size_t) sizeof(char),
					(int)inInfo->dataLen, input) == 0)
					return (SimStandardError(IO_ERROR, input));
			#else
				if(fread(buffer, (size_t) sizeof(char),
					(size_t)inInfo->dataLen, input) == 0)
					return (SimStandardError(IO_ERROR, input));
			#endif

			#ifdef Stardent
				if(fwrite(buffer, (size_t) sizeof(char),
					(int)inInfo->dataLen, output) == 0)
					return (SimStandardError(IO_ERROR, output));
			#else
				if(fwrite(buffer, (size_t) sizeof(char),
					(size_t)inInfo->dataLen, output) == 0)
					return (SimStandardError(IO_ERROR, output));
			#endif

				free(buffer);

			}			/* Loop over input history records		*/
		}				/* Loop over input files.				*/
	}					/* Loop over output files.				*/
	return(SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimGetDescriptor

 Description:
	Gets parameters of current descriptor record.

 Returns:
	SUCCESS, UNEXPECTED_EOF, LEGAL_EOF, IO_ERROR, BAD_HEADER,
	BAD_RECORD_NUMBER, NO_DESCRIPTOR, END_OF_RECORD, BAD_PHRASE,
	MISSING_STD_DESC, MISSING_DESC_KEY

 Notes:
	Purposely does not move file pointer to next record.  This allows
	calling this function more than once for the same input record,
	which is useful to test the record information content before
	reading all the parameters.
																	*/
SimErrCode	SimGetDescriptor(
	int		numParams,		/* Number of parameters					*/
	vector<param>	&paramList,		/* Pointer to parameters				*/
	FILE	*filePtr)		/* Stream pointer						*/
{
	SimErrCode status;		/* Error return code					*/
	char	name[MAXNAMESIZE];	/* Keyword name						*/
	int		usrKey;			/* Keyword counter for user keywords	*/
	int		stdKey;			/* Keyword counter for standard keywords*/

	//TODO параметры не тождественны, надо их уравнять
	//if(numParams != paramList.size()) Error(ssprintf("SimGetDescriptor, numParams(%d)!=paramList.size(%d)", numParams, int(paramList.size())));


	/* Initialize user-requested keyword structure.					*/
	for(usrKey = 0; usrKey < numParams; usrKey++)
		paramList[usrKey].timesFound = 0;

	/* Initialize standard keyword structure.						*/
	for(stdKey = 0; stdKey < nStdDescriptorParams; stdKey++)
	{
		ProcessorTypeComment[0] = '\0';
		DataTypeComment[0] = '\0';
		DataCountComment[0] = '\0';
		stdDescriptorParams[stdKey].timesFound = 0;
	}

/* Read the record header and make sure it is a descriptor.		*/
	if((status = SimGetHeader(filePtr)) != SUCCESS)
		return (SimStandardError(status, filePtr));

	if(SimFiles[fileno(filePtr)].recType != DESCRIPTOR)
		return (SimStandardError(NO_DESCRIPTOR, filePtr));

	/* Loop to extract the requested keywords from the descriptor	*/
	/* record.														*/

	for(;;)
	{

	/* Read the next keyword name.  Exit the loop when we hit a	*/
	/* legal end-of-file or the end of the record. Report all	*/
	/* other errors.											*/
		if((status = SimGetKeywordName(name, filePtr)) != SUCCESS)
			if(status == LEGAL_EOF || status == END_OF_RECORD)
				break;
			else
				return (SimStandardError(status, filePtr));

		/* Search the user-requested keywords for a match.			*/
		for(usrKey = 0; usrKey < numParams; usrKey++)
		{
			if(strcmp(name, paramList[usrKey].name) == 0)
				if(SUCCESS != (status =
				   SimGetKeyword(&paramList[usrKey], filePtr)))
					return (SimParmError(status, name, filePtr));
		}

	/* Search the standard keywords for a match.				*/
		for(stdKey = 0; stdKey < nStdDescriptorParams; stdKey++)
		{
			if(strcmp(name, stdDescriptorParams[stdKey].name) == 0)
				if(SUCCESS != (status =
				   SimGetKeyword(&stdDescriptorParams[stdKey], filePtr)))
					return (SimParmError(status, name, filePtr));
		}

	/* Skip to the end of this keyword.							*/
		if((status = SimSkipKeyword(filePtr)) != SUCCESS)
			return (SimParmError(status, name, filePtr));

	}
					/* Loop over keywords					*/
/* Loop to make sure all the standard keywords were found.		*/
	for(stdKey = 0; stdKey < nStdDescriptorParams; stdKey++)
		if(stdDescriptorParams[stdKey].timesFound == 0)
			return (SimParmError(MISSING_STD_DESC,
					stdDescriptorParams[stdKey].name, filePtr));

/* Loop to make sure all the user-requested keywords were found.*/
	for(usrKey = 0; usrKey < numParams; usrKey++)
		if(paramList[usrKey].timesFound == 0)
			return (SimParmError(MISSING_DESC_KEY,
					paramList[usrKey].name, filePtr));

/* Put the record pointer back to the beginning of the record.	*/
	fseek(filePtr, SimFiles[fileno(filePtr)].recOffset, SEEK_SET);
	SimFiles[fileno(filePtr)].recNum--;

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimPutDescriptor

 Description:
	Writes new descriptor record.

 Returns:
	SUCCESS, MISSING_STD_DESC, IO_ERROR

 Notes:
	For the sake of legibility, SimPutDescriptor will put a line
	terminator before each keyword phrase, and an extra line
	terminator at the end of the record.
																	*/
SimErrCode	SimPutDescriptor(
	int		numParams,		/* Number of parameters					*/
	std::vector<param>	&paramList,		/* Parameter names and addr.			*/
	FILE	*filePtr)		/* Stream pointer						*/
{
	int		stdKey;			/* Counts standard keywords				*/
	int		usrKey;			/* Counts user's keywords				*/
	bool	found;			/* Flag that we have found the keyword	*/
	unsigned checksum;		/* Computed checksum					*/
	SimErrCode status;		/* Status returned by checksum routine	*/

	//if(numParams != paramList.size()) Error(ssprintf("SimPutDescriptor, numParams(%d)!=paramList.size(%d)", numParams, int(paramList.size())));

	/* Check whether we have all the standard parameters.			*/
	for(stdKey = 0; stdKey < nStdDescriptorParams; stdKey++)
	{
		found = false;
		for(usrKey = 0; usrKey < numParams; usrKey++)
			if(strcmp(stdDescriptorParams[stdKey].name,
			   paramList[usrKey].name) == 0)
			{
				found = true;
				break;
			}
		if(!found)
			return (SimParmError(MISSING_STD_DESC,
					stdDescriptorParams[stdKey].name, filePtr));
	}

/* Clear any error flags on the output stream.					*/
	clearerr(filePtr);

	/* Write a dummy record header.									*/
	SimPutHeader(0, 0, DESCRIPTOR, filePtr);

	/* Loop through user-supplied keywords.							*/
	for(usrKey = 0; usrKey < numParams; usrKey++)
	{

	/* Write a line feed and the keyword name.					*/
		fprintf(filePtr, "\r%s ", paramList[usrKey].name);

		/* Write the keyword value.									*/
		switch(paramList[usrKey].type)
		{
			case ASTRING:
				fprintf(filePtr, "\"%s\"", (char *)paramList[usrKey].value);
				break;
			case ADOUBLE:
				fprintf(filePtr, "%f", *(double *)paramList[usrKey].value);
				break;
			case AINT:
				fprintf(filePtr, "%d", *(int *)paramList[usrKey].value);
				break;
			case ALONG:
				fprintf(filePtr, "%ld", *(long *)paramList[usrKey].value);
				break;
			case AULONG:
				fprintf(filePtr, "%lu", *(unsigned long *)paramList[usrKey].value);
				break;
		}

	/* Write the comment, if any.								*/
		if(paramList[usrKey].comment != NULL)
			fprintf(filePtr, " %s", paramList[usrKey].comment);

		/* Write the phrase terminator.								*/
		fprintf(filePtr, "%c", PHRASE_END_KEY);
	}

/* Write another line feed.										*/
	fprintf(filePtr, "\r");

	/* Calculate the actual record length.							*/
	SimFiles[fileno(filePtr)].dataLen = int(ftell(filePtr) - SimFiles[fileno(filePtr)].recOffset - SimFiles[fileno(filePtr)].headLen);

	/* Calculate the checksum.										*/
	if(SUCCESS != (status = SimCheckDisk(
		SimFiles[fileno(filePtr)].recOffset +
		(long)SimFiles[fileno(filePtr)].headLen,
		SimFiles[fileno(filePtr)].dataLen,
		&checksum, filePtr)))
		return (SimStandardError(status, filePtr));

	/* Put file pointer back to the beginning of the record header.	*/
	fseek(filePtr, SimFiles[fileno(filePtr)].recOffset, SEEK_SET);
	SimFiles[fileno(filePtr)].recNum--;

	/* Write the real record header.								*/
	SimPutHeader(checksum, SimFiles[fileno(filePtr)].dataLen,
				 DESCRIPTOR, filePtr);

			 /* Skip to the end of this record.								*/
	SimNextRecord(filePtr);

	/* Check whether any of this has caused an I/O error.			*/
	if(ferror(filePtr))
		return (SimStandardError(IO_ERROR, filePtr));

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimGetData

 Description:
	Skips descriptor record and reads the binary data record.  The
	size of the buffer is provided so that SimGetData does not
	overflow it.

 Returns:
	SUCCESS, LEGAL_EOF, UNEXPECTED_EOF, IO_ERROR, BAD_HEADER,
	BAD_RECORD_NUMBER, TOO_MUCH_DATA

 Notes:
	Moves file pointer to next record.
																	*/
SimErrCode	SimGetData(
	size_t	maxBytes,		/* Maximum number of bytes				*/
	void	*bufferPtr,		/* Buffer pointer						*/
	FILE	*filePtr)		/* Stream pointer						*/
{
	SimErrCode status;		/* Status returned by internal routines	*/

	/* Skip the descriptor record.									*/
	if((status = SimGetHeader(filePtr)) != SUCCESS)
		return (SimStandardError(status, filePtr));
	SimNextRecord(filePtr);

	/* Read the record header and make sure this is a data record.	*/
	if((status = SimGetHeader(filePtr)) != SUCCESS)
		return (SimStandardError(status, filePtr));

	if(SimFiles[fileno(filePtr)].recType != DATA)
		return (SimStandardError(NO_DATA_, filePtr));

	/* Read the data; return an error if appropriate.				*/

#ifdef Stardent
	if(fread(bufferPtr, (size_t) sizeof(char),
		(int)min(maxBytes, SimFiles[fileno(filePtr)].dataLen),
	   filePtr) == 0)
		if(feof(filePtr))
			return (SimStandardError(UNEXPECTED_EOF, filePtr));
		else
			return (SimStandardError(IO_ERROR, filePtr));
#else
	if(fread(bufferPtr, (size_t) sizeof(char),
		(size_t)min((int)maxBytes, SimFiles[fileno(filePtr)].dataLen),
	   filePtr) == 0)
		if(feof(filePtr))
			return (SimStandardError(UNEXPECTED_EOF, filePtr));
		else
			return (SimStandardError(IO_ERROR, filePtr));
#endif

	/* Return an error message if there was more data than buffer.	*/
	if(maxBytes < SimFiles[fileno(filePtr)].dataLen)
		return (SimStandardError(TOO_MUCH_DATA, filePtr));

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimPutData

 Description:
	Writes the binary data record.

 Returns:
	SUCCESS, IO_ERROR

 Notes:
	Leaves the file pointer after the new data record.
																	*/
SimErrCode	SimPutData(
	size_t	nBytes,			/* Number of data bytes					*/
	void	*dataPtr,		/* Pointer to data						*/
	FILE	*filePtr)		/* Stream pointer						*/
{
	unsigned checksum;		/* Calculated checksum					*/

	/* Clear any error flags on the output stream.					*/
	clearerr(filePtr);

	/* Calculate the checksum.										*/
	SimCheckRAM(dataPtr, (int)nBytes, &checksum);

	/* Write the data header.										*/
	SimPutHeader(checksum, (int)nBytes, DATA, filePtr);

	/* Write the data buffer.										*/
#ifdef Stardent
	fwrite(dataPtr, (size_t) sizeof(char), (int)nBytes, filePtr);
#else
	fwrite(dataPtr, (size_t) sizeof(char), (size_t)nBytes, filePtr);
#endif

	/* Check whether this caused errors.							*/
	if(ferror(filePtr))
		return (SimStandardError(IO_ERROR, filePtr));

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimPutMuchData

 Description:
	Writes the binary data record.
	Use this instead of SimPutData when size_t is inadequate to
	describe the data length, i.e. particularly on a PC (where size_t
	is unsigned int).	The data is regarded as a rectangular 2-D
	array specified by an array of pointers

 Returns:
	SUCCESS, IO_ERROR

 Notes:
	Leaves the file pointer after the new data record.
	*/
SimErrCode	SimPutMuchData(
	void	*dataPtrArray[],	/* Array of pointers to data			*/
	int	 	nBlocks,			/* # pointers (eg: # rows)				*/
	int		nBytesPerBlock,		/* # bytes/block (eg: # cols/row)		*/
	FILE	*filePtr)			/* Stream pointer						*/
{
	unsigned checksum;			/* running checksum				*/
	unsigned char	*p;			/* running buffer pointer		*/
	unsigned char	*pstop;		/* ending buffer pointer		*/
	int	block;

	/* Clear any error flags on the output stream.						*/
	clearerr(filePtr);

	/* Calculate the checksum (in blocks).								*/
	checksum = 0;
	for(block=0; block<nBlocks; block++)
	{
		p = (unsigned char *)dataPtrArray[block];
		pstop = p + nBytesPerBlock;
		for(; p < pstop; p++)
		{
			checksum += *p;
		}
	}

/* Write the data header.											*/
	SimPutHeader(checksum, (int)nBlocks*nBytesPerBlock, DATA, filePtr);

	/* Write the data buffer in blocks.									*/
#ifdef Stardent
	for(block=0; block<nBlocks; block++)
	{
		fwrite(dataPtrArray[block], (size_t) sizeof(char),
			(int)nBytesPerBlock, filePtr);
	}
#else
	for(block=0; block<nBlocks; block++)
	{
		fwrite(dataPtrArray[block], (size_t) sizeof(char),
			(size_t)nBytesPerBlock, filePtr);
	}
#endif

	/* Check whether this caused errors.								*/
	if(ferror(filePtr))
		return (SimStandardError(IO_ERROR, filePtr));

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimSkipData

 Description:
	Skips to the next descriptor record.

 Returns:
	SUCCESS, BAD_HEADER, BAD_RECORD_NUMBER

 Notes:
																	*/
SimErrCode	SimSkipData(
	FILE	*filePtr) //Stream pointer
{
	SimErrCode status;	//Status returned by internal routines
	bool	skippedOne = false;
							// Flag that we have skipped at least
							// one record

	// Loop to skip to the next descriptor record
	for(;;)
	{

	// Read the next record header
		if((status = SimGetHeader(filePtr)) != SUCCESS)
			if(status == LEGAL_EOF)
				break;
			else
				return (SimStandardError(status, filePtr));

		// Check whether this record is a descriptor.
		if(SimFiles[fileno(filePtr)].recType == DESCRIPTOR)
			if(skippedOne)
				break;
			else
				skippedOne = true;

		/* Skip the next record.									*/
		SimNextRecord(filePtr);
	}

/* Position us back at the beginning of the record.				*/
	fseek(filePtr, SimFiles[fileno(filePtr)].recOffset, SEEK_SET);
	SimFiles[fileno(filePtr)].recNum--;

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimSkipHistory

 Description:
	Skips to the first descriptor record.

 Returns:
	SUCCESS, BAD_HEADER, BAD_RECORD_NUMBER

 Notes:
																	*/
SimErrCode	SimSkipHistory(
	FILE	*filePtr)		/* Stream pointer						*/
{
	SimErrCode status;		/* Status returned by internal routines	*/

	/* Rewind the file.												*/
	rewind(filePtr);
	SimFiles[fileno(filePtr)].recNum = 0;

	/* Loop to skip to the next descriptor record.					*/
	for(;;)
	{

	/* Read the next record header.								*/
		if((status = SimGetHeader(filePtr)) != SUCCESS)
			if(status == LEGAL_EOF)
				break;
			else
				return (SimStandardError(status, filePtr));

		/* Check whether this record is a descriptor.				*/
		if(SimFiles[fileno(filePtr)].recType == DESCRIPTOR)
			break;

		/* Skip the next record.									*/
		SimNextRecord(filePtr);
	}

/* Position us back at the beginning of the record.				*/
	fseek(filePtr, SimFiles[fileno(filePtr)].recOffset, SEEK_SET);
	SimFiles[fileno(filePtr)].recNum--;

	return (SUCCESS);
}

/*--------------------------------------------------------------------
 Function:
	SimStandardError

 Description:
	Produces standard error message using the default error handling
	method.

 Returns:
	The input error code.

 Notes:
	SimStandardError restores the file pointer to the beginning of the
	record which caused the error.
																	*/
SimErrCode	SimStandardError(
	SimErrCode errorCode,	/* Error code							*/
	FILE	*filePtr)		/* Ptr to stream with error				*/
{
	char	*msg;			/* Pointer to buffer with message		*/
	static char format[] = "Record # %d of file '%s'";
							/* Format for error message				*/
	int		fudge = 10;		/* Fudge factor for msg buffer length	*/
	int		nBytes = int(strlen(format) + strlen(SimFiles[fileno(filePtr)].name)) +
		fudge;	/* Number of bytes in message buffer	*/

/* Restore file pointer to beginning of record.					*/
	fseek(filePtr, SimFiles[fileno(filePtr)].recOffset, SEEK_SET);

	/* If we can allocate memory for the message, build the standard*/
	/* message and send it out.										*/
	if((msg = (char *)malloc(nBytes)) != NULL)
	{
		sprintf(msg, format,
				SimFiles[fileno(filePtr)].recNum,
				SimFiles[fileno(filePtr)].name);
		SimError(errorCode, msg, DefaultErrMode.exitFlag, DefaultErrMode.outPtr);
		free(msg);
	}

/* Otherwise send out the error with a null message.			*/
	else
		SimError(errorCode, "", DefaultErrMode.exitFlag, DefaultErrMode.outPtr);

	return (errorCode);
}

/*--------------------------------------------------------------------
 Function:
	SimError

 Description:
	Processes an error of a given type.

 Returns:

 Notes:
	Set outptr to SUPPRESS to suppress the message; so
	(RETURN, SUPPRESS) turns off all automatic error handling.
																	*/
void		SimError(
	SimErrCode errorCode,	/* Error code							*/
	const char	*message,		/* Supplemental message					*/
	bool	exitFlag,		/* Flag: EXIT or RETURN					*/
	FILE	*outPtr)		/* Stream pointer for msg				*/
{
/* Print the error message if the output stream is not NULL.	*/
	if(outPtr != NULL)
	{
		fflush(stdout);
		fflush(stderr);
		fprintf(outPtr, "\n\a***Simulation I/O error:\n%s\n%s\n",
				errorCode, message);
	}

/* Exit with a non-zero return code, if requested.				*/
	if(exitFlag)
	{
		exit(1);
	}
}

/*--------------------------------------------------------------------
 Function:
	SimErrorMode

 Description:
	Sets the default error handling method.  If an error is
	encountered, all simulation file-access primitives will
	automatically call SimError in the mode set by this subroutine.

 Returns:

 Notes:
	Set outptr to SUPPRESS to suppress the message; so
	(RETURN, SUPPRESS) turns off all automatic error handling.
	If SimErrorMode is not called, the default error-handling mode
	is (EXIT, stderr).
																	*/
void		SimErrorMode(
	bool	exitFlag,		/* Exit flag: true or false				*/
	FILE	*outPtr)		/* Stream pointer for msg				*/
{
/* Save the specified error handling method as the default.		*/
	DefaultErrMode.exitFlag = exitFlag;
	if(outPtr) DefaultErrMode.outPtr = outPtr;
	else DefaultErrMode.outPtr = stdout;
}


/*--------------------------------------------------------------------
 Function:
	SimCheckSumCheckEnable

 Description:
	Sets/resets the local check sum check control flag

 Returns:

 Notes:
																	*/
void		SimCheckSumCheckEnable(
	bool	checksumFlag)		/*  checking on  <-- true			*/
{				/*  checking off <-- false			*/
	CheckSumCheckEnable = checksumFlag;
}
