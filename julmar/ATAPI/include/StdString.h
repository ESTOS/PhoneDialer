// =============================================================================
//  FILE:  StdString.h
//  AUTHOR:	Joe O'Leary (with outside help noted in comments)
//
//		If you find any bugs in this code, please let me know:
//
//				jmoleary@earthlink.net
//				http://www.joeo.net/stdstring.htm (a bit outdated)
//
//      The latest version of this code should always be available at the
//      following link:
//
//              http://www.joeo.net/code/StdString.zip (Dec 6, 2003)
//
//
//  REMARKS:
//		This header file declares the CStdStr template.  This template derives
//		the Standard C++ Library basic_string<> template and add to it the
//		the following conveniences:
//			- The full MFC CString set of functions (including implicit cast)
//			- writing to/reading from COM IStream interfaces
//			- Functional objects for use in STL algorithms
//
//		From this template, we intstantiate two classes:  CStdStringA and
//		CStdStringW.  The name "CStdString" is just a #define of one of these,
//		based upone the UNICODE macro setting
//
// COPYRIGHT:
//		2002 Joseph M. O'Leary.  This code is 100% free.  Use it anywhere you
//      want.  Rewrite it, restructure it, whatever.  If you can write software
//      that makes money off of it, good for you.  I kinda like capitalism. 
//      Please don't blame me if it causes your $30 billion dollar satellite
//      explode in orbit.  If you redistribute it in any form, I'd appreciate it
//      if you would leave this notice here.
// =============================================================================

// Avoid multiple inclusion

#ifndef STDSTRING_H
#define STDSTRING_H

// When using VC, turn off browser references
// Turn off unavoidable compiler warnings

#if defined(_MSC_VER) && (_MSC_VER > 1100)
#pragma component(browser, off, references, "CStdString")
//#pragma warning (disable : 4290) // C++ Exception Specification ignored
//#pragma warning (disable : 4127) // Conditional expression is constant
//#pragma warning (disable : 4097) // typedef name used as synonym for class name
/*
#ifndef _SCL_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#endif
*/
#endif

// Borland warnings to turn off

#ifdef __BORLANDC__
#pragma option push -w-inl
//	#pragma warn -inl   // Turn off inline function warnings
#endif


// MACRO: SS_ALLOWDEPECRECATES
// Allow Deprecated Functions
// #define SS_ALLOWDEPECRECATES


// MACRO: SS_NO_IMPLICIT_CAST
// --------------------------
//      Some people don't like the implicit cast to const char* (or rather to
//      const CT*) that CStdString (and MFC's CString) provide.  That was the
//      whole reason I created this class in the first place, but hey, whatever
//      bakes your cake.  Just #define this macro to get rid of the the implicit
//      cast.

//#define SS_NO_IMPLICIT_CAST // gets rid of operator const CT*()


// MACRO: SS_WIN32
// ---------------
//      When this flag is set, we are building code for the Win32 platform and
//      may use Win32 specific functions (such as LoadString).  This gives us
//      a couple of nice extras for the code.
//
//      Obviously, Microsoft's is not the only compiler available for Win32 out
//      there.  So I can't just check to see if _MSC_VER is defined to detect
//      if I'm building on Win32.  So for now, if you use MS Visual C++ or
//      Borland's compiler, I turn this on.  Otherwise you may turn it on
//      yourself, if you prefer

#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(_WIN32)
#define SS_WIN32
#endif

// MACRO: SS_ANSI
// --------------
//      When this macro is defined, the code attempts only to use ANSI/ISO
//      standard library functions to do it's work.  It will NOT attempt to use
//      any Win32 of Visual C++ specific functions -- even if they are
//      available.  You may define this flag yourself to prevent any Win32
//      of VC++ specific functions from being called. 

// If we're not on Win32, we MUST use an ANSI build

#ifndef SS_WIN32
#if !defined(SS_NO_ANSI)
#define SS_ANSI
#endif
#endif

// MACRO: SS_ALLOCA
// ----------------
//      Some implementations of the Standard C Library have a non-standard
//      function known as alloca().  This functions allows one to allocate a
//      variable amount of memory on the stack.  It is needed to implement
//      the ASCII/MBCS conversion macros.
//
//      I wanted to find some way to determine automatically if alloca() is
//		available on this platform via compiler flags but that is asking for
//		trouble.  The crude test presented here will likely need fixing on
//		other platforms.  Therefore I'll leave it up to you to fiddle with
//		this test to determine if it exists.  Just make sure SS_ALLOCA is or
//		is not defined as appropriate and you control this feature.

#if defined(_MSC_VER) && !defined(SS_ANSI)
#define SS_ALLOCA
#endif


// MACRO SS_NO_LOCALE
// ------------------
// If your implementation of the Standard C++ Library lacks the <locale> header,
// you can #define this macro to make your code build properly.  Note that this
// is some of my newest code and frankly I'm not very sure of it, though it does
// pass my unit tests.

// #define SS_NO_LOCALE


// Compiler Error regarding _UNICODE and UNICODE
// -----------------------------------------------
// Microsoft header files are screwy.  Sometimes they depend on a preprocessor 
// flag named "_UNICODE".  Other times they check "UNICODE" (note the lack of
// leading underscore in the second version".  In several places, they silently
// "synchronize" these two flags this by defining one of the other was defined. 
// In older version of this header, I used to try to do the same thing. 
//
// However experience has taught me that this is a bad idea.  You get weird
// compiler errors that seem to indicate things like LPWSTR and LPTSTR not being
// equivalent in UNICODE builds, stuff like that (when they MUST be in a proper
// UNICODE  build).  You end up scratching your head and saying, "But that HAS
// to compile!".
//
// So what should you do if you get this error?
//
// Make sure that both macros (_UNICODE and UNICODE) are defined before this
// file is included.  You can do that by either
//
//		a) defining both yourself before any files get included
//		b) including the proper MS headers in the proper order
//		c) including this file before any other file, uncommenting
//		   the #defines below, and commenting out the #errors
//
//	Personally I recommend solution a) but it's your call.

#ifdef _MSC_VER
#if defined (_UNICODE) && !defined (UNICODE)
#error UNICODE defined  but not UNICODE
//	#define UNICODE  // no longer silently fix this
#endif
#if defined (UNICODE) && !defined (_UNICODE)
#error Warning, UNICODE defined  but not _UNICODE
//	#define _UNICODE  // no longer silently fix this
#endif
#endif


// -----------------------------------------------------------------------------
// MIN and MAX.  The Standard C++ template versions go by so many names (at
// at least in the MS implementation) that you never know what's available 
// -----------------------------------------------------------------------------
template<class Type>
inline const Type& SSMIN(const Type& arg1, const Type& arg2)
{
	return arg2 < arg1 ? arg2 : arg1;
}
template<class Type>
inline const Type& SSMAX(const Type& arg1, const Type& arg2)
{
	return arg2 > arg1 ? arg2 : arg1;
}

// If they have not #included W32Base.h (part of my W32 utility library) then
// we need to define some stuff.  Otherwise, this is all defined there.

#if !defined(W32BASE_H)

// If they want us to use only standard C++ stuff (no Win32 stuff)

#ifdef SS_ANSI

// On Win32 we have TCHAR.H so just include it.  This is NOT violating
// the spirit of SS_ANSI as we are not calling any Win32 functions here.

#ifdef SS_WIN32

#include <TCHAR.H>
#include <WTYPES.H>
#ifndef STRICT
#define STRICT
#endif

// ... but on non-Win32 platforms, we must #define the types we need.

#else

typedef const char*		PCSTR;
typedef char*			PSTR;
typedef const wchar_t*	PCWSTR;
typedef wchar_t*		PWSTR;
#ifdef UNICODE
typedef wchar_t		TCHAR;
#else
typedef char		TCHAR;
#endif
typedef wchar_t			OLECHAR;

#endif	// #ifndef _WIN32


// Make sure ASSERT and verify are defined using only ANSI stuff

#ifndef ASSERT
#include <assert.h>
#define ASSERT(f) assert((f))
#endif
#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(x) ASSERT((x))
#else
#define VERIFY(x) x
#endif
#endif

#else // ...else SS_ANSI is NOT defined

#include <TCHAR.H>
#include <WTYPES.H>
#ifndef STRICT
#define STRICT
#endif

// Make sure ASSERT and verify are defined

#ifndef ASSERT
#include <crtdbg.h>
#define ASSERT(f) _ASSERTE((f))
#endif
#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(x) ASSERT((x))
#else
#define VERIFY(x) x
#endif
#endif

#endif // #ifdef SS_ANSI

#ifndef UNUSED
#define UNUSED(x) x
#endif

#endif // #ifndef W32BASE_H

// Standard headers needed

#include <string>			// basic_string
#include <algorithm>		// for_each, etc.
#include <functional>		// for StdStringLessNoCase, et al
#ifndef SS_NO_LOCALE
#include <locale>			// for various facets
#endif
#include <list>				// for containers
#include <set>				// for containers
#include <vector>			// for containers
#include <map>				// for containers

// If this is a recent enough version of VC include comdef.h, so we can write
// member functions to deal with COM types & compiler support classes e.g.
// _bstr_t

#if defined (_MSC_VER) && (_MSC_VER >= 1100)
#include <comdef.h>
//#define SS_INC_COMDEF		// signal that we #included MS comdef.h file
#define SS_NOTHROW __declspec(nothrow)
#else
#define SS_NOTHROW
#endif

#ifndef TRACE
#define TRACE_DEFINED_HERE
#define TRACE
#endif

// Microsoft defines PCSTR, PCWSTR, etc, but no PCTSTR.  I hate to use the
// versions with the "L" in front of them because that's a leftover from Win 16
// days, even though it evaluates to the same thing.  Therefore, Define a PCSTR
// as an LPCTSTR.

#if !defined(PCTSTR) && !defined(PCTSTR_DEFINED)
typedef const TCHAR*			PCTSTR;
#define PCTSTR_DEFINED
#endif

#if !defined(PCOLESTR) && !defined(PCOLESTR_DEFINED)
typedef const OLECHAR*			PCOLESTR;
#define PCOLESTR_DEFINED
#endif

#if !defined(POLESTR) && !defined(POLESTR_DEFINED)
typedef OLECHAR*				POLESTR;
#define POLESTR_DEFINED
#endif

#if !defined(PCUSTR) && !defined(PCUSTR_DEFINED)
typedef const unsigned char*	PCUSTR;
typedef unsigned char*			PUSTR;
#define PCUSTR_DEFINED
#endif


// SGI compiler 7.3 doesnt know these  types - oh and btw, remember to use
// -LANG:std in the CXX Flags
#if defined(__sgi)
typedef unsigned long           DWORD;
typedef void *                  LPCVOID;
#endif


// SS_USE_FACET macro and why we need it:
//
// Since I'm a good little Standard C++ programmer, I use locales.  Thus, I
// need to make use of the use_facet<> template function here.   Unfortunately,
// this need is complicated by the fact the MS' implementation of the Standard
// C++ Library has a non-standard version of use_facet that takes more
// arguments than the standard dictates.  Since I'm trying to write CStdString
// to work with any version of the Standard library, this presents a problem.
//
// The upshot of this is that I can't do 'use_facet' directly.  The MS' docs
// tell me that I have to use a macro, _USE() instead.  Since _USE obviously
// won't be available in other implementations, this means that I have to write
// my OWN macro -- SS_USE_FACET -- that evaluates either to _USE or to the
// standard, use_facet.
//
// If you are having trouble with the SS_USE_FACET macro, in your implementation
// of the Standard C++ Library, you can define your own version of SS_USE_FACET.

#ifndef schMSG
#define schSTR(x)	   #x
#define schSTR2(x)	schSTR(x)
#define schMSG(desc) message(__FILE__ "(" schSTR2(__LINE__) "):" #desc)
#endif

#ifndef SS_USE_FACET

// STLPort #defines a macro (__STL_NO_EXPLICIT_FUNCTION_TMPL_ARGS) for
// all MSVC builds, erroneously in my opinion.  It causes problems for
// my SS_ANSI builds.  In my code, I always comment out that line.  You'll
// find it in   \stlport\config\stl_msvc.h

#if defined(__SGI_STL_PORT) && (__SGI_STL_PORT >= 0x400 )

#if defined(__STL_NO_EXPLICIT_FUNCTION_TMPL_ARGS) && defined(_MSC_VER)
#ifdef SS_ANSI
#pragma schMSG(__STL_NO_EXPLICIT_FUNCTION_TMPL_ARGS defined!!)
#endif
#endif
#define SS_USE_FACET(loc, fac) std::use_facet<fac >(loc)

#elif defined(_MSC_VER )

#define SS_USE_FACET(loc, fac) std::use_facet<fac >(loc)

// ...and
#elif defined(_RWSTD_NO_TEMPLATE_ON_RETURN_TYPE)

#define SS_USE_FACET(loc, fac) std::use_facet(loc, (fac*)0)

#else

#define SS_USE_FACET(loc, fac) std::use_facet<fac >(loc)

#endif

#endif

// =============================================================================
// UNICODE/MBCS conversion macros.  Made to work just like the MFC/ATL ones.
// =============================================================================

#include <wchar.h>      // Added to Std Library with Amendment #1.

// First define the conversion helper functions.  We define these regardless of
// any preprocessor macro settings since their names won't collide. 

// Not sure if we need all these headers.   I believe ANSI says we do.

#include <stdio.h>
#include <stdarg.h>
#include <wctype.h>
#include <ctype.h>
#include <stdlib.h>
#ifndef va_start
#include <varargs.h>
#endif


#ifdef SS_NO_LOCALE

#if defined(_WIN32) || defined (_WIN32_WCE)

inline PWSTR StdCodeCvt(PWSTR pDstW, int nDst, PCSTR pSrcA, int nSrc,
	UINT acp = CP_ACP)
{
	ASSERT(0 != pSrcA);
	ASSERT(0 != pDstW);
	pDstW[0] = '\0';
	MultiByteToWideChar(acp, 0, pSrcA, nSrc, pDstW, nDst);
	return pDstW;
}
inline PWSTR StdCodeCvt(PWSTR pDstW, int nDst, PCUSTR pSrcA, int nSrc,
	UINT acp = CP_ACP)
{
	return StdCodeCvt(pDstW, nDst, (PCSTR)pSrcA, nSrc, acp);
}

inline PSTR StdCodeCvt(PSTR pDstA, int nDst, PCWSTR pSrcW, int nSrc,
	UINT acp = CP_ACP)
{
	ASSERT(0 != pDstA);
	ASSERT(0 != pSrcW);
	pDstA[0] = '\0';
	WideCharToMultiByte(acp, 0, pSrcW, nSrc, pDstA, nDst, 0, 0);
	return pDstA;
}
inline PUSTR StdCodeCvt(PUSTR pDstA, int nDst, PCWSTR pSrcW, int nSrc,
	UINT acp = CP_ACP)
{
	return (PUSTR)StdCodeCvt((PSTR)pDstA, nDst, pSrcW, nSrc, acp);
}
#else
#endif

#else

// StdCodeCvt - made to look like Win32 functions WideCharToMultiByte
//				and MultiByteToWideChar but uses locales in SS_ANSI
//				builds.  There are a number of overloads.
//              First argument is the destination buffer.
//              Second argument is the source buffer
//#if defined (SS_ANSI) || !defined (SS_WIN32)

// 'SSCodeCvt' - shorthand name for the codecvt facet we use

typedef std::codecvt<wchar_t, char, mbstate_t> SSCodeCvt;

inline PWSTR StdCodeCvt(PWSTR pDstW, int nDst, PCSTR pSrcA, int nSrc,
	const std::locale& loc = std::locale())
{
	ASSERT(0 != pSrcA);
	ASSERT(0 != pDstW);

	pDstW[0] = '\0';

	if (nSrc > 0)
	{
		PCSTR pNextSrcA = pSrcA;
		PWSTR pNextDstW = pDstW;
		SSCodeCvt::result res = SSCodeCvt::ok;
		const SSCodeCvt& conv = SS_USE_FACET(loc, SSCodeCvt);
		SSCodeCvt::state_type st = { 0 };
		res = conv.in(st,
			pSrcA, pSrcA + nSrc, pNextSrcA,
			pDstW, pDstW + nDst, pNextDstW);

		ASSERT(SSCodeCvt::ok == res);
		ASSERT(SSCodeCvt::error != res);
		ASSERT(pNextDstW >= pDstW);
		ASSERT(pNextSrcA >= pSrcA);

		// Null terminate the converted string

		if (pNextDstW - pDstW > nDst)
			*(pDstW + nDst) = '\0';
		else
			*pNextDstW = '\0';
	}
	return pDstW;
}
inline PWSTR StdCodeCvt(PWSTR pDstW, int nDst, PCUSTR pSrcA, int nSrc,
	const std::locale& loc = std::locale())
{
	return StdCodeCvt(pDstW, nDst, (PCSTR)pSrcA, nSrc, loc);
}

inline PSTR StdCodeCvt(PSTR pDstA, int nDst, PCWSTR pSrcW, int nSrc,
	const std::locale& loc = std::locale())
{
	ASSERT(0 != pDstA);
	ASSERT(0 != pSrcW);

	pDstA[0] = '\0';

	if (nSrc > 0)
	{
		PSTR pNextDstA = pDstA;
		PCWSTR pNextSrcW = pSrcW;
		SSCodeCvt::result res = SSCodeCvt::ok;
		const SSCodeCvt& conv = SS_USE_FACET(loc, SSCodeCvt);
		SSCodeCvt::state_type st = { 0 };
		res = conv.out(st,
			pSrcW, pSrcW + nSrc, pNextSrcW,
			pDstA, pDstA + nDst, pNextDstA);

		ASSERT(SSCodeCvt::error != res);
		ASSERT(SSCodeCvt::ok == res);	// strict, comment out for sanity
		ASSERT(pNextDstA >= pDstA);
		ASSERT(pNextSrcW >= pSrcW);

		// Null terminate the converted string

		if (pNextDstA - pDstA > nDst)
			*(pDstA + nDst) = '\0';
		else
			*pNextDstA = '\0';
	}
	return pDstA;
}

inline PUSTR StdCodeCvt(PUSTR pDstA, int nDst, PCWSTR pSrcW, int nSrc,
	const std::locale& loc = std::locale())
{
	return (PUSTR)StdCodeCvt((PSTR)pDstA, nDst, pSrcW, nSrc, loc);
}

#endif



// Unicode/MBCS conversion macros are only available on implementations of
// the "C" library that have the non-standard _alloca function.  As far as I
// know that's only Microsoft's though I've heard that the function exists
// elsewhere.  

#if defined(SS_ALLOCA) && !defined SS_NO_CONVERSION

#include <malloc.h>	// needed for _alloca

// Define our conversion macros to look exactly like Microsoft's to
// facilitate using this stuff both with and without MFC/ATL

#ifdef _CONVERSION_USES_THREAD_LOCALE

#ifndef _DEBUG
#define SSCVT int _cvt; _cvt; UINT _acp=GetACP(); \
			    _acp; PCWSTR _pw; _pw; PCSTR _pa; _pa
#else
#define SSCVT int _cvt = 0; _cvt; UINT _acp=GetACP();\
			     _acp; PCWSTR _pw=0; _pw; PCSTR _pa=0; _pa
#endif
#define SSA2W(pa) (\
		    ((_pa = pa) == 0) ? 0 : (\
			    _cvt = (sslen(_pa)),\
			    StdCodeCvt((PWSTR) _alloca((_cvt+1)*2), (_cvt+1)*2, \
							_pa, _cvt, _acp)))
#define SSW2A(pw) (\
		    ((_pw = pw) == 0) ? 0 : (\
			    _cvt = sslen(_pw), \
			    StdCodeCvt((LPSTR) _alloca((_cvt+1)*2), (_cvt+1)*2, \
					_pw, _cvt, _acp)))
#else

#ifndef _DEBUG
#define SSCVT int _cvt; _cvt; UINT _acp=CP_ACP; _acp;\
			     PCWSTR _pw; _pw; PCSTR _pa; _pa
#else
#define SSCVT int _cvt = 0; _cvt; UINT _acp=CP_ACP; \
			    _acp; PCWSTR _pw=0; _pw; PCSTR _pa=0; _pa
#endif
#define SSA2W(pa) (\
		    ((_pa = pa) == 0) ? 0 : (\
			    _cvt = (sslen(_pa)),\
			    StdCodeCvt((PWSTR) _alloca((_cvt+1)*2), (_cvt+1)*2, \
					_pa, _cvt)))
#define SSW2A(pw) (\
		    ((_pw = pw) == 0) ? 0 : (\
			    _cvt = (sslen(_pw)),\
			    StdCodeCvt((LPSTR) _alloca((_cvt+1)*2), (_cvt+1)*2, \
					_pw, _cvt)))
#endif

#define SSA2CW(pa) ((PCWSTR)SSA2W((pa)))
#define SSW2CA(pw) ((PCSTR)SSW2A((pw)))

#ifdef UNICODE
#define SST2A	SSW2A
#define SSA2T	SSA2W
#define SST2CA	SSW2CA
#define SSA2CT	SSA2CW
// (Did you get a compiler error here about not being able to convert
// PTSTR into PWSTR?  Then your _UNICODE and UNICODE flags are messed 
// up.  Best bet: #define BOTH macros before including any MS headers.)
inline PWSTR	SST2W(PTSTR p) { return p; }
inline PTSTR	SSW2T(PWSTR p) { return p; }
inline PCWSTR	SST2CW(PCTSTR p) { return p; }
inline PCTSTR	SSW2CT(PCWSTR p) { return p; }
#else
#define SST2W	SSA2W
#define SSW2T	SSW2A
#define SST2CW	SSA2CW
#define SSW2CT	SSW2CA
inline PSTR		SST2A(PTSTR p) { return p; }
inline PTSTR	SSA2T(PSTR p) { return p; }
inline PCSTR	SST2CA(PCTSTR p) { return p; }
inline PCTSTR	SSA2CT(PCSTR p) { return p; }
#endif // #ifdef UNICODE

#if defined(UNICODE)
// in these cases the default (TCHAR) is the same as OLECHAR
inline PCOLESTR	SST2COLE(PCTSTR p) { return p; }
inline PCTSTR	SSOLE2CT(PCOLESTR p) { return p; }
inline POLESTR	SST2OLE(PTSTR p) { return p; }
inline PTSTR	SSOLE2T(POLESTR p) { return p; }
#elif defined(OLE2ANSI)
// in these cases the default (TCHAR) is the same as OLECHAR
inline PCOLESTR	SST2COLE(PCTSTR p) { return p; }
inline PCTSTR	SSOLE2CT(PCOLESTR p) { return p; }
inline POLESTR	SST2OLE(PTSTR p) { return p; }
inline PTSTR	SSOLE2T(POLESTR p) { return p; }
#else
//CharNextW doesn't work on Win95 so we use this
#define SST2COLE(pa)	SSA2CW((pa))
#define SST2OLE(pa)		SSA2W((pa))
#define SSOLE2CT(po)	SSW2CA((po))
#define SSOLE2T(po)		SSW2A((po))
#endif

