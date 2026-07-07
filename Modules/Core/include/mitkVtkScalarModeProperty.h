/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkScalarModeProperty_h
#define mitkVtkScalarModeProperty_h

#include <mitkEnumerationProperty.h>

namespace mitk
{
  /**
   * \brief Encapsulates the VTK scalar mode enumeration as an EnumerationProperty.
   *
   * Valid values are (VTK constant / Id / string representation):
   * \li VTK_SCALAR_MODE_DEFAULT / 0 / Default
   * \li VTK_SCALAR_MODE_USE_POINT_DATA / 1 / PointData
   * \li VTK_SCALAR_MODE_USE_CELL_DATA / 2 / CellData
   * \li VTK_SCALAR_MODE_USE_POINT_FIELD_DATA / 3 / PointFieldData
   * \li VTK_SCALAR_MODE_USE_CELL_FIELD_DATA / 4 / CellFieldData
   */
  class MITKCORE_EXPORT VtkScalarModeProperty : public EnumerationProperty
  {
  public:
    mitkClassMacro(VtkScalarModeProperty, EnumerationProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkNewMacro1Param(VtkScalarModeProperty, const IdType &);

    mitkNewMacro1Param(VtkScalarModeProperty, const std::string &);

    /**
     * \brief Return the current scalar mode value as defined by VTK constants.
     *
     * \return The current scalar mode as a VTK constant (e.g. VTK_SCALAR_MODE_DEFAULT).
     */
    virtual int GetVtkScalarMode();

    /** \brief Set the scalar mode to VTK_SCALAR_MODE_DEFAULT. */
    virtual void SetScalarModeToDefault();

    /** \brief Set the scalar mode to VTK_SCALAR_MODE_USE_POINT_DATA. */
    virtual void SetScalarModeToPointData();

    /** \brief Set the scalar mode to VTK_SCALAR_MODE_USE_CELL_DATA. */
    virtual void SetScalarModeToCellData();

    /** \brief Set the scalar mode to VTK_SCALAR_MODE_USE_POINT_FIELD_DATA. */
    virtual void SetScalarModeToPointFieldData();

    /** \brief Set the scalar mode to VTK_SCALAR_MODE_USE_CELL_FIELD_DATA. */
    virtual void SetScalarModeToCellFieldData();

    using BaseProperty::operator=;

  protected:
    /**
     * \brief Default constructor. Sets the scalar mode to VTK_SCALAR_MODE_DEFAULT.
     */
    VtkScalarModeProperty();

    /**
     * \brief Construct with an integer scalar mode value.
     *
     * If the value is not a valid enumeration id, the scalar mode is set to
     * VTK_SCALAR_MODE_DEFAULT (0).
     *
     * \param[in] value The integer representation of the scalar mode.
     */
    VtkScalarModeProperty(const IdType &value);

    /**
     * \brief Construct with a string scalar mode value.
     *
     * If the value is not a valid enumeration string, the scalar mode is set to
     * VTK_SCALAR_MODE_DEFAULT (0).
     *
     * \param[in] value The string representation of the scalar mode.
     */
    VtkScalarModeProperty(const std::string &value);

    /** \brief Copy constructor. */
    VtkScalarModeProperty(const VtkScalarModeProperty &other);

    /**
     * \brief Add an enumeration value.
     *
     * Overridden as protected so that external code cannot add
     * additional invalid scalar mode types.
     *
     * \param[in] name  The string name of the enumeration entry.
     * \param[in] id    The integer id of the enumeration entry.
     * \return True if the entry was added successfully.
     */
    bool AddEnum(const std::string &name, const IdType &id) override;

    /**
     * \brief Populate the enumeration with the VTK-defined scalar mode types.
     */
    virtual void AddInterpolationTypes();

    mitkCloneMacro(VtkScalarModeProperty);

  private:
    // purposely not implemented
    VtkScalarModeProperty &operator=(const VtkScalarModeProperty &);
  };


} // end of namespace mitk

#endif
