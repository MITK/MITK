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



#include "mitkCompositeMapper.h"


mitk::CompositeMapper::CompositeMapper()
{
  m_OdfMapper = mitk::OdfVtkMapper2D<float,QBALL_ODFSIZE>::New();
  m_ImgMapper = mitk::CopyImageMapper2D::New();
}

mitk::CompositeMapper::~CompositeMapper()
{
}
