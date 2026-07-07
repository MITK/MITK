/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGridVolumeMapperProperty_h
#define mitkGridVolumeMapperProperty_h

#include <MitkDataTypesExtExports.h>
#include <mitkEnumerationProperty.h>

namespace mitk
{
  /**
   * \brief Enumeration property for volume mapper types used with unstructured grids.
   *
   * Extends EnumerationProperty with the following values:
   * - RAYCAST (0) -- ray casting volume mapper (default)
   * - PT (1) -- projected tetrahedra volume mapper
   * - ZSWEEP (2) -- ZSweep volume mapper
   *
   * \sa EnumerationProperty, GridRepresentationProperty
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT GridVolumeMapperProperty : public EnumerationProperty
  {
  public:
    /**
     * \brief Available volume mapper types.
     */
    enum MapperType
    {
      RAYCAST = 0, ///< Ray casting mapper.
      PT = 1,      ///< Projected tetrahedra mapper.
      ZSWEEP = 2   ///< ZSweep mapper.
    };

    mitkClassMacro(GridVolumeMapperProperty, EnumerationProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkNewMacro1Param(GridVolumeMapperProperty, const IdType &);

    mitkNewMacro1Param(GridVolumeMapperProperty, const std::string &);

    /** \brief Set the volume mapper to projected tetrahedra. */
    virtual void SetVolumeMapperToPT();

    /** \brief Set the volume mapper to ZSweep. */
    virtual void SetVolumeMapperToZSweep();

    /** \brief Set the volume mapper to ray casting. */
    virtual void SetVolumeMapperToRayCast();

  protected:
    /**
     * Constructor. Sets the representation to a default value of Wireframe(1)
     */
    GridVolumeMapperProperty();

    GridVolumeMapperProperty(const GridVolumeMapperProperty &other);

    /**
     * Constructor. Sets the representation to the given value. If it is not
     * valid, the representation is set to Wireframe(1)
     * @param value the integer representation of the representation
     */
    GridVolumeMapperProperty(const IdType &value);

    /**
     * Constructor. Sets the representation to the given value. If it is not
     * valid, the representation is set to Wireframe(1)
     * @param value the string representation of the representation
     */
    GridVolumeMapperProperty(const std::string &value);

    /**
     * this function is overridden as protected, so that the user may not add
     * additional invalid representation types.
     */
    bool AddEnum(const std::string &name, const IdType &id) override;

    /**
     * Adds the enumeration types as defined by vtk to the list of known
     * enumeration values.
     */
    virtual void AddRepresentationTypes();

    mitkCloneMacro(GridVolumeMapperProperty);
  };
} // end of namespace mitk
#endif
