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

#ifndef mitkTag_h
#define mitkTag_h

#include <vector>

#include "DICOMReaderExports.h"

namespace mitk
{

class DICOMReader_EXPORT DICOMTag
{
  public:

    DICOMTag(unsigned int group, unsigned int element);
    DICOMTag(const DICOMTag& other);
    DICOMTag& operator=(const DICOMTag& other);

    unsigned int GetGroup() const;
    unsigned int GetElement() const;

  protected:

    unsigned int m_Group;
    unsigned int m_Element;
};

typedef std::vector<DICOMTag> DICOMTagList;

}

#endif
