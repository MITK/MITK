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


#include "mitkOperation.h"

//##ModelId=3E7830E70054
mitk::Operation::Operation(mitk::OperationType operationType)
: m_OperationType(operationType)
{
}

//##ModelId=3F01770A0007
mitk::Operation::~Operation()
{
}

//##ModelId=3E7831B600CA
mitk::OperationType mitk::Operation::GetOperationType()
{
	return m_OperationType;
}
