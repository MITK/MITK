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


#ifndef BASEVTKMAPPER3D_H_HEADER_INCLUDED
#define BASEVTKMAPPER3D_H_HEADER_INCLUDED

#include "mitkVtkMapper.h"

namespace mitk {

  // typedef allows integration of mappers into the new mapper architecture
  // \deprecatedSince{2013_03} Use VtkMapper instead
  DEPRECATED(typedef VtkMapper VtkMapper3D);

} // namespace mitk

#endif /* BASEVTKMAPPER3D_H_HEADER_INCLUDED */