#ifdef OLE2ANSI
#define SSW2OLE		SSW2A
#define SSOLE2W		SSA2W
#define SSW2COLE	SSW2CA
#define SSOLE2CW	SSA2CW
inline POLESTR		SSA2OLE(PSTR p) { return p; }
inline PSTR			SSOLE2A(POLESTR p) { return p; }
inline PCOLESTR		SSA2COLE(PCSTR p) { return p; }
inline PCSTR		SSOLE2CA(PCOLESTR p) { return p; }
#else
#define SSA2OLE		SSA2W
#define SSOLE2A		SSW2A
#define SSA2COLE	SSA2CW
#define SSOLE2CA	SSW2CA
inline POLESTR		SSW2OLE(PWSTR p) { return p; }
inline PWSTR		SSOLE2W(POLESTR p) { return p; }
inline PCOLESTR		SSW2COLE(PCWSTR p) { return p; }
inline PCWSTR		SSOLE2CW(PCOLESTR p) { return p; }
#endif

// Above we've defined macros that look like MS' but all have
// an 'SS' prefix.  Now we need the real macros.  We'll either
// get them from the macros above or from MFC/ATL. 

#if defined (USES_CONVERSION)

#define _NO_STDCONVERSION	// just to be consistent

#else

#ifdef _MFC_VER

#include <afxconv.h>
#define _NO_STDCONVERSION // just to be consistent

#else

#define USES_CONVERSION SSCVT
#define A2CW			SSA2CW
#define W2CA			SSW2CA
#define T2A				SST2A
#define A2T				SSA2T
#define T2W				SST2W
#define W2T				SSW2T
#define T2CA			SST2CA
#define A2CT			SSA2CT
#define T2CW			SST2CW
#define W2CT			SSW2CT
#define ocslen			sslen
#define ocscpy			sscpy
#define T2COLE			SST2COLE
#define OLE2CT			SSOLE2CT
#define T2OLE			SST2COLE
#define OLE2T			SSOLE2CT
#define A2OLE			SSA2OLE
#define OLE2A			SSOLE2A
#define W2OLE			SSW2OLE
#define OLE2W			SSOLE2W
#define A2COLE			SSA2COLE
#define OLE2CA			SSOLE2CA
#define W2COLE			SSW2COLE
#define OLE2CW			SSOLE2CW

#endif // #ifdef _MFC_VER
#endif // #ifndef USES_CONVERSION
#endif // #ifndef SS_NO_CONVERSION

// Define ostring - generic name for std::basic_string<OLECHAR>

#if !defined(ostring) && !defined(OSTRING_DEFINED)
typedef std::basic_string<OLECHAR> ostring;
#define OSTRING_DEFINED
#endif

// StdCodeCvt when there's no conversion to be done
inline PSTR StdCodeCvt(PSTR pDst, int nDst, PCSTR pSrc, int nSrc)
{
	int nChars = SSMIN(nSrc, nDst);

	if (nChars > 0)
	{
		pDst[0] = '\0';
#if defined (_MSC_VER) && (_MSC_VER >= 1400)
		std::basic_string<char>::traits_type::_Copy_s(pDst, nDst, pSrc, nChars);
#else
		std::basic_string<char>::traits_type::copy(pDst, pSrc, nChars);
#endif
		//		std::char_traits<char>::copy(pDst, pSrc, nChars);
		pDst[nChars] = '\0';
	}

	return pDst;
}
inline PSTR StdCodeCvt(PSTR pDst, int nDst, PCUSTR pSrc, int nSrc)
{
	return StdCodeCvt(pDst, nDst, (PCSTR)pSrc, nSrc);
}
inline PUSTR StdCodeCvt(PUSTR pDst, int nDst, PCSTR pSrc, int nSrc)
{
	return (PUSTR)StdCodeCvt((PSTR)pDst, nDst, pSrc, nSrc);
}

inline PWSTR StdCodeCvt(PWSTR pDst, int nDst, PCWSTR pSrc, int nSrc)
{
	int nChars = SSMIN(nSrc, nDst);

	if (nChars > 0)
	{
		pDst[0] = '\0';
#if defined (_MSC_VER) && (_MSC_VER >= 1400)
		std::basic_string<wchar_t>::traits_type::_Copy_s(pDst, nDst, pSrc, nChars);
#else
		std::basic_string<wchar_t>::traits_type::copy(pDst, pSrc, nChars);
#endif
		//		std::char_traits<wchar_t>::copy(pDst, pSrc, nChars);
		pDst[nChars] = '\0';
	}

	return pDst;
}


// Define tstring -- generic name for std::basic_string<TCHAR>

#if !defined(tstring) && !defined(TSTRING_DEFINED)
typedef std::basic_string<TCHAR> tstring;
#define TSTRING_DEFINED
#endif

// =============================================================================
// INLINE FUNCTIONS ON WHICH CSTDSTRING RELIES
//
// Usually for generic text mapping, we rely on preprocessor macro definitions
// to map to string functions.  However the CStdStr<> template cannot use
// macro-based generic text mappings because its character types do not get
// resolved until template processing which comes AFTER macro processing.  In
// other words, the preprocessor macro UNICODE is of little help to us in the
// CStdStr template
//
// Therefore, to keep the CStdStr declaration simple, we have these inline
// functions.  The template calls them often.  Since they are inline (and NOT
// exported when this is built as a DLL), they will probably be resolved away
// to nothing. 
//
// Without these functions, the CStdStr<> template would probably have to broken
// out into two, almost identical classes.  Either that or it would be a huge,
// convoluted mess, with tons of "if" statements all over the place checking the
// size of template parameter CT.
// =============================================================================

#ifdef SS_NO_LOCALE

// --------------------------------------------------------------------------
// Win32 GetStringTypeEx wrappers
// --------------------------------------------------------------------------
inline bool wsGetStringType(LCID lc, DWORD dwT, PCSTR pS, int nSize,
	WORD* pWd)
{
	return FALSE != GetStringTypeExA(lc, dwT, pS, nSize, pWd);
}
inline bool wsGetStringType(LCID lc, DWORD dwT, PCWSTR pS, int nSize,
	WORD* pWd)
{
	return FALSE != GetStringTypeExW(lc, dwT, pS, nSize, pWd);
}


template<typename CT>
inline bool ssisspace(CT t)
{
	WORD toYourMother;
	return	wsGetStringType(GetThreadLocale(), CT_CTYPE1, &t, 1, &toYourMother)
		&& 0 != (C1_BLANK & toYourMother);
}

#endif

// -----------------------------------------------------------------------------
// sslen: strlen/wcslen wrappers
// -----------------------------------------------------------------------------
template<typename CT> inline int sslen(const CT* pT)
{
	return 0 == pT ? 0 : (int)std::basic_string<CT>::traits_type::length(pT);
	//	return 0 == pT ? 0 : std::char_traits<CT>::length(pT);
}
inline SS_NOTHROW int sslen(const std::string& s)
{
	return static_cast<int>(s.length());
}
inline SS_NOTHROW int sslen(const std::wstring& s)
{
	return static_cast<int>(s.length());
}

// -----------------------------------------------------------------------------
// sstolower/sstoupper -- convert characters to upper/lower case
// -----------------------------------------------------------------------------

#ifdef SS_NO_LOCALE
inline char sstoupper(char ch) { return (char)::toupper(ch); }
inline wchar_t sstoupper(wchar_t ch) { return (wchar_t)::towupper(ch); }
inline char sstolower(char ch) { return (char)::tolower(ch); }
inline wchar_t sstolower(wchar_t ch) { return (wchar_t)::tolower(ch); }
#else
template<typename CT>
inline CT sstolower(const CT& t, const std::locale& loc = std::locale())
{
	return std::tolower<CT>(t, loc);
}
template<typename CT>
inline CT sstoupper(const CT& t, const std::locale& loc = std::locale())
{
	return std::toupper<CT>(t, loc);
}
#endif

// -----------------------------------------------------------------------------
// ssasn: assignment functions -- assign "sSrc" to "sDst"
// -----------------------------------------------------------------------------
typedef std::string::size_type		SS_SIZETYPE; // just for shorthand, really
typedef std::string::pointer		SS_PTRTYPE;
typedef std::wstring::size_type		SW_SIZETYPE;
typedef std::wstring::pointer		SW_PTRTYPE;

inline void	ssasn(std::string& sDst, const std::string& sSrc)
{
	if (sDst.c_str() != sSrc.c_str())
	{
		sDst.erase();
		sDst.assign(sSrc);
	}
}
inline void	ssasn(std::string& sDst, PCSTR pA)
{
	// Watch out for NULLs, as always.

	if (0 == pA)
	{
		sDst.erase();
	}

	// If pA actually points to part of sDst, we must NOT erase(), but
	// rather take a substring

	else if (pA >= sDst.c_str() && pA <= sDst.c_str() + sDst.size())
	{
		sDst = sDst.substr(static_cast<SS_SIZETYPE>(pA - sDst.c_str()));
	}

	// Otherwise (most cases) apply the assignment bug fix, if applicable
	// and do the assignment

	else
	{
		sDst.assign(pA);
	}
}

inline void ssasn(std::string& sDst, const std::wstring& sSrc)
{
	if (sSrc.empty())
	{
		sDst.erase();
	}
	else
	{
		int nDst = static_cast<int>(sSrc.size());
		sDst.resize(nDst + 1);
		StdCodeCvt(const_cast<SS_PTRTYPE>(sDst.data()), nDst,
			sSrc.c_str(), static_cast<int>(sSrc.size()));

		sDst.resize(sSrc.size());
	}
}

inline void ssasn(std::string& sDst, PCWSTR pW)
{
	int nSrc = sslen(pW);
	if (nSrc > 0)
	{
		//CG unneeded!
		//int nSrc	= sslen(pW);
		int nDst = nSrc;

		sDst.resize(nDst + 1);
		StdCodeCvt(const_cast<SS_PTRTYPE>(sDst.data()), nDst,
			pW, nSrc);

		sDst.resize(nDst);
	}
	else
	{
		sDst.erase();
	}
}

inline void ssasn(std::string& sDst, const int nNull)
{
	UNUSED(nNull);
	ASSERT(nNull == 0);
	sDst.assign("");
}

inline void ssasn(std::wstring& sDst, const std::wstring& sSrc)
{
	if (sDst.c_str() != sSrc.c_str())
	{
		sDst.erase();
		sDst.assign(sSrc);
	}
}

inline void ssasn(std::wstring& sDst, PCWSTR pW)
{
	// Watch out for NULLs, as always.

	if (0 == pW)
	{
		sDst.erase();
	}

	// If pW actually points to part of sDst, we must NOT erase(), but
	// rather take a substring

	else if (pW >= sDst.c_str() && pW <= sDst.c_str() + sDst.size())
	{
		sDst = sDst.substr(static_cast<SW_SIZETYPE>(pW - sDst.c_str()));
	}

	// Otherwise (most cases) apply the assignment bug fix, if applicable
	// and do the assignment

	else
	{
		sDst.assign(pW);
	}
}
#undef StrSizeType
inline void ssasn(std::wstring& sDst, const std::string& sSrc)
{
	if (sSrc.empty())
	{
		sDst.erase();
	}
	else
	{
		int nSrc = static_cast<int>(sSrc.size());
		int nDst = nSrc;

		sDst.resize(nSrc + 1);
		PCWSTR szCvt = StdCodeCvt(const_cast<SW_PTRTYPE>(sDst.data()), nDst,
			sSrc.c_str(), nSrc);

		sDst.resize(sslen(szCvt));
	}
}

inline void ssasn(std::wstring& sDst, PCSTR pA)
{
	int nSrc = sslen(pA);

	if (0 == nSrc)
	{
		sDst.erase();
	}
	else
	{
		int nDst = nSrc;
		sDst.resize(nDst + 1);
		PCWSTR szCvt = StdCodeCvt(const_cast<SW_PTRTYPE>(sDst.data()), nDst, pA,
			nSrc);

		sDst.resize(sslen(szCvt));
	}
}

