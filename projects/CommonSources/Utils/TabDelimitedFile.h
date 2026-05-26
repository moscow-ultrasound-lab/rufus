#ifndef ParseTable_h__
#define ParseTable_h__

/*!
 * \file ParseTable.h
 * \date 2017/11/17 10:56
 *
 * \author kulberg
 *
 * \brief 
 *
 * TODO: long description
 *
 * \note
*/

#include <vector>
#include <string>

XRAD_BEGIN

typedef vector<vector<wstring>> cells_type;
cells_type	LoadTabDelimitedFile(wstring filename, bool erase_whitespace_cells);
cells_type	ParseTabDelimitedText(const wstring &text, bool erase_whitespace_cells);


XRAD_END

#endif // ParseTable_h__
