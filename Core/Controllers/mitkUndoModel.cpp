/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkUndoModel.h"


mitk::UndoModel::UndoModel()
{}

mitk::UndoModel::~UndoModel()
{}

//##ModelId=3F01770A018E
void mitk::UndoModel::SwapOperations(mitk::OperationEvent *operationEvent)
{
	operationEvent->SwapOperations();
}
