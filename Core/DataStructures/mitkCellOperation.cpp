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


#include "mitkCellOperation.h"

mitk::CellOperation::CellOperation(OperationType operationType, int cellId, Vector3D vector)
: mitk::Operation(operationType), m_CellId(cellId), m_Vector(vector)
{
}

mitk::CellOperation::CellOperation(OperationType operationType, int cellId)
: mitk::Operation(operationType), m_CellId(cellId)
{
  m_Vector.Fill(0);
}
