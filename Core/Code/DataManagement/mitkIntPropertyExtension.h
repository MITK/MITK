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

#ifndef mitkIntPropertyExtension_h
#define mitkIntPropertyExtension_h

#include <mitkPropertyExtension.h>
#include <MitkExports.h>

namespace mitk
{
  class MITK_CORE_EXPORT IntPropertyExtension : public PropertyExtension
  {
  public:
    IntPropertyExtension();
    IntPropertyExtension(int minimum, int maximum, int singleStep = 1);
    ~IntPropertyExtension();

    int GetMaximum() const;
    void SetMaximum(int maximum);

    int GetMinimum() const;
    void SetMinimum(int minimum);

    int GetSingleStep() const;
    void SetSingleStep(int singleStep);

  private:
    struct Impl;
    Impl* m_Impl;
  };
}

#endif
