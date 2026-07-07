/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGridRepresentationProperty_h
#define mitkGridRepresentationProperty_h

#include <MitkDataTypesExtExports.h>
#include <mitkEnumerationProperty.h>

namespace mitk
{
  /**
   * \brief Enumeration property for grid representation types.
   *
   * Extends EnumerationProperty with the following values:
   * - POINTS (0) -- render as points
   * - WIREFRAME (1) -- render as wireframe (default)
   * - SURFACE (2) -- render as surface
   *
   * \sa EnumerationProperty, GridVolumeMapperProperty
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT GridRepresentationProperty : public EnumerationProperty
  {
  public:
    /**
     * \brief Available representation types for unstructured grids.
     */
    enum RepresentationType
    {
      POINTS = 0,    ///< Render grid as points.
      WIREFRAME = 1, ///< Render grid as wireframe.
      SURFACE = 2    ///< Render grid as filled surface.
    };

    mitkClassMacro(GridRepresentationProperty, EnumerationProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkNewMacro1Param(GridRepresentationProperty, const IdType &);

    mitkNewMacro1Param(GridRepresentationProperty, const std::string &);

    /** \brief Set the representation type to POINTS. */
    virtual void SetRepresentationToPoints();

    /** \brief Set the representation type to WIREFRAME. */
    virtual void SetRepresentationToWireframe();

    /** \brief Set the representation type to SURFACE. */
    virtual void SetRepresentationToSurface();

    /** \brief Set the representation type to WIREFRAME_SURFACE (combined). */
    virtual void SetRepresentationToWireframeSurface();

  protected:
    /**
     * Constructor. Sets the representation to a default value of Wireframe(1)
     */
    GridRepresentationProperty();

    GridRepresentationProperty(const GridRepresentationProperty &other);

    /**
     * Constructor. Sets the representation to the given value. If it is not
     * valid, the representation is set to Wireframe(1)
     * @param value the integer representation of the representation
     */
    GridRepresentationProperty(const IdType &value);

    /**
     * Constructor. Sets the representation to the given value. If it is not
     * valid, the representation is set to Wireframe(1)
     * @param value the string representation of the representation
     */
    GridRepresentationProperty(const std::string &value);

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

    mitkCloneMacro(GridRepresentationProperty);
  };
} // end of namespace mitk
#endif
