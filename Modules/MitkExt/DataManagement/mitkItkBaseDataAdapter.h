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

#include "mitkBaseData.h"
#include "MitkExtExports.h"

#include <itkDataObject.h>

namespace mitk {

/**
 * \brief Small class to store itk::DataObjects in the mitk::DataStorage.
 *
 * This class can used to store any itk::DataObject in the DataStorage.
 * Please be aware that these objects cannot be visualized via mappers.
 * The user of this class must care himself for type safety.
 **/
class MitkExt_EXPORT ItkBaseDataAdapter : public BaseData
{
public:
  mitkClassMacro(ItkBaseDataAdapter, BaseData);
  itkNewMacro(Self);

  /** Typedef for the data object */
  typedef itk::DataObject DataType;

  /** Returns the data object. **/
  itkGetObjectMacro(ItkDataObject, DataType);
  /** Sets the data object. **/
  itkSetObjectMacro(ItkDataObject, DataType);

  virtual void SetRequestedRegionToLargestPossibleRegion() {}
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() { return false; }
  virtual bool VerifyRequestedRegion() { return true; }
  virtual void SetRequestedRegion(itk::DataObject*) {}

protected:
  DataType::Pointer m_ItkDataObject;

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;
};

}
