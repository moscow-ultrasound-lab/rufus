#include "pre.h"
#include <map>
#include <vector>
#include <string>
#include "TabDelimitedFile.h"
#include <XRADSystem/CFile.h>
#include <XRADSystem/Sources/TextFile/text_file.h>

/*!
 * \file ParseTable.cpp
 * \date 2017/11/17 10:55
 *
 * \author kulberg
 *
 * \brief 
 *
 * TODO: long description
 *
 * \note
*/

XRAD_BEGIN
namespace
{


bool	is_whitespace_string(wstring s)
{
	size_t counter = 0;
	for(auto c: s)
	{
		if(c>127) ++counter;
		else if(!isspace(c)) ++counter;
	}
	return !counter;
}


vector<wstring> ParseTabDelimitedRow(const wstring &data, wchar_t delimiter, bool erase_whitespace_cells)
{
	vector<wstring> row;
	auto cell_start = data.begin();
	auto is_delimiter = [delimiter](wchar_t c){return c==delimiter;};

	while(cell_start < data.end())
	{
		auto cell_end = find_if(cell_start, data.end(), is_delimiter);
		row.push_back(wstring(cell_start, cell_end));
		cell_start = cell_end<data.end() ? cell_end+1 : cell_end;
	}

	if(erase_whitespace_cells) for(auto &cell: row)
	{
		if(is_whitespace_string(cell)) cell = L"";
	}

	return row;
}



}//namespace

cells_type	ParseTabDelimitedText(const wstring &text, bool erase_whitespace_cells)
{
	cells_type cells;

	auto row_start = text.begin();
	auto is_crlf = [](wchar_t c){return c==L'\n' || c==L'\r';};
	auto is_not_crlf = [is_crlf](wchar_t c){return !is_crlf(c);};

	while(row_start < text.end())
	{
		auto row_end = find_if(row_start, text.end(), is_crlf);
		if(row_end>row_start) cells.push_back(ParseTabDelimitedRow(wstring(row_start, row_end), L'\t', erase_whitespace_cells));
		row_start = find_if(row_end, text.end(), is_not_crlf);
	}


	return cells;
}


cells_type	LoadTabDelimitedFile(wstring filename, bool erase_whitespace_cells)
{
	text_file_reader	tfile(filename, text_encoding::recognize_encoding_content);
	wstring	data;
	tfile.read(data);

	return ParseTabDelimitedText(data, erase_whitespace_cells);
}

XRAD_END

