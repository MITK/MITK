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

#ifndef mitkFloatPropertyExtension_h
#define mitkFloatPropertyExtension_h

#include <mitkPropertyExtension.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /** \brief Property extension for mitk::FloatProperty.
    *
    * The property view uses this extension to configure the corresponding property editor.
    */
  class MITKCORE_EXPORT FloatPropertyExtension : public PropertyExtension
  {
  public:
    mitkClassMacro(FloatPropertyExtension, PropertyExtension);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    mitkNewMacro2Param(Self, float, float);
    mitkNewMacro3Param(Self, float, float, float);
    mitkNewMacro4Param(Self, float, float, float, int);

    int GetDecimals() const;
    void SetDecimals(int decimals);

    float GetMaximum() const;
    void SetMaximum(float maximum);

    float GetMinimum() const;
    void SetMinimum(float minimum);

    float GetSingleStep() const;
    void SetSingleStep(float singleStep);

  private:
    FloatPropertyExtension();
    FloatPropertyExtension(float minimum, float maximum, float singleStep = 0.1f, int decimals = 2);

    ~FloatPropertyExtension();

    struct Impl;
    Impl* m_Impl;
  };
}

#endif
