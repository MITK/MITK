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
#include <MitkCoreExports.h>

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

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
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

    ~IntPropertyExtension();

    struct Impl;
    Impl* m_Impl;
  };
}

#endif
