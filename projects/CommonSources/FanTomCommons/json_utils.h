#ifndef json_utils_h__
#define json_utils_h__

/*!
	\file
	\date 2018/07/18 14:44
	\author kulberg

	\brief  
*/

#include <XRADBasic/ThirdParty/nlohmann/json.hpp >

//using json = nlohmann::basic_json<std::map, std::vector, std::wstring>;
using json = nlohmann::json;


XRAD_BEGIN


template<class T>
bool	try_get(T &value, string8 key, const json &j)
{
	auto	found = j.find(key);
	if(found==j.end()) return false;
	
	T buffer = *found;
	value = std::move(buffer);
	// простой вызов value = *found может приводить к ошибке C2593 "operator = is abmiguous"

	return true;
}

template<>
inline bool	try_get(wstring &value, string8 key, const json &j)
{
	string8	buffer;
	if(!try_get(buffer, key, j)) return false;
	value = string8_to_wstring(buffer);
	return true;
}

XRAD_END

#endif // json_utils_h__
