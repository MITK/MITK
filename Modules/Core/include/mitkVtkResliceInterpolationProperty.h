/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkResliceInterpolationProperty_h
#define mitkVtkResliceInterpolationProperty_h

#include <mitkEnumerationProperty.h>

#include <vtkImageReslice.h>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * \brief Encapsulates the VTK reslice interpolation enumeration as an EnumerationProperty.
   *
   * Valid values are (VTK constant / Id / string representation):
   * VTK_RESLICE_NEAREST, VTK_RESLICE_LINEAR, VTK_RESLICE_CUBIC.
   * Default is VTK_RESLICE_NEAREST.
   */
  class MITKCORE_EXPORT VtkResliceInterpolationProperty : public EnumerationProperty
  {
  public:
    mitkClassMacro(VtkResliceInterpolationProperty, EnumerationProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkNewMacro1Param(VtkResliceInterpolationProperty, const IdType &);

    mitkNewMacro1Param(VtkResliceInterpolationProperty, const std::string &);

    /**
     * \brief Return the current reslice interpolation value as defined by VTK constants.
     * \return The current interpolation as a VTK constant.
     */
    virtual int GetInterpolation();

    /** \brief Set the interpolation type to VTK_RESLICE_NEAREST. */
    virtual void SetInterpolationToNearest();

    /** \brief Set the interpolation type to VTK_RESLICE_LINEAR. */
    virtual void SetInterpolationToLinear();

    /** \brief Set the interpolation type to VTK_RESLICE_CUBIC. */
    virtual void SetInterpolationToCubic();

    using BaseProperty::operator=;

  protected:
    /** \brief Constructor. Sets reslice interpolation to default (VTK_RESLICE_NEAREST). */
    VtkResliceInterpolationProperty();

    /**
     * \brief Constructor. Sets reslice interpolation to the given value.
     * \param[in] value The integer representation of the interpolation type.
     */
    VtkResliceInterpolationProperty(const IdType &value);

    /**
     * \brief Constructor. Sets reslice interpolation to the given value.
     * \param[in] value The string representation of the interpolation type.
     */
    VtkResliceInterpolationProperty(const std::string &value);

    VtkResliceInterpolationProperty(const VtkResliceInterpolationProperty &other);

    /**
     * \brief Overridden as protected to prevent adding invalid interpolation types.
     */
    bool AddEnum(const std::string &name, const IdType &id) override;

    /**
     * \brief Add the VTK-defined reslice interpolation types to the enumeration.
     */
    virtual void AddInterpolationTypes();

    mitkCloneMacro(VtkResliceInterpolationProperty);

  private:
    // purposely not implemented
    VtkResliceInterpolationProperty &operator=(const VtkResliceInterpolationProperty &);
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // end of namespace mitk

#endif
