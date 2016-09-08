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

#ifndef mitkAnnotationService_h
#define mitkAnnotationService_h

#include <mitkIAnnotationService.h>
#include <map>

namespace mitk
{
  class AnnotationService : public IAnnotationService
  {
  public:
    AnnotationService();
    ~AnnotationService();

  private:

    AnnotationService(const AnnotationService&);
    AnnotationService& operator=(const AnnotationService&);
  };
}

#endif
