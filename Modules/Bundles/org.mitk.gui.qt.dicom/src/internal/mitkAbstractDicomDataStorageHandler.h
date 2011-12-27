/*=========================================================================
Program: Medical Imaging & Interaction Toolkit
Module: $RCSfile$
Language: C++
Date: $Date$
Version: $Revision$
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notices for more information.
=========================================================================*/

#ifndef __AbstractDicomDataStorageHandler_h
#define __AbstractDicomDataStorageHandler_h

#include "... .h"

namespace mitk
{
class AbstractDicomDataStorageHandler
{


//Methods

//Signals

//Slots

//Data Member
  QSharedPointer<ctkDICOMDatabase> m_DICOMDatabase;
  QSharedPointer<ctkDICOMIndexer> m_DICOMIndexer;
  QSharedPointer<ctkDICOMModel> m_DICOMModel;

};
}
#endif