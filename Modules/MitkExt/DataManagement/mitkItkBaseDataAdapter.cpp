/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkItkBaseDataAdapter.h"

namespace mitk {

void ItkBaseDataAdapter::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << "ItkBaseDataAdapter:" << std::endl;
  if(m_ItkDataObject.IsNull())
  {
    os << indent << "No DataObject!" << std::endl;
  }
  else
  {
    m_ItkDataObject->Print(os, indent);
  }
}

}
