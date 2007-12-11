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


#include "mitkLineOperation.h"

mitk::LineOperation::LineOperation(OperationType operationType, int cellId, Vector3D vector, int pIdA, int pIdB, int id)
: mitk::CellOperation(operationType, cellId, vector), m_PIdA(pIdA), m_PIdB(pIdB), m_Id(id)
{}
mitk::LineOperation::LineOperation(OperationType operationType, int cellId, int pIdA, int pIdB, int id)
: mitk::CellOperation(operationType, cellId), m_PIdA(pIdA), m_PIdB(pIdB), m_Id(id)
{}

int mitk::LineOperation::GetPIdA()
{
	return m_PIdA;
}

int mitk::LineOperation::GetPIdB()
{
	return m_PIdB;
}

int mitk::LineOperation::GetId()
{
	return m_Id;
}


