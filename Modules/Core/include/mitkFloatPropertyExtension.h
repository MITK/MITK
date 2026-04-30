/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFloatPropertyExtension_h
#define mitkFloatPropertyExtension_h

#include <MitkCoreExports.h>
#include <mitkPropertyExtension.h>

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

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);
    mitkNewMacro2Param(Self, float, float);
    mitkNewMacro3Param(Self, float, float, float);
    mitkNewMacro4Param(Self, float, float, float, int);

    /** \brief Get the number of decimal places for display.
     * \return The number of decimals.
     */
    int GetDecimals() const;

    /** \brief Set the number of decimal places for display.
     * \param decimals The number of decimals.
     */
    void SetDecimals(int decimals);

    /** \brief Get the maximum allowed value.
     * \return The maximum value.
     */
    float GetMaximum() const;

    /** \brief Set the maximum allowed value.
     * \param maximum The maximum value.
     */
    void SetMaximum(float maximum);

    /** \brief Get the minimum allowed value.
     * \return The minimum value.
     */
    float GetMinimum() const;

    /** \brief Set the minimum allowed value.
     * \param minimum The minimum value.
     */
    void SetMinimum(float minimum);

    /** \brief Get the single step increment for the property editor.
     * \return The single step value.
     */
    float GetSingleStep() const;

    /** \brief Set the single step increment for the property editor.
     * \param singleStep The single step value.
     */
    void SetSingleStep(float singleStep);

  private:
    /** \brief Default constructor. Uses default min (0), max (~100), step (1), and 2 decimals. */
    FloatPropertyExtension();

    /** \brief Constructor with range and step configuration.
     * \param minimum The minimum allowed value.
     * \param maximum The maximum allowed value.
     * \param singleStep The step increment (default 0.1).
     * \param decimals The number of decimal places (default 2).
     */
    FloatPropertyExtension(float minimum, float maximum, float singleStep = 0.1f, int decimals = 2);

    ~FloatPropertyExtension() override;

    struct Impl;
    Impl *m_Impl;
  };
}

#endif
