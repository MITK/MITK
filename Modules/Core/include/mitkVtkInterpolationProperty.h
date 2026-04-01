/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkInterpolationProperty_h
#define mitkVtkInterpolationProperty_h

#include <mitkEnumerationProperty.h>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * \brief Encapsulates the VTK interpolation enumeration as an EnumerationProperty.
   *
   * Valid values are (VTK constant / Id / string representation):
   * VTK_FLAT/0/Flat, VTK_GOURAUD/1/Gouraud, VTK_PHONG/2/Phong.
   * Default is Gouraud interpolation.
   */
  class MITKCORE_EXPORT VtkInterpolationProperty : public EnumerationProperty
  {
  public:
    mitkClassMacro(VtkInterpolationProperty, EnumerationProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    mitkNewMacro1Param(VtkInterpolationProperty, const IdType &);

    mitkNewMacro1Param(VtkInterpolationProperty, const std::string &);

    /**
     * \brief Return the current interpolation value as defined by VTK constants.
     * \return The current interpolation as a VTK constant.
     */
    virtual int GetVtkInterpolation();

    /** \brief Set the interpolation type to VTK_FLAT. */
    virtual void SetInterpolationToFlat();

    /** \brief Set the interpolation type to VTK_GOURAUD. */
    virtual void SetInterpolationToGouraud();

    /** \brief Set the interpolation type to VTK_PHONG. */
    virtual void SetInterpolationToPhong();

    using BaseProperty::operator=;

  protected:
    /** \brief Constructor. Sets the interpolation to a default value of Gouraud(1). */
    VtkInterpolationProperty();

    /**
     * \brief Constructor. Sets the interpolation to the given value.
     *
     * If the value is not valid, the interpolation is set to Gouraud(1).
     *
     * \param[in] value The integer representation of the interpolation.
     */
    VtkInterpolationProperty(const IdType &value);

    /**
     * \brief Constructor. Sets the interpolation to the given value.
     *
     * If the value is not valid, the interpolation is set to Gouraud(1).
     *
     * \param[in] value The string representation of the interpolation.
     */
    VtkInterpolationProperty(const std::string &value);

    VtkInterpolationProperty(const VtkInterpolationProperty &other);

    /**
     * \brief Overridden as protected to prevent adding invalid interpolation types.
     */
    bool AddEnum(const std::string &name, const IdType &id) override;

    /**
     * \brief Add the VTK-defined interpolation types to the enumeration.
     */
    virtual void AddInterpolationTypes();

    mitkCloneMacro(VtkInterpolationProperty);

  private:
    // purposely not implemented
    VtkInterpolationProperty &operator=(const VtkInterpolationProperty &);
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // end of namespace mitk

#endif
