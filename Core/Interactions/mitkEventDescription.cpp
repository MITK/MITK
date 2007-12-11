/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkEventDescription.h"

//##ModelId=3E5B30A30095
mitk::EventDescription::EventDescription(int type, int button, int buttonState, int key, std::string name, int id)
: Event(NULL, type, button, buttonState, key), m_Name(name), m_Id(id)
{}

//##ModelId=3E5B3103030A
std::string mitk::EventDescription::GetName() const
{
	return m_Name;
}

//##ModelId=3E5B3132027C
int mitk::EventDescription::GetId() const
{
	return m_Id;
}

