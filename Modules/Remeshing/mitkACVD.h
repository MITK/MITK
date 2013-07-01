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

#ifndef mitkACVD_h
#define mitkACVD_h

#include <RemeshingExports.h>

namespace mitk
{
  class Remeshing_EXPORT ACVD
  {
  public:
    ACVD();
    ~ACVD();

  private:
    ACVD(const ACVD&);
    ACVD& operator=(const ACVD&);
  };
}

#endif
