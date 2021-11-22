
#pragma once

// STD
#include <iostream>
#include <fstream>
#include <cstdlib>
//
#include <string>
#include <algorithm>
#include <functional>
#include <iterator>
#include <list>
#include <vector>
#include <map>
#include <queue>

#include <locale>         // std::wstring_convert
#include <codecvt>        // std::codecvt_utf8
#include <cstdint>        // std::uint_least32_t


// STD::
using namespace std;



// Template (case insensitive) LESS comparison function
struct StringCaseILess {
	inline bool operator() (const string& x, const string& y) const
	{
		//return stricmp (p.data(), q.data()) < 0; 
		string::const_iterator p = x.begin();
		string::const_iterator q = y.begin();
		while (p != x.end() && q != y.end() && toupper(*p) == toupper(*q)) {
			++p; ++q;
		}
		if (p == x.end()) return q != y.end();
		return toupper(*p) < toupper(*q);
	}
};
struct WStringCaseILess {
	inline bool operator() (const wstring& x, const wstring& y) const
	{
		//return stricmp (p.data(), q.data()) < 0; 
		wstring::const_iterator p = x.begin();
		wstring::const_iterator q = y.begin();
		while (p != x.end() && q != y.end() && towupper(*p) == towupper(*q)) {
			++p; ++q;
		}
		if (p == x.end()) return q != y.end();
		return towupper(*p) < towupper(*q);
	}
};




// --------------------------------
// typedefs for STL based entities
// --------------------------------
typedef less<string>							LessStrFunc;
typedef less<wstring>							LessWStrFunc;
typedef	pair<string, string>					StringPair;
typedef	pair<wstring, wstring>					WStringPair;
typedef string::iterator						StringIter;
typedef wstring::iterator						WStringIter;

typedef list<string>							StringList;
typedef list<wstring>							WStringList;
typedef StringList::iterator					StringListIter;
typedef WStringList::iterator					WStringListIter;

typedef	map<string, string>						StringHash;
typedef	map<wstring, wstring>					WStringHash;
typedef StringHash::iterator					StringHashIter;
typedef WStringHash::iterator					WStringHashIter;

