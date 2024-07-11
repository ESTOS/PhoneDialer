// ETSPVarStruct.h: interface for the ETSPVarStruct class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ETSPVARSTRUCT_H__14ED531F_4F32_4154_AC32_65B8328ED808__INCLUDED_)
#define AFX_ETSPVARSTRUCT_H__14ED531F_4F32_4154_AC32_65B8328ED808__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <malloc.h>
#include <stdstring.h>

template <class STRUCTTYPE> long VarInfoGetStringA(STRUCTTYPE* pData, DWORD dwOffset, DWORD dwSize, LPSTR szString, DWORD dwNumStringBytes)
{
	if (dwOffset + dwSize > pData->dwTotalSize)
		return -1;

	if (!dwOffset || !dwSize || dwNumStringBytes < sizeof(CHAR))
		return -1;

	DWORD dwMaxSize = dwSize;
	if (dwNumStringBytes - sizeof(CHAR) < dwSize)
		dwMaxSize = dwNumStringBytes - sizeof(CHAR);

	memcpy(szString, ((LPBYTE)pData) + dwOffset, dwMaxSize);
	memset(((LPBYTE)szString) + dwMaxSize, 0x00, sizeof(CHAR));
	return NO_ERROR;
}

template <class STRUCTTYPE> long VarInfoGetStringW(STRUCTTYPE* pData, DWORD dwOffset, DWORD dwSize, LPWSTR szString, DWORD dwNumStringChars)
{
	DWORD dwNumStringBytes = dwNumStringChars * sizeof(WCHAR);
	if (dwOffset + dwSize > pData->dwTotalSize)
		return -1;

	if (!dwOffset || !dwSize || dwNumStringBytes < sizeof(WCHAR))
		return -1;

	DWORD dwMaxSize = dwSize;
	if (dwNumStringBytes - sizeof(WCHAR) < dwSize)
		dwMaxSize = dwNumStringBytes - sizeof(WCHAR);

	memcpy(szString, ((LPBYTE)pData) + dwOffset, dwMaxSize);
	memset(((LPBYTE)szString) + dwMaxSize, 0x00, sizeof(WCHAR));
	return NO_ERROR;
}

template <class STRUCTTYPE> CStdStringW VarInfoGetStdStringW(STRUCTTYPE* pData, DWORD dwOffset, DWORD dwSize, DWORD dwMaxChars)
{
	CStdStringW wstrReturn;
	DWORD dwNumStringBytes = dwMaxChars * sizeof(WCHAR);
	if (dwOffset + dwSize > pData->dwTotalSize)
		return wstrReturn;

	if (!dwOffset || !dwSize || dwNumStringBytes < sizeof(WCHAR))
		return wstrReturn;

	DWORD dwMaxSize = dwSize;
	if (dwNumStringBytes - sizeof(WCHAR) < dwSize)
		dwMaxSize = dwNumStringBytes - sizeof(WCHAR);
	wstrReturn.assign((const wchar_t*)(((LPBYTE)pData) + dwOffset), dwMaxSize / sizeof(wchar_t));
	wstrReturn.TrimRight((wchar_t)0);
	return wstrReturn;
}

template <class STRUCTTYPE> CStdStringA VarInfoGetStdStringA(STRUCTTYPE* pData, DWORD dwOffset, DWORD dwSize, DWORD dwMaxChars)
{
	CStdStringA astrReturn;
	DWORD dwNumStringBytes = dwMaxChars * sizeof(char);
	if (dwOffset + dwSize > pData->dwTotalSize)
		return astrReturn;

	if (!dwOffset || !dwSize || dwNumStringBytes < sizeof(char))
		return astrReturn;

	DWORD dwMaxSize = dwSize;
	if (dwNumStringBytes - sizeof(char) < dwSize)
		dwMaxSize = dwNumStringBytes - sizeof(char);
	astrReturn.assign((const char*)(((LPBYTE)pData) + dwOffset), dwMaxSize / sizeof(char));
	astrReturn.TrimRight((char)0);
	return astrReturn;
}

#ifdef _UNICODE
#define VarInfoGetString VarInfoGetStringW
#else
#define VarInfoGetString VarInfoGetStringA
#endif

template <class TYPE> class ETSPVarStruct
{
public:
	ETSPVarStruct()
	{
		pData = NULL;
	}
	virtual ~ETSPVarStruct()
	{
		if (pData)
			free(pData);
		pData = NULL;
	}

	TYPE* pData;

	ETSPVarStruct<TYPE>& operator=(const ETSPVarStruct<TYPE>& mVarStruct)
	{
		if (this == &mVarStruct)
			return *this;

		if (ReAlloc(mVarStruct.pData->dwTotalSize))
			memcpy(pData, mVarStruct.pData, mVarStruct.pData->dwTotalSize);
		return *this;
	}

	TYPE* ReAlloc(size_t iSize)
	{
		ASSERT(iSize < 20000);

		if (!pData)
		{
			// nocurrent data
			if (!iSize)
			{
				// No size requested.
				return NULL;
			}

			pData = (TYPE*)calloc(1, iSize);
			if (pData)
				pData->dwTotalSize = (DWORD)iSize;
			return pData;
		}

		if (_msize(pData) == 0)
		{
			free(pData);
			pData = NULL;

			if (!iSize) // No size requested.
				return NULL;

			pData = (TYPE*)calloc(1, iSize);
			if (pData)
				pData->dwTotalSize = (DWORD)iSize;
			return pData;
		}

		TYPE* pData2 = (TYPE*)realloc(pData, iSize);
		if (pData2)
		{
			pData = pData2;
			pData->dwTotalSize = (DWORD)iSize;
		}

		return pData;
	}

	DWORD GetSize()
	{
		if (pData)
			return (DWORD)_msize(pData);

		return 0;
	}
};

#endif // !defined(AFX_ETSPVARSTRUCT_H__14ED531F_4F32_4154_AC32_65B8328ED808__INCLUDED_)
