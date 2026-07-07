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

    /** \brief Get the maximum allowed value. Defaults to 100.
     *  \return The maximum value.
     */
    int GetMaximum() const;

    /** \brief Set the maximum allowed value. */
    void SetMaximum(int maximum);

    /** \brief Get the minimum allowed value. Defaults to 0.
     *  \return The minimum value.
     */
    int GetMinimum() const;

    /** \brief Set the minimum allowed value. */
    void SetMinimum(int minimum);

    /** \brief Get the single step increment. Defaults to 1.
     *  \return The single step value.
     */
    int GetSingleStep() const;

    /** \brief Set the single step increment. */
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