inline void ssasn(std::wstring& sDst, const int nNull)
{
	UNUSED(nNull);
	ASSERT(nNull == 0);
	sDst.assign(L"");
}

//UNICODE / ANSI Converter
inline void EStringToUnicode(const std::string& sSrc, std::wstring& sDst)
{
	if (sSrc.empty())
	{
		sDst.erase();
	}
	else
	{
		int nSrc = static_cast<int>(sSrc.size());
		int nDst = nSrc;

		sDst.resize(nSrc + 1);
		PCWSTR szCvt = StdCodeCvt(const_cast<SW_PTRTYPE>(sDst.data()), nDst,
			sSrc.c_str(), nSrc);

		sDst.resize(sslen(szCvt));
	}
}

inline void EStringToUnicode(const char* pA, std::wstring& sDst)
{
	int nSrc = sslen(pA);
	if (0 == nSrc)
	{
		sDst.erase();
	}
	else
	{
		int nDst = nSrc;
		sDst.resize(nDst + 1);
		PCWSTR szCvt = StdCodeCvt(const_cast<SW_PTRTYPE>(sDst.data()), nDst, pA,
			nSrc);

		sDst.resize(sslen(szCvt));
	}
}

inline void EStringToAnsi(const std::wstring& sSrc, std::string& sDst)
{
	if (sSrc.empty())
	{
		sDst.erase();
	}
	else
	{
		int nDst = static_cast<int>(sSrc.size());
		sDst.resize(nDst + 1);
		StdCodeCvt(const_cast<SS_PTRTYPE>(sDst.data()), nDst,
			sSrc.c_str(), static_cast<int>(sSrc.size()));

		sDst.resize(sSrc.size());
	}
}

inline void EStringToAnsi(const wchar_t* pW, std::string& sDst)
{
	int nSrc = sslen(pW);
	if (nSrc > 0)
	{
		int nDst = nSrc;

		sDst.resize(nDst + 1);
		StdCodeCvt(const_cast<SS_PTRTYPE>(sDst.data()), nDst,
			pW, nSrc);

		sDst.resize(nDst);
	}
	else
	{
		sDst.erase();
	}
}


// -----------------------------------------------------------------------------
// ssadd: string object concatenation -- add second argument to first
// -----------------------------------------------------------------------------
#ifdef SS_ALLOWDEPECRECATES
[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]] inline void ssadd(std::string& sDst, const std::wstring& sSrc)
{
	int nSrc = static_cast<int>(sSrc.size());

	if (nSrc > 0)
	{
		int nDst = static_cast<int>(sDst.size());
		int nAdd = nSrc;

		sDst.resize(nDst + nAdd + 1);
		PCSTR szCvt = StdCodeCvt(const_cast<SS_PTRTYPE>(sDst.data() + nDst),
			nAdd, sSrc.c_str(), nSrc);

		sDst.resize(nDst + nAdd);
	}
}
#endif

inline void ssadd(std::string& sDst, const std::string& sSrc)
{
	sDst += sSrc;
}

#ifdef SS_ALLOWDEPECRECATES
[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]] inline void ssadd(std::string& sDst, PCWSTR pW)
{
	int nSrc = sslen(pW);
	if (nSrc > 0)
	{
		int nDst = static_cast<int>(sDst.size());
		int nAdd = nSrc;

		sDst.resize(nDst + nAdd + 1);
		PCSTR szCvt = StdCodeCvt(const_cast<SS_PTRTYPE>(sDst.data() + nDst),
			nAdd, pW, nSrc);

		sDst.resize(nDst + nSrc);
	}
}
#endif

inline void ssadd(std::string& sDst, PCSTR pA)
{
	if (pA)
	{
		// If the string being added is our internal string or a part of our
		// internal string, then we must NOT do any reallocation without
		// first copying that string to another object (since we're using a
		// direct pointer)

		if (pA >= sDst.c_str() && pA <= sDst.c_str() + sDst.length())
		{
			if (sDst.capacity() <= sDst.size() + sslen(pA))
				sDst.append(std::string(pA));
			else
				sDst.append(pA);
		}
		else
		{
			sDst.append(pA);
		}
	}
}

inline void ssadd(std::wstring& sDst, const std::wstring& sSrc)
{
	sDst += sSrc;
}

#ifdef SS_ALLOWDEPECRECATES
[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]] inline void ssadd(std::wstring& sDst, const std::string& sSrc)
{
	if (!sSrc.empty())
	{
		int nSrc = static_cast<int>(sSrc.size());
		int nDst = static_cast<int>(sDst.size());

		sDst.resize(nDst + nSrc + 1);
		PCWSTR szCvt = StdCodeCvt(const_cast<SW_PTRTYPE>(sDst.data() + nDst),
			nSrc, sSrc.c_str(), nSrc + 1);

		sDst.resize(nDst + nSrc);
	}
}
#endif

#ifdef SS_ALLOWDEPECRECATES
[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]] inline void ssadd(std::wstring& sDst, PCSTR pA)
{
	int nSrc = sslen(pA);

	if (nSrc > 0)
	{
		int nDst = static_cast<int>(sDst.size());

		sDst.resize(nDst + nSrc + 1);
		PCWSTR szCvt = StdCodeCvt(const_cast<SW_PTRTYPE>(sDst.data() + nDst),
			nSrc, pA, nSrc + 1);

		sDst.resize(nDst + nSrc);
	}
}
#endif

inline void ssadd(std::wstring& sDst, PCWSTR pW)
{
	if (pW)
	{
		// If the string being added is our internal string or a part of our
		// internal string, then we must NOT do any reallocation without
		// first copying that string to another object (since we're using a
		// direct pointer)

		if (pW >= sDst.c_str() && pW <= sDst.c_str() + sDst.length())
		{
			if (sDst.capacity() <= sDst.size() + sslen(pW))
				sDst.append(std::wstring(pW));
			else
				sDst.append(pW);
		}
		else
		{
			sDst.append(pW);
		}
	}
}


// -----------------------------------------------------------------------------
// sscmp: comparison (case sensitive, not affected by locale)
// -----------------------------------------------------------------------------
template<typename CT>
inline int sscmp(const CT* pA1, const CT* pA2)
{
	CT f;
	CT l;

	do
	{
		f = *(pA1++);
		l = *(pA2++);
	} while ((f) && (f == l));

	return (int)(f - l);
}

// -----------------------------------------------------------------------------
// ssicmp: comparison (case INsensitive, not affected by locale)
// -----------------------------------------------------------------------------
template<typename CT>
inline int ssicmp(const CT* pA1, const CT* pA2)
{
	// Using the "C" locale = "not affected by locale"

	std::locale loc = std::locale::classic();
	const std::ctype<CT>& ct = SS_USE_FACET(loc, std::ctype<CT>);
	CT f;
	CT l;

	do
	{
		f = ct.tolower(*(pA1++));
		l = ct.tolower(*(pA2++));
	} while ((f) && (f == l));

	return (int)(f - l);
}

// -----------------------------------------------------------------------------
// ssupr/sslwr: Uppercase/Lowercase conversion functions
// -----------------------------------------------------------------------------

template<typename CT>
inline void sslwr(CT* pT, size_t nLen, const std::locale& loc = std::locale())
{
	SS_USE_FACET(loc, std::ctype<CT>).tolower(pT, pT + nLen);
}
template<typename CT>
inline void ssupr(CT* pT, size_t nLen, const std::locale& loc = std::locale())
{
	SS_USE_FACET(loc, std::ctype<CT>).toupper(pT, pT + nLen);
}

// -----------------------------------------------------------------------------
// sscoll/ssicoll: Collation wrappers
//		Note -- with MSVC I have reversed the arguments order here because the
//		functions appear to return the opposite of what they should
// -----------------------------------------------------------------------------
#ifndef SS_NO_LOCALE
template <typename CT>
inline int sscoll(const CT* sz1, int nLen1, const CT* sz2, int nLen2)
{
	const std::collate<CT>& coll =
		SS_USE_FACET(std::locale(), std::collate<CT>);

	return coll.compare(sz2, sz2 + nLen2, sz1, sz1 + nLen1);
}
template <typename CT>
inline int ssicoll(const CT* sz1, int nLen1, const CT* sz2, int nLen2, const std::locale& loc)
{
	const std::collate<CT>& coll = SS_USE_FACET(loc, std::collate<CT>);

	// Some implementations seem to have trouble using the collate<>
	// facet typedefs so we'll just default to basic_string and hope
	// that's what the collate facet uses (which it generally should)

	//	std::collate<CT>::string_type s1(sz1);
	//	std::collate<CT>::string_type s2(sz2);
	const std::basic_string<CT> sEmpty;
	std::basic_string<CT> s1(sz1 ? sz1 : sEmpty.c_str());
	std::basic_string<CT> s2(sz2 ? sz2 : sEmpty.c_str());

	sslwr(const_cast<CT*>(s1.c_str()), nLen1, loc);
	sslwr(const_cast<CT*>(s2.c_str()), nLen2, loc);
	return coll.compare(s2.c_str(), s2.c_str() + nLen2,
		s1.c_str(), s1.c_str() + nLen1);
}
template <typename CT>
inline int ssicoll(const CT* sz1, int nLen1, const CT* sz2, int nLen2)
{
	const std::locale loc;
	return ssicoll(sz1, nLen1, sz2, nLen2, loc);
}
#endif


// FUNCTION: sscpy.  Copies up to 'nMax' characters from pSrc to pDst.
// -----------------------------------------------------------------------------
// FUNCTION:  sscpy
//		inline int sscpy(PSTR pDst, PCSTR pSrc, int nMax=-1);
//		inline int sscpy(PUSTR pDst,  PCSTR pSrc, int nMax=-1)
//		inline int sscpy(PSTR pDst, PCWSTR pSrc, int nMax=-1);
//		inline int sscpy(PWSTR pDst, PCWSTR pSrc, int nMax=-1);
//		inline int sscpy(PWSTR pDst, PCSTR pSrc, int nMax=-1);
//
// DESCRIPTION:
//		This function is very much (but not exactly) like strcpy.  These
//		overloads simplify copying one C-style string into another by allowing
//		the caller to specify two different types of strings if necessary.
//
//		The strings must NOT overlap
//
//		"Character" is expressed in terms of the destination string, not
//		the source.  If no 'nMax' argument is supplied, then the number of
//		characters copied will be sslen(pSrc).  A NULL terminator will
//		also be added so pDst must actually be big enough to hold nMax+1
//		characters.  The return value is the number of characters copied,
//		not including the NULL terminator.
//
// PARAMETERS: 
//		pSrc - the string to be copied FROM.  May be a char based string, an
//			   MBCS string (in Win32 builds) or a wide string (wchar_t).
//		pSrc - the string to be copied TO.  Also may be either MBCS or wide
//		nMax - the maximum number of characters to be copied into szDest.  Note
//			   that this is expressed in whatever a "character" means to pDst.
//			   If pDst is a wchar_t type string than this will be the maximum
//			   number of wchar_ts that my be copied.  The pDst string must be
//			   large enough to hold least nMaxChars+1 characters.
//			   If the caller supplies no argument for nMax this is a signal to
//			   the routine to copy all the characters in pSrc, regardless of
//			   how long it is.
//
// RETURN VALUE: none
// -----------------------------------------------------------------------------
template<typename CT1, typename CT2>
inline int sscpycvt(CT1* pDst, const CT2* pSrc, int nMax)
{
	// Note -- we assume pDst is big enough to hold pSrc.  If not, we're in
	// big trouble.  No bounds checking.  Caveat emptor.

	int nSrc = sslen(pSrc);

	const CT1* szCvt = StdCodeCvt(pDst, nMax, pSrc, nSrc);

	// If we're copying the same size characters, then all the "code convert"
	// just did was basically memcpy so the #of characters copied is the same
	// as the number requested.  I should probably specialize this function
	// template to achieve this purpose as it is silly to do a runtime check
	// of a fact known at compile time.  I'll get around to it.

	return sslen(szCvt);
}

inline int sscpycvt(PSTR pDst, PCSTR pSrc, int nMax)
{
	int nCount = nMax;
	for (; nCount > 0 && *pSrc; ++pSrc, ++pDst, --nCount)
		std::basic_string<char>::traits_type::assign(*pDst, *pSrc);

	*pDst = '\0';
	return nMax - nCount;
}
inline int sscpycvt(PWSTR pDst, PCWSTR pSrc, int nMax)
{
	int nCount = nMax;
	for (; nCount > 0 && *pSrc; ++pSrc, ++pDst, --nCount)
		std::basic_string<wchar_t>::traits_type::assign(*pDst, *pSrc);

	*pDst = L'\0';
	return nMax - nCount;
}
inline int sscpycvt(PWSTR pDst, PCSTR pSrc, int nMax)
{
	// Note -- we assume pDst is big enough to hold pSrc.  If not, we're in
	// big trouble.  No bounds checking.  Caveat emptor.

	const PWSTR szCvt = StdCodeCvt(pDst, nMax, pSrc, nMax);
	return sslen(szCvt);
}

template<typename CT1, typename CT2>
inline int sscpy(CT1* pDst, const CT2* pSrc, int nMax, int nLen)
{
	return sscpycvt(pDst, pSrc, SSMIN(nMax, nLen));
}
template<typename CT1, typename CT2>
inline int sscpy(CT1* pDst, const CT2* pSrc, int nMax)
{
	return sscpycvt(pDst, pSrc, SSMIN(nMax, sslen(pSrc)));
}
template<typename CT1, typename CT2>
inline int sscpy(CT1* pDst, const CT2* pSrc)
{
	return sscpycvt(pDst, pSrc, sslen(pSrc));
}
template<typename CT1, typename CT2>
inline int sscpy(CT1* pDst, const std::basic_string<CT2>& sSrc, int nMax)
{
	return sscpycvt(pDst, sSrc.c_str(), SSMIN(nMax, (int)sSrc.length()));
}
template<typename CT1, typename CT2>
inline int sscpy(CT1* pDst, const std::basic_string<CT2>& sSrc)
{
	return sscpycvt(pDst, sSrc.c_str(), (int)sSrc.length());
}

#ifdef SS_INC_COMDEF
template<typename CT1>
inline int sscpy(CT1* pDst, const _bstr_t& bs, int nMax)
{
	return sscpycvt(pDst, static_cast<PCOLESTR>(bs),
		SSMIN(nMax, static_cast<int>(bs.length())));
}
template<typename CT1>
inline int sscpy(CT1* pDst, const _bstr_t& bs)
{
	return sscpy(pDst, bs, static_cast<int>(bs.length()));
}
#endif


// -----------------------------------------------------------------------------
// Functional objects for changing case.  They also let you pass locales
// -----------------------------------------------------------------------------

#ifdef SS_NO_LOCALE
template<typename CT>
struct SSToUpper : public std::unary_function<CT, CT>
{
	inline CT operator()(const CT& t) const
	{
		return sstoupper(t);
	}
};
template<typename CT>
struct SSToLower : public std::unary_function<CT, CT>
{
	inline CT operator()(const CT& t) const
	{
		return sstolower(t);
	}
};
#else
template<typename CT>
struct SSToUpper : public std::binary_function<CT, std::locale, CT>
{
	inline CT operator()(const CT& t, const std::locale& loc) const
	{
		return sstoupper<CT>(t, loc);
	}
};
template<typename CT>
struct SSToLower : public std::binary_function<CT, std::locale, CT>
{
	inline CT operator()(const CT& t, const std::locale& loc) const
	{
		return sstolower<CT>(t, loc);
	}
};
#endif

// This struct is used for TrimRight() and TrimLeft() function implementations.
//template<typename CT>
//struct NotSpace : public std::unary_function<CT, bool>
//{
//	const std::locale& loc;
//	inline NotSpace(const std::locale& locArg) : loc(locArg) {}
//	inline bool operator() (CT t) { return !std::isspace(t, loc); }
//};
template<typename CT>
struct NotSpace : public std::unary_function<CT, bool>
{
	// DINKUMWARE BUG:
	// Note -- using std::isspace in a COM DLL gives us access violations
	// because it causes the dynamic addition of a function to be called
	// when the library shuts down.  Unfortunately the list is maintained
	// in DLL memory but the function is in static memory.  So the COM DLL
	// goes away along with the function that was supposed to be called,
	// and then later when the DLL CRT shuts down it unloads the list and
	// tries to call the long-gone function.
	// This is DinkumWare's implementation problem.  If you encounter this
	// problem, you may replace the calls here with good old isspace() and
	// iswspace() from the CRT unless they specify SS_ANSI

#ifdef SS_NO_LOCALE

	bool operator() (CT t) const { return !ssisspace(t); }

#else
	const std::locale loc;
	NotSpace(const std::locale& locArg = std::locale()) : loc(locArg) {}
	bool operator() (CT t) const { return !std::isspace(t, loc); }
#endif
};




//			Now we can define the template (finally!)
// =============================================================================
// TEMPLATE: CStdStr
//		template<typename CT> class CStdStr : public std::basic_string<CT>
//
// REMARKS:
//		This template derives from basic_string<CT> and adds some MFC CString-
//		like functionality
//
//		Basically, this is my attempt to make Standard C++ library strings as
//		easy to use as the MFC CString class.
//
//		Note that although this is a template, it makes the assumption that the
//		template argument (CT, the character type) is either char or wchar_t.  
// =============================================================================

//#define CStdStr _SS	// avoid compiler warning 4786

//    template<typename ARG> ARG& FmtArg(ARG& arg)  { return arg; }
//    PCSTR  FmtArg(const std::string& arg)  { return arg.c_str(); }
//    PCWSTR FmtArg(const std::wstring& arg) { return arg.c_str(); }

template<typename ARG>
struct FmtArg
{
	explicit FmtArg(const ARG& arg) : a_(arg) {}
	const ARG& operator()() const { return a_; }
	const ARG& a_;
private:
	FmtArg& operator=(const FmtArg&) { return *this; }
};

template<typename CT, typename OTHERCT>
class CStdStr : public std::basic_string<CT>
{
	// Typedefs for shorter names.  Using these names also appears to help
	// us avoid some ambiguities that otherwise arise on some platforms

	#define MYBASE std::basic_string<CT>				 // my base class
	#define OTHERBASE std::basic_string<OTHERCT>				 // the other (not allowed base)
	//typedef typename std::basic_string<CT>		MYBASE;	 // my base class
	typedef CStdStr<CT, OTHERCT>				MYTYPE;	 // myself
	typedef typename MYBASE::const_pointer		PCMYSTR; // PCSTR or PCWSTR 
	typedef typename MYBASE::pointer			PMYSTR;	 // PSTR or PWSTR
	typedef typename MYBASE::iterator			MYITER;  // my iterator type
	typedef typename MYBASE::const_iterator		MYCITER; // you get the idea...
	typedef typename MYBASE::reverse_iterator	MYRITER;
	typedef typename MYBASE::size_type			MYSIZE;
	typedef typename MYBASE::value_type			MYVAL;
	typedef typename MYBASE::allocator_type		MYALLOC;

public:

	using MYBASE::basic_string;

	// CStdStr inline constructors
	CStdStr()
	{
	}

	CStdStr(const MYTYPE& str) : MYBASE(str)
	{
	}

	CStdStr(MYTYPE&& str) : MYBASE(std::move(str))
	{
	}

	CStdStr(const MYBASE& str) : MYBASE(str)
	{
	}

	CStdStr(MYBASE&& str) : MYBASE(std::move(str))
	{
	}

	#ifdef SS_ALLOWDEPECRECATES
	//[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
	CStdStr(const OTHERBASE& str)
	{
		*this = str;
		//ssasn(*this, str);
	}
	#endif
	CStdStr(PCMYSTR pT, MYSIZE n) : MYBASE(pT, n)
	{
	}

	CStdStr(const CT* p)
	{
		ssasn(*this, p);
	}
	
	#ifdef SS_ALLOWDEPECRECATES
	//[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
	CStdStr(const OTHERCT* p)
	{
		*this = p;
		//ssasn(*this, p);
	}
	#endif
	
	CStdStr(MYCITER first, MYCITER last)
		: MYBASE(first, last)
	{
	}

	CStdStr(MYSIZE nSize, MYVAL ch, const MYALLOC& al = MYALLOC())
		: MYBASE(nSize, ch, al)
	{
	}

#ifdef SS_INC_COMDEF
	CStdStr(const _bstr_t& bstr)
	{
		if (bstr.length() > 0)
			this->append(static_cast<PCMYSTR>(bstr), bstr.length());
	}
#endif

	// CStdStr inline assignment operators -- the ssasn function now takes care
	MYTYPE& operator=(const MYTYPE& str)
	{
		__super::operator=(str);
		return *this;
	}

	//move assignment operator
	MYTYPE& operator=(MYTYPE&& str)
	{
		__super::operator=(std::move(str));
		return *this;
	}

	//move assignment operator
	MYTYPE& operator=(MYBASE&& str)
	{
		__super::operator=(std::move(str));
		return *this;
	}

	MYTYPE& operator=(const MYBASE& str)
	{
		__super::operator=(str);
		return *this;
	}

	#ifdef SS_ALLOWDEPECRECATES
	[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
	MYTYPE& operator=(const OTHERBASE& str)
	{
		ssasn(*this, str);
		return *this;
	}
	#endif

	MYTYPE& operator=(const CT* p)
	{
		ssasn(*this, p);
		return *this;
	}

	#ifdef SS_ALLOWDEPECRECATES
	[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
	MYTYPE& operator=(const OTHERCT* p)
	{
		ssasn(*this, p);
		return *this;
	}
	#endif

	MYTYPE& operator=(CT t)
	{
		this->assign(1, t);
		return *this;
	}

#ifdef SS_INC_COMDEF
	MYTYPE& operator=(const _bstr_t& bstr)
	{
		if (bstr.length() > 0)
		{
			this->assign(static_cast<PCMYSTR>(bstr), bstr.length());
			return *this;
		}
		else
		{
			this->erase();
			return *this;
		}
	}
#endif


	// -------------------------------------------------------------------------
	// CStdStr inline concatenation.
	// -------------------------------------------------------------------------
	MYTYPE& operator+=(const MYTYPE& str)
	{
		__super::operator+=(str);
		return *this;
	}

	MYTYPE& operator+=(const CT* str)
	{
		ssadd(*this, str); 
		return *this;
	}

	#ifdef SS_ALLOWDEPECRECATES
	[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
	MYTYPE& operator+=(const OTHERBASE& str)
	{
		ssadd(*this, str);
		return *this;
	}

	[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
	MYTYPE& operator+=(const OTHERCT* pA)
	{
		ssadd(*this, pA);
		return *this;
	}
	#endif

	MYTYPE& operator+=(CT t)
	{
		this->append(1, t);
		return *this;
	}
	
#ifdef SS_INC_COMDEF	// if we have _bstr_t, define a += for it too.
	MYTYPE& operator+=(const _bstr_t& bstr)
	{
		return this->operator+=(static_cast<PCMYSTR>(bstr));
	}
#endif

	// -------------------------------------------------------------------------
	// Case changing functions
	// -------------------------------------------------------------------------

	MYTYPE& ToUpper(const std::locale& loc = std::locale())
	{
		// Note -- if there are any MBCS character sets in which the lowercase
		// form a character takes up a different number of bytes than the
		// uppercase form, this would probably not work...

		std::transform(this->begin(),
			this->end(),
			this->begin(),
#ifdef SS_NO_LOCALE
			SSToUpper<CT>());
#else
			std::bind2nd(SSToUpper<CT>(), loc));
#endif

		// ...but if it were, this would probably work better.  Also, this way
		// seems to be a bit faster when anything other then the "C" locale is
		// used...

		//		if ( !empty() )
		//		{
		//			ssupr(this->GetBuf(), this->size(), loc);
		//			this->RelBuf();
		//		}

		return *this;
	}

	MYTYPE& ToLower(const std::locale& loc = std::locale())
	{
		// Note -- if there are any MBCS character sets in which the lowercase
		// form a character takes up a different number of bytes than the
		// uppercase form, this would probably not work...

		std::transform(this->begin(),
			this->end(),
			this->begin(),
#ifdef SS_NO_LOCALE
			SSToLower<CT>());
#else
			std::bind2nd(SSToLower<CT>(), loc));
#endif

		// ...but if it were, this would probably work better.  Also, this way
		// seems to be a bit faster when anything other then the "C" locale is
		// used...

		//		if ( !empty() )
		//		{
		//			sslwr(this->GetBuf(), this->size(), loc);
		//			this->RelBuf();
		//		}
		return *this;
	}


	MYTYPE& Normalize()
	{
		return Trim().ToLower();
	}


	// -------------------------------------------------------------------------
	// CStdStr -- Direct access to character buffer.  In the MS' implementation,
	// the at() function that we use here also calls _Freeze() providing us some
	// protection from multithreading problems associated with ref-counting.
	// In VC 7 and later, of course, the ref-counting stuff is gone.
	// -------------------------------------------------------------------------

	CT* GetBuf(int nMinLen = -1)
	{
		if (static_cast<int>(this->size()) < nMinLen)
			this->resize(static_cast<MYSIZE>(nMinLen));

		return this->empty() ? const_cast<CT*>(this->data()) : &(this->at(0));
	}

	CT* SetBuf(int nLen)
	{
		nLen = (nLen > 0 ? nLen : 0);
		if (this->capacity() < 1 && nLen == 0)
			this->resize(1);

		this->resize(static_cast<MYSIZE>(nLen));
		return const_cast<CT*>(this->data());
	}
	void RelBuf(int nNewLen = -1)
	{
		this->resize(static_cast<MYSIZE>(nNewLen > -1 ? nNewLen :
			sslen(this->c_str())));
	}

	void BufferRel() { RelBuf(); }			// backwards compatability
	CT*  Buffer() { return GetBuf(); }	// backwards compatability
	CT*  BufferSet(int nLen) { return SetBuf(nLen); }// backwards compatability

	bool Equals(const CT* pT, bool bUseCase = false) const
	{
		return  0 == (bUseCase ? this->compare(pT) : ssicmp(this->c_str(), pT));
	}


	// -------------------------------------------------------------------------
	// CString Facade Functions:
	//
	// The following methods are intended to allow you to use this class as a
	// near drop-in replacement for CString.
	// -------------------------------------------------------------------------
#ifdef SS_WIN32
	BSTR AllocSysString() const
	{
		ostring os;
		ssasn(os, *this);
		return ::SysAllocString(os.c_str());
	}
#endif

#ifndef SS_NO_LOCALE
	int Collate(PCMYSTR szThat) const
	{
		return sscoll(this->c_str(), (int)this->length(), szThat, (int)sslen(szThat));
	}

	int CollateNoCase(PCMYSTR szThat) const
	{
		return ssicoll(this->c_str(), (int)this->length(), szThat, (int)sslen(szThat));
	}

	int CollateNoCase(PCMYSTR szThat, const std::locale& loc) const
	{
		return ssicoll(this->c_str(), (int)this->length(), szThat, (int)sslen(szThat), loc);
	}
#endif
	int Compare(PCMYSTR szThat) const
	{
		return this->compare(szThat);
	}

	int CompareNoCase(PCMYSTR szThat)	const
	{
		return ssicmp(this->c_str(), szThat);
	}

	int Delete(int nIdx, int nCount = 1)
	{
		if (nIdx < 0)
			nIdx = 0;

		if (nIdx < this->GetLength())
			this->erase(static_cast<MYSIZE>(nIdx), static_cast<MYSIZE>(nCount));

		return GetLength();
	}

	void Empty()
	{
		this->erase();
	}

	int Find(CT ch) const
	{
		MYSIZE nIdx = this->find_first_of(ch);
		return static_cast<int>(MYBASE::npos == nIdx ? -1 : nIdx);
	}

	int Find(PCMYSTR szSub) const
	{
		MYSIZE nIdx = this->find(szSub);
		return static_cast<int>(MYBASE::npos == nIdx ? -1 : nIdx);
	}

	int Find(CT ch, int nStart) const
	{
		// CString::Find docs say add 1 to nStart when it's not zero
		// CString::Find code doesn't do that however.  We'll stick
		// with what the code does

		MYSIZE nIdx = this->find_first_of(ch, static_cast<MYSIZE>(nStart));
		return static_cast<int>(MYBASE::npos == nIdx ? -1 : nIdx);
	}

	int Find(PCMYSTR szSub, int nStart) const
	{
		// CString::Find docs say add 1 to nStart when it's not zero
		// CString::Find code doesn't do that however.  We'll stick
		// with what the code does

		MYSIZE nIdx = this->find(szSub, static_cast<MYSIZE>(nStart));
		return static_cast<int>(MYBASE::npos == nIdx ? -1 : nIdx);
	}

	int FindOneOf(PCMYSTR szCharSet) const
	{
		MYSIZE nIdx = this->find_first_of(szCharSet);
		return static_cast<int>(MYBASE::npos == nIdx ? -1 : nIdx);
	}

	// GetAllocLength -- an MSVC7 function but it costs us nothing to add it.
	int GetAllocLength()
	{
		return static_cast<int>(this->capacity());
	}

	// -------------------------------------------------------------------------
	// GetXXXX -- Direct access to character buffer
	// -------------------------------------------------------------------------
	CT GetAt(int nIdx) const
	{
		return this->at(static_cast<MYSIZE>(nIdx));
	}

	CT* GetBuffer(int nMinLen = -1)
	{
		return GetBuf(nMinLen);
	}

	CT* GetBufferSetLength(int nLen)
	{
		return BufferSet(nLen);
	}

	// GetLength() -- MFC docs say this is the # of BYTES but
	// in truth it is the number of CHARACTERs (chars or wchar_ts)
	int GetLength() const
	{
		return static_cast<int>(this->length());
	}

	int Insert(int nIdx, CT ch)
	{
		if (static_cast<MYSIZE>(nIdx) > this->size() - 1)
			this->append(1, ch);
		else
			this->insert(static_cast<MYSIZE>(nIdx), 1, ch);

		return GetLength();
	}
	int Insert(int nIdx, PCMYSTR sz)
	{
		if (static_cast<MYSIZE>(nIdx) >= this->size())
			this->append(sz, static_cast<MYSIZE>(sslen(sz)));
		else
			this->insert(static_cast<MYSIZE>(nIdx), sz);

		return GetLength();
	}

	bool IsEmpty() const
	{
		return this->empty();
	}

	MYTYPE Left(int nCount) const
	{
		// Range check the count.

		nCount = SSMAX(0, SSMIN(nCount, static_cast<int>(this->size())));
		return this->substr(0, static_cast<MYSIZE>(nCount));
	}

	void MakeLower()
	{
		ToLower();
	}

	void MakeReverse()
	{
		std::reverse(this->begin(), this->end());
	}

	void MakeUpper()
	{
		ToUpper();
	}

	MYTYPE Mid(int nFirst) const
	{
		return Mid(nFirst, this->GetLength() - nFirst);
	}

	MYTYPE Mid(int nFirst, int nCount) const
	{
		// CString does range checking here.  Since we're trying to emulate it,
		// we must check too.

		if (nFirst < 0)
			nFirst = 0;
		if (nCount < 0)
			nCount = 0;

		int nSize = static_cast<int>(this->size());

		if (nFirst + nCount > nSize)
			nCount = nSize - nFirst;

		if (nFirst > nSize)
			return MYTYPE();

		ASSERT(nFirst >= 0);
		ASSERT(nFirst + nCount <= nSize);

		return this->substr(static_cast<MYSIZE>(nFirst),
			static_cast<MYSIZE>(nCount));
	}

	void ReleaseBuffer(int nNewLen = -1)
	{
		RelBuf(nNewLen);
	}

	int Remove(CT ch)
	{
		MYSIZE nIdx = 0;
		int nRemoved = 0;
		while ((nIdx = this->find_first_of(ch)) != MYBASE::npos)
		{
			this->erase(nIdx, 1);
			nRemoved++;
		}
		return nRemoved;
	}

	int Replace(CT chOld, CT chNew)
	{
		int nReplaced = 0;

		for (MYITER iter = this->begin(); iter != this->end(); iter++)
		{
			if (*iter == chOld)
			{
				*iter = chNew;
				nReplaced++;
			}
		}

		return nReplaced;
	}

	int Replace(PCMYSTR szOld, PCMYSTR szNew)
	{
		int nReplaced = 0;
		MYSIZE nIdx = 0;
		MYSIZE nOldLen = sslen(szOld);

		if (0 != nOldLen)
		{
			// If the replacement string is longer than the one it replaces, this
			// string is going to have to grow in size,  Figure out how much
			// and grow it all the way now, rather than incrementally

			MYSIZE nNewLen = sslen(szNew);
			if (nNewLen > nOldLen)
			{
				int nFound = 0;
				while (nIdx < this->length() &&
					(nIdx = this->find(szOld, nIdx)) != MYBASE::npos)
				{
					nFound++;
					nIdx += nOldLen;
				}
				this->reserve(this->size() + nFound * (nNewLen - nOldLen));
			}


			static const CT ch = CT(0);
			PCMYSTR szRealNew = szNew == 0 ? &ch : szNew;
			nIdx = 0;

			while (nIdx < this->length() &&
				(nIdx = this->find(szOld, nIdx)) != MYBASE::npos)
			{
				this->replace(this->begin() + nIdx, this->begin() + nIdx + nOldLen,
					szRealNew);

				nReplaced++;
				nIdx += nNewLen;
			}
		}

		return nReplaced;
	}

	int ReplaceNoCase(PCMYSTR szOld, PCMYSTR szNew, BOOL bGlobal = TRUE)
	{
		int nReplaced = 0;
		MYSIZE nIdx = 0;
		MYSIZE nOldLen = sslen(szOld);

		if (0 != nOldLen)
		{
			// If the replacement string is longer than the one it replaces, this
			// string is going to have to grow in size,  Figure out how much
			// and grow it all the way now, rather than incrementally

			MYSIZE nNewLen = sslen(szNew);

			MYTYPE sLowerThis = *this;
			sLowerThis.MakeLower();

			MYTYPE sLowerOld = szOld;
			sLowerOld.MakeLower();

			if (nNewLen > nOldLen)
			{
				int nFound = 0;
				while (nIdx < sLowerThis.length() &&
					(nIdx = sLowerThis.find(sLowerOld, nIdx)) != MYBASE::npos)
				{
					nFound++;
					nIdx += nOldLen;
				}
				this->reserve(this->size() + nFound * (nNewLen - nOldLen));
			}

			static const CT ch = CT(0);
			PCMYSTR szRealNew = szNew == 0 ? &ch : szNew;
			nIdx = 0;

			while (nIdx < sLowerThis.length() &&
				(nIdx = sLowerThis.find(sLowerOld, nIdx)) != MYBASE::npos)
			{
				this->replace(this->begin() + nIdx,
					this->begin() + nIdx + nOldLen, szRealNew);

				sLowerThis.replace(sLowerThis.begin() + nIdx,
					sLowerThis.begin() + nIdx + nOldLen, szRealNew);

				nReplaced++;
				nIdx += nNewLen;
				if (!bGlobal) break;
			}
		}

		return nReplaced;
	}

	int ReverseFind(CT ch) const
	{
		MYSIZE nIdx = this->find_last_of(ch);
		return static_cast<int>(MYBASE::npos == nIdx ? -1 : nIdx);
	}

	// ReverseFind overload that's not in CString but might be useful
	int ReverseFind(PCMYSTR szFind, MYSIZE pos = MYBASE::npos) const
	{
		MYSIZE nIdx = this->rfind(0 == szFind ? MYTYPE() : szFind, pos);
		return static_cast<int>(MYBASE::npos == nIdx ? -1 : nIdx);
	}

	MYTYPE Right(int nCount) const
	{
		// Range check the count.

		nCount = SSMAX(0, SSMIN(nCount, static_cast<int>(this->size())));
		return this->substr(this->size() - static_cast<MYSIZE>(nCount));
	}

	void SetAt(int nIndex, CT ch)
	{
		ASSERT(this->size() > static_cast<MYSIZE>(nIndex));
		this->at(static_cast<MYSIZE>(nIndex)) = ch;
	}

#ifndef SS_ANSI
	BSTR SetSysString(BSTR* pbstr) const
	{
		ostring os;
		ssasn(os, *this);
		if (!::SysReAllocStringLen(pbstr, os.c_str(), os.length()))
			throw std::runtime_error("out of memory");

		ASSERT(*pbstr != 0);
		return *pbstr;
	}
#endif

	MYTYPE SpanExcluding(PCMYSTR szCharSet) const
	{
		MYSIZE pos = this->find_first_of(szCharSet);
		return pos == MYBASE::npos ? *this : Left((int)pos);
	}

	MYTYPE SpanIncluding(PCMYSTR szCharSet) const
	{
		MYSIZE pos = this->find_first_not_of(szCharSet);
		return pos == MYBASE::npos ? *this : Left((int)pos);
	}

#if defined SS_WIN32 && !defined(UNICODE) && !defined(SS_ANSI)

	// CString's OemToAnsi and AnsiToOem functions are available only in
	// Unicode builds.  However since we're a template we also need a
	// runtime check of CT and a reinterpret_cast to account for the fact
	// that CStdStringW gets instantiated even in non-Unicode builds.

	void AnsiToOem()
	{
		if (sizeof(CT) == sizeof(char) && !empty())
		{
			::CharToOem(reinterpret_cast<PCSTR>(this->c_str()),
				reinterpret_cast<PSTR>(GetBuf()));
		}
		else
		{
			ASSERT(false);
		}
	}

	void OemToAnsi()
	{
		if (sizeof(CT) == sizeof(char) && !empty())
		{
			::OemToChar(reinterpret_cast<PCSTR>(this->c_str()),
				reinterpret_cast<PSTR>(GetBuf()));
		}
		else
		{
			ASSERT(false);
		}
	}

#endif


	// -------------------------------------------------------------------------
	// Trim and its variants
	// -------------------------------------------------------------------------
	MYTYPE& Trim()
	{
		return TrimLeft().TrimRight();
	}

	MYTYPE& TrimLeft()
	{
		this->erase(this->begin(),
			std::find_if(this->begin(), this->end(), NotSpace<CT>()));

		return *this;
	}

	MYTYPE&  TrimLeft(CT tTrim)
	{
		this->erase(0, this->find_first_not_of(tTrim));
		return *this;
	}

	MYTYPE&  TrimLeft(PCMYSTR szTrimChars)
	{
		this->erase(0, this->find_first_not_of(szTrimChars));
		return *this;
	}

	MYTYPE& TrimRight()
	{
		// NOTE:  When comparing reverse_iterators here (MYRITER), I avoid using
		// operator!=.  This is because namespace rel_ops also has a template
		// operator!= which conflicts with the global operator!= already defined
		// for reverse_iterator in the header <utility>.
		// Thanks to John James for alerting me to this.

		MYRITER it = std::find_if(this->rbegin(), this->rend(), NotSpace<CT>());
		if (!(this->rend() == it))
			this->erase(this->rend() - it);

		this->erase(!(it == this->rend()) ? this->find_last_of(*it) + 1 : 0);
		return *this;
	}

	MYTYPE&  TrimRight(CT tTrim)
	{
		MYSIZE nIdx = this->find_last_not_of(tTrim);
		this->erase(MYBASE::npos == nIdx ? 0 : ++nIdx);
		return *this;
	}

	MYTYPE&  TrimRight(PCMYSTR szTrimChars)
	{
		MYSIZE nIdx = this->find_last_not_of(szTrimChars);
		this->erase(MYBASE::npos == nIdx ? 0 : ++nIdx);
		return *this;
	}

	void			FreeExtra()
	{
		MYTYPE mt;
		this->swap(mt);
		if (!mt.empty())
			this->assign(mt.c_str(), mt.size());
	}

	// I have intentionally not implemented the following CString
	// functions.   You cannot make them work without taking advantage
	// of implementation specific behavior.  However if you absolutely
	// MUST have them, uncomment out these lines for "sort-of-like"
	// their behavior.  You're on your own.

	//	CT*				LockBuffer()	{ return GetBuf(); }// won't really lock
	//	void			UnlockBuffer(); { }	// why have UnlockBuffer w/o LockBuffer?

	// Array-indexing operators.  Required because we defined an implicit cast
	// to operator const CT* (Thanks to Julian Selman for pointing this out)

	CT& operator[](int nIdx)
	{
		return static_cast<MYBASE*>(this)->operator[](static_cast<MYSIZE>(nIdx));
	}

	const CT& operator[](int nIdx) const
	{
		return static_cast<const MYBASE*>(this)->operator[](static_cast<MYSIZE>(nIdx));
	}

	CT& operator[](unsigned int nIdx)
	{
		return static_cast<MYBASE*>(this)->operator[](static_cast<MYSIZE>(nIdx));
	}

	const CT& operator[](unsigned int nIdx) const
	{
		return static_cast<const MYBASE*>(this)->operator[](static_cast<MYSIZE>(nIdx));
	}

#ifndef SS_NO_IMPLICIT_CAST
	operator const CT*() const
	{
		return this->c_str();
	}
#endif

};

// -----------------------------------------------------------------------------
// MSVC USERS: HOW TO EXPORT CSTDSTRING FROM A DLL
//
// If you are using MS Visual C++ and you want to export CStdStringA and
// CStdStringW from a DLL, then all you need to
//
//		1.	make sure that all components link to the same DLL version
//			of the CRT (not the static one).
//		2.	Uncomment the 3 lines of code below
//		3.	#define 2 macros per the instructions in MS KnowledgeBase
//			article Q168958.  The macros are:
//
//		MACRO		DEFINTION WHEN EXPORTING		DEFINITION WHEN IMPORTING
//		-----		------------------------		-------------------------
//		SSDLLEXP	(nothing, just #define it)		extern
//		SSDLLSPEC	__declspec(dllexport)			__declspec(dllimport)
//
//		Note that these macros must be available to ALL clients who want to 
//		link to the DLL and use the class.  If they 
//
// A word of advice: Don't bother.
//
// Really, it is not necessary to export CStdString functions from a DLL.  I
// never do.  In my projects, I do generally link to the DLL version of the
// Standard C++ Library, but I do NOT attempt to export CStdString functions.
// I simply include the header where it is needed and allow for the code
// redundancy.
//
// That redundancy is a lot less than you think.  This class does most of its
// work via the Standard C++ Library, particularly the base_class basic_string<>
// member functions.  Most of the functions here are small enough to be inlined
// anyway.  Besides, you'll find that in actual practice you use less than 1/2
// of the code here, even in big projects and different modules will use as
// little as 10% of it.  That means a lot less functions actually get linked
// your binaries.  If you export this code from a DLL, it ALL gets linked in.
//
// I've compared the size of the binaries from exporting vs NOT exporting.  Take
// my word for it -- exporting this code is not worth the hassle.
//
// -----------------------------------------------------------------------------
//#pragma warning(disable:4231) // non-standard extension ("extern template")
//	SSDLLEXP template class SSDLLSPEC CStdStr<char>;
//	SSDLLEXP template class SSDLLSPEC CStdStr<wchar_t>;


// =============================================================================
//						END OF CStdStr INLINE FUNCTION DEFINITIONS
// =============================================================================

//	Now typedef our class names based upon this humongous template

typedef CStdStr<char, wchar_t>		CStdStringA;	// a better std::string
typedef std::string					CStdStringUTF8;	// a simple Character Buffer holding an UTF8 encoded string
typedef CStdStr<wchar_t, char>		CStdStringW;	// a better std::wstring

// -----------------------------------------------------------------------------
// CStdStr addition functions defined as inline
// -----------------------------------------------------------------------------
inline CStdStringA operator+(const CStdStringA& s1, const CStdStringA& s2)
{
	CStdStringA sRet(s1);
	sRet.append(s2);
	return sRet;
}

inline CStdStringA operator+(const CStdStringA& s1, CStdStringA::value_type t)
{
	CStdStringA sRet(s1);
	sRet.append(1, t);
	return sRet;
}

inline CStdStringA operator+(const CStdStringA& s1, PCSTR pA)
{
	CStdStringA sRet(s1);
	sRet.append(pA);
	return sRet;
}

inline CStdStringA operator+(PCSTR pA, const CStdStringA& sA)
{
	CStdStringA sRet;
	CStdStringA::size_type nObjSize = sA.size();
	CStdStringA::size_type nLitSize =
		static_cast<CStdStringA::size_type>(sslen(pA));

	sRet.reserve(nLitSize + nObjSize);
	sRet.assign(pA);
	sRet.append(sA);
	return sRet;
}

#ifdef SS_ALLOWDEPECRECATES
[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
inline CStdStringA operator+(const CStdStringA& s1, const CStdStringW& s2)
{
	CStdStringA sa;
	EStringToAnsi(s2, sa);
	return s1 + sa;
}
#endif

inline CStdStringW operator+(const CStdStringW& s1, const CStdStringW& s2)
{
	CStdStringW sRet(s1);
	sRet.append(s2);
	return sRet;
}

#ifdef SS_ALLOWDEPECRECATES
[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
inline CStdStringA operator+(const CStdStringA& s1, PCWSTR pW)
{
	CStdStringA sa;
	EStringToAnsi(pW, sa);
	return s1 + sa;
}
#endif

#ifdef SS_ALLOWDEPECRECATES
#ifdef UNICODE
[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
inline CStdStringW operator+(PCWSTR pW, const CStdStringA& sA)
{
	CStdStringW sW;
	EStringToUnicode(sA, sW);
	return CStdStringW(pW) + sW;
}

[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
inline CStdStringW operator+(PCSTR pA, const CStdStringW& sW)
{
	CStdStringW pW;
	EStringToUnicode(pA, pW);
	return pW + sW;
}
#else
[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
inline CStdStringA operator+(PCWSTR pW, const CStdStringA& sA)
{
	return CStdStringA(pW) + sA;
}

[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
inline CStdStringA operator+(PCSTR pA, const CStdStringW& sW)
{
	return pA + CStdStringA(sW);
}
#endif
#endif

// ...Now the wide string versions.
inline CStdStringW operator+(const CStdStringW& s1, CStdStringW::value_type t)
{
	CStdStringW sRet(s1);
	sRet.append(1, t);
	return sRet;
}
inline CStdStringW operator+(const CStdStringW& s1, PCWSTR pW)
{
	CStdStringW sRet(s1);
	sRet.append(pW);
	return sRet;
}

inline CStdStringW operator+(PCWSTR pW, const CStdStringW& sW)
{
	CStdStringW sRet;
	CStdStringW::size_type nObjSize = sW.size();
	CStdStringA::size_type nLitSize =
		static_cast<CStdStringW::size_type>(sslen(pW));

	sRet.reserve(nLitSize + nObjSize);
	sRet.assign(pW);
	sRet.append(sW);
	return sRet;
}

#ifdef SS_ALLOWDEPECRECATES
[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
inline CStdStringW operator+(const CStdStringW& s1, const CStdStringA& s2)
{
	CStdStringW ws2;
	EStringToUnicode(s2, ws2);
	return s1 + ws2;
}

[[deprecated("Use EStringToAnsi and EStringToUnicode instead")]]
inline CStdStringW operator+(const CStdStringW& s1, PCSTR pA)
{
	CStdStringW ws2;
	EStringToUnicode(pA, ws2);
	return s1 + ws2;
}
#endif


// Define TCHAR based friendly names for some of these functions

#ifdef UNICODE
typedef CStdStringW				CStdString;
#else
typedef CStdStringA				CStdString;
#endif

// -----------------------------------------------------------------------------
// FUNCTIONAL COMPARATORS:
// REMARKS:
//		These structs are derived from the std::binary_function template.  They
//		give us functional classes (which may be used in Standard C++ Library
//		collections and algorithms) that perform case-insensitive comparisons of
//		CStdString objects.  This is useful for maps in which the key may be the
//		 proper string but in the wrong case.
// -----------------------------------------------------------------------------
#define StdStringLessNoCaseW		SSLNCW	// avoid VC compiler warning 4786
#define StdStringEqualsNoCaseW		SSENCW		
#define StdStringLessNoCaseA		SSLNCA		
#define StdStringEqualsNoCaseA		SSENCA		

#ifdef UNICODE
#define StdStringLessNoCase		SSLNCW		
#define StdStringEqualsNoCase	SSENCW		
#else
#define StdStringLessNoCase		SSLNCA		
#define StdStringEqualsNoCase	SSENCA		
#endif

struct StdStringLessNoCaseW
	: std::binary_function<CStdStringW, CStdStringW, bool>
{
	inline
		bool operator()(const CStdStringW& sLeft, const CStdStringW& sRight) const
	{
		return ssicmp(sLeft.c_str(), sRight.c_str()) < 0;
	}
};
struct StdStringEqualsNoCaseW
	: std::binary_function<CStdStringW, CStdStringW, bool>
{
	inline
		bool operator()(const CStdStringW& sLeft, const CStdStringW& sRight) const
	{
		return ssicmp(sLeft.c_str(), sRight.c_str()) == 0;
	}
};
struct StdStringLessNoCaseA
	: std::binary_function<CStdStringA, CStdStringA, bool>
{
	inline
		bool operator()(const CStdStringA& sLeft, const CStdStringA& sRight) const
	{
		return ssicmp(sLeft.c_str(), sRight.c_str()) < 0;
	}
};
struct StdStringEqualsNoCaseA
	: std::binary_function<CStdStringA, CStdStringA, bool>
{
	inline
		bool operator()(const CStdStringA& sLeft, const CStdStringA& sRight) const
	{
		return ssicmp(sLeft.c_str(), sRight.c_str()) == 0;
	}
};

// If we had to define our own version of TRACE above, get rid of it now

#ifdef TRACE_DEFINED_HERE
#undef TRACE
#undef TRACE_DEFINED_HERE
#endif

//Typedefs for containers
typedef std::list<CStdString>  CStdStringList;
typedef std::list<CStdStringA> CStdStringListA;
typedef std::list<CStdStringW> CStdStringListW;
typedef std::list<CStdStringUTF8>  CStdStringListUTF8;

typedef std::set<CStdString>  CStdStringSet;
typedef std::set<CStdStringA> CStdStringSetA;
typedef std::set<CStdStringW> CStdStringSetW;
typedef std::set<CStdStringUTF8> CStdStringSetUTF8;

typedef std::set<CStdString, StdStringLessNoCase>  CStdStringSetNoCase;
typedef std::set<CStdStringA, StdStringLessNoCaseA> CStdStringSetANoCase;
typedef std::set<CStdStringW, StdStringLessNoCaseW> CStdStringSetWNoCase;

typedef std::pair<CStdStringA, CStdStringA> CStdStringPairA;
typedef std::pair<CStdStringW, CStdStringW> CStdStringPairW;
typedef std::pair<CStdStringUTF8, CStdStringUTF8> CStdStringPairUTF8;

#ifdef _UNICODE
#define CStdStringPair CStdStringPairW
#else
#define CStdStringPair CStdStringPairA
#endif

//Typedefs for containers
typedef std::list<CStdStringPair>  CStdStringPairList;
typedef std::list<CStdStringPairA> CStdStringPairListA;
typedef std::list<CStdStringPairW> CStdStringPairListW;
typedef std::list<CStdStringPairUTF8>  CStdStringPairListUTF8;

//Typedefs for containers
typedef std::map<CStdString, CStdString>   CStdStringMap;
typedef std::map<CStdStringA, CStdStringA> CStdStringMapA;
typedef std::map<CStdStringW, CStdStringW> CStdStringMapW;

//Typedefs for containers
typedef std::map<CStdString, CStdStringList>   CStdStringToListMap;
typedef std::map<CStdStringA, CStdStringListA> CStdStringToListMapA;
typedef std::map<CStdStringW, CStdStringListW> CStdStringToListMapW;

// Kontainer um Binärdaten austauschen zu können
typedef std::map<CStdString, std::string> CStdStringToBinaryMap;

//Typedefs for containers
typedef std::map<CStdString, CStdString, StdStringLessNoCase>   CStdStringMapNoCase;
typedef std::map<CStdStringA, CStdStringA, StdStringLessNoCaseA> CStdStringMapANoCase;
typedef std::map<CStdStringW, CStdStringW, StdStringLessNoCaseW> CStdStringMapWNoCase;

//Typedefs for containers
typedef std::map<CStdString, CStdStringList, StdStringLessNoCase>   CStdStringToListMapNoCase;
typedef std::map<CStdStringA, CStdStringListA, StdStringLessNoCaseA> CStdStringToListMapANoCase;
typedef std::map<CStdStringW, CStdStringListW, StdStringLessNoCaseW> CStdStringToListMapWNoCase;


/*! String Tokenizer.
Zerlegt einen String nach Tokens.
Verwendet wird CStdStringTokA oder CStdStringTokW */
template<typename CT>
class CStdTokenizer
{
public:
	CStdTokenizer(const std::basic_string<CT>& _token_str, const std::basic_string<CT>& _delim, bool bSkipEmptyTokens = false)
	{
		token_str = _token_str;
		delim = _delim;

		if (bSkipEmptyTokens)
		{
			//Alle leeren tokens entfernen

			//Remove sequential delimiter
			size_t curr_pos = 0;
			while (true)
			{
				if ((curr_pos = token_str.find(delim, curr_pos)) != std::basic_string<CT>::npos)
				{
					curr_pos += delim.length();

					while (token_str.find(delim, curr_pos) == curr_pos)
					{
						token_str.erase(curr_pos, delim.length());
					}
				}
				else
					break;
			}

			//Trim leading delimiter
			if (token_str.find(delim, 0) == 0)
			{
				token_str.erase(0, delim.length());
			}

			//Trim ending delimiter
			curr_pos = 0;
			if ((curr_pos = token_str.rfind(delim)) != std::basic_string<CT>::npos)
			{
				if (curr_pos != (token_str.length() - delim.length())) return;
				token_str.erase(token_str.length() - delim.length(), delim.length());
			}
		}
	}
	/*! countTokens zählt die Anzahl der Werte zwischen den Tokens.
	Dabei zählt ein Wert am Ende des Strings mit (ohne Delimiter danach) */
	int countTokens()
	{
		size_t prev_pos = 0;
		int num_tokens = 0;
		if (token_str.length() > 0)
		{
			size_t curr_pos = 0;
			while (true)
			{
				if ((curr_pos = token_str.find(delim, curr_pos)) != std::basic_string<CT>::npos)
				{
					num_tokens++;
					prev_pos = curr_pos;
					curr_pos += delim.length();
				}
				else
					break;
			}
			return ++num_tokens;
		}
		else
		{
			return 0;
		}
	}
	/*! Der nächste Wert */
	std::basic_string<CT> nextToken()
	{
		return nextToken(delim);
	}
	std::basic_string<CT> nextToken(const std::basic_string<CT>& delimiter)
	{
		std::basic_string<CT> tmp_str;
		if (token_str.length() == 0)
			return tmp_str;

		size_t pos = token_str.find(delimiter, 0);

		if (pos != std::basic_string<CT>::npos)
		{
			tmp_str = token_str.substr(0, pos);
			token_str = token_str.substr(pos + delimiter.length(), token_str.length() - pos);
		}
		else
		{
			tmp_str = token_str.substr(0, token_str.length());
			token_str.clear();
		}

		return tmp_str;
	}
	//Get the next token, but do not remove it.
	std::basic_string<CT> nextTokenPeek()
	{
		std::basic_string<CT> tmp_str;
		if (token_str.length() == 0)
			return tmp_str;

		size_t pos = token_str.find(delim, 0);

		if (pos != std::basic_string<CT>::npos)
		{
			tmp_str = token_str.substr(0, pos);
		}
		else
		{
			tmp_str = token_str.substr(0, token_str.length());
		}

		return tmp_str;
	}
	std::basic_string<CT> remainingString()
	{
		return token_str;
	}
	bool hasMoreTokens()
	{
		return !token_str.empty();
	}

private:
	std::basic_string<CT>  token_str;
	std::basic_string<CT>  delim;
};

typedef CStdTokenizer<char>		CStdStringTokA;
typedef CStdTokenizer<wchar_t>	CStdStringTokW;

#ifdef _UNICODE
#define CStdStringTok CStdStringTokW
#else
#define CStdStringTok CStdStringTokA
#endif

// Turn back on any Borland warnings we turned off.

#ifdef __BORLANDC__
#pragma option pop  // Turn back on inline function warnings
//	#pragma warn +inl   // Turn back on inline function warnings
#endif

inline std::wstring EStringToUnicode(const std::string& sSrc)
{
	std::wstring sDst;
	EStringToUnicode(sSrc, sDst);
	return sDst;
}

inline std::string EStringToAnsi(const std::wstring& sSrc)
{
	std::string sDst;
	EStringToAnsi(sSrc, sDst);
	return sDst;
}

inline std::wstring EStringToUnicode(const char* sSrc)
{
	std::wstring sDst;
	EStringToUnicode(sSrc, sDst);
	return sDst;
}

inline std::string EStringToAnsi(const wchar_t* sSrc)
{
	std::string sDst;
	EStringToAnsi(sSrc, sDst);
	return sDst;
}

inline std::wstring EStringFromBSTR(const BSTR bstrSrc)
{
	std::wstring strData;
	const int iLen = ::SysStringLen(bstrSrc);
	if (iLen > 0)
		strData.append(bstrSrc, iLen);
	return strData;
}

// wrapper um nullptr Zuweisung an CStdStringUTF8 abzufangen
inline const char* ENullptrToEmpty(char const* s)
{
	return (nullptr == s ? "" : s);
}

#endif	// #ifndef STDSTRING_H
