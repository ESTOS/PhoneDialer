// TAPIOBJ.CPP
//
// This file contains the basic object functions for the
// class library.
//
// This is a part of the TAPI Applications Classes C++ library.
// Original Copyright � 1995-2004 JulMar Entertainment Technology, Inc. All rights reserved.
//
// "This program is free software; you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation; version 2 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program; if not, write
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// Or, contact: JulMar Technology, Inc. at: info@julmar.com."
//

#include "stdafx.h"
#include "atapi.h"

IMPLEMENT_DYNCREATE(CTapiObject, CObject)

//////////////////////////////////////////////////////////////////////////////
// CTapiObject::CTapiObject
//
// Constructor for the tapi object
//
CTapiObject::CTapiObject()
	: m_dwUserData(0)
{
} // CTapiObject::CTapiObject

////////////////////////////////////////////////////////////////////////////////////
// CTapiObject::~CTapiObject
//
// Destructor
//
CTapiObject::~CTapiObject()
{
} // CTapiObject::~CTapiObject

////////////////////////////////////////////////////////////////////////////////////
// CTapiObject::GetUserData
//
// Retrieve the user data value
//
DWORD_PTR CTapiObject::GetUserData() const
{
	return m_dwUserData;

} // CTapiObject::GetUserData

////////////////////////////////////////////////////////////////////////////////////
// CTapiObject::GetUserDataPtr
//
// Retrieve the user data value as a pointer.
//
void* CTapiObject::GetUserDataPtr() const
{
	return (void*)m_dwUserData;

} // CTapiObject::GetUserDataPtr

////////////////////////////////////////////////////////////////////////////////////
// CTapiObject::SetUserData
//
// Set the userdata value
//
void CTapiObject::SetUserData(DWORD_PTR dwUserData)
{
	m_dwUserData = dwUserData;

} // CTapiObject::SetUserData

////////////////////////////////////////////////////////////////////////////////////
// CTapiObject::SetUserDataPtr
//
// Set the userdata value to a pointer
//
void CTapiObject::SetUserDataPtr(void* pValue)
{
	m_dwUserData = (DWORD_PTR)pValue;

} // CTapiObject::SetUserDataPtr
