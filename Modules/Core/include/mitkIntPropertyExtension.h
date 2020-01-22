/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIntPropertyExtension_h
#define mitkIntPropertyExtension_h

#include <MitkCoreExports.h>
#include <mitkPropertyExtension.h>

namespace mitk
{
  /** \brief Property extension for mitk::IntProperty.
    *
    * The property view uses this extension to configure the corresponding property editor.
    */
  class MITKCORE_EXPORT IntPropertyExtension : public PropertyExtension
  {
  public:
    mitkClassMacro(IntPropertyExtension, PropertyExtension);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);
    mitkNewMacro2Param(Self, int, int);
    mitkNewMacro3Param(Self, int, int, int);

    int GetMaximum() const;
    void SetMaximum(int maximum);

    int GetMinimum() const;
    void SetMinimum(int minimum);

    int GetSingleStep() const;
    void SetSingleStep(int singleStep);

  private:
    IntPropertyExtension();
    IntPropertyExtension(int minimum, int maximum, int singleStep = 1);

    ~IntPropertyExtension() override;

    struct Impl;
    Impl *m_Impl;
  };
}

#endif
