// stdafx.cpp : source file that includes just the standard includes
// smc-psip.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"


bool IsNumber(const char* str)
{
	if (str == NULL)
		return false;
	size_t uiLen = strlen(str);
	if (uiLen < 1)
		return false;
	//{{ddh-2014/6/20

	for (size_t i = 0; i < uiLen; ++i)
	{
		//负数判断
		if ((i == 0) && ((str[0]) == '-'))
			continue;
		if (str[i] < '0' || str[i] > '9')
			return false;
	}
	//}}
	return true;
}


namespace SMC_PSIP
{
	CLog* g_Log_ptr = NULL;
}