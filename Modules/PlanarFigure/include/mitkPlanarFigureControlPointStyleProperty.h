/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureControlPointStyleProperty_h
#define mitkPlanarFigureControlPointStyleProperty_h

#include <mitkEnumerationProperty.h>

#include <MitkPlanarFigureExports.h>

namespace mitk
{
  /**
   * \brief Shape of control points, square or circle, of a previous planar figure mapper.
   *
   * Nothing reads it anymore. Scenes of previous versions contain it, and
   * reading them requires the property type to be known.
   *
   * \ingroup MitkPlanarFigureModule
   */
  class MITKPLANARFIGURE_EXPORT PlanarFigureControlPointStyleProperty : public EnumerationProperty
  {
  public:
    mitkClassMacro(PlanarFigureControlPointStyleProperty, EnumerationProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkNewMacro1Param(PlanarFigureControlPointStyleProperty, const IdType &);

    mitkNewMacro1Param(PlanarFigureControlPointStyleProperty, const std::string &);

    typedef enum { Square, Circle } Shape;

    virtual Shape GetShape();

    virtual void SetShape(Shape);

  protected:
    PlanarFigureControlPointStyleProperty();

    PlanarFigureControlPointStyleProperty(const PlanarFigureControlPointStyleProperty &other);

    PlanarFigureControlPointStyleProperty(const IdType &value);

    /**
     * Constructor. Sets the decoration type to the given value. If it is not
     * valid, the representation is set to none
     */
    PlanarFigureControlPointStyleProperty(const std::string &value);

    /**
     * this function is overridden as protected, so that the user may not add
     * additional invalid types.
     */
    bool AddEnum(const std::string &name, const IdType &id) override;

    /**
     * Adds the standard enumeration types with corresponding strings.
     */
    virtual void AddEnumTypes();

    mitkCloneMacro(PlanarFigureControlPointStyleProperty);

  private:
    // purposely not implemented
    PlanarFigureControlPointStyleProperty &operator=(const PlanarFigureControlPointStyleProperty &);
  };

} // end of namespace mitk

#endif
