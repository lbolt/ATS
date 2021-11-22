#pragma once
#include "../Utils/Includes/CommonX.h"
#include "../Utils/Includes/PropsX.h"

#include "ATS_CODES.h"
#include "../AT/Shared/ATServerAPIDefines.h"

#include "../AT/ActiveTickServerAPI/ActiveTickServerAPI.h"
#include "../AT/ActiveTickServerAPI/ActiveTickStreamListener.h"
#include "../AT/ActiveTickServerAPI/ActiveTickServerRequestor.h"

#include <string>

using namespace ATS_CODES;
using namespace std;




class AT_Commands
{
public:
	AT_Commands(void*);
	~AT_Commands();
	ATS_CODE initialize(void*);



private:
	// Declare private methods and objects
	AT_Commands();
	AT_Commands(const AT_Commands &);
	AT_Commands& operator=(const AT_Commands&);


};

