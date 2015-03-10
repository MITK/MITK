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

#ifndef mitkPlanarFigureControlPointStyleProperty_h
#define mitkPlanarFigureControlPointStyleProperty_h

#include "mitkEnumerationProperty.h"

#include <MitkPlanarFigureExports.h>

namespace mitk
{

/**
 * \brief Defines the rendering style of control points for PlanarFigure objects.
 * \sa mitk::PlanarFigureMapper2D
 * @ingroup MitkPlanarFigureModule
 *
 * Used by PlanarFigureMapper2D to determine which of several control point shapes to use.
 * Currently this is basically the choice between squares and circles. If more options
 * are implemented, this class should be enhanced.
 *
 * After construction, the default shape is a square.
 *
 */
class MITKPLANARFIGURE_EXPORT PlanarFigureControlPointStyleProperty : public EnumerationProperty
{
public:

  mitkClassMacro( PlanarFigureControlPointStyleProperty, EnumerationProperty );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  mitkNewMacro1Param(PlanarFigureControlPointStyleProperty, const IdType&);

  mitkNewMacro1Param(PlanarFigureControlPointStyleProperty, const std::string&);

  typedef enum
  {
    Square, Circle
  } Shape;

  virtual Shape GetShape();

  virtual void SetShape( Shape );

protected:

  PlanarFigureControlPointStyleProperty( );

  PlanarFigureControlPointStyleProperty( const IdType &value );

  /**
   * Constructor. Sets the decoration type to the given value. If it is not
   * valid, the representation is set to none
   */
  PlanarFigureControlPointStyleProperty( const std::string &value );


  /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid types.
   */
  virtual bool AddEnum( const std::string &name, const IdType &id );

  /**
   * Adds the standard enumeration types with corresponding strings.
   */
  virtual void AddEnumTypes();

private:

  // purposely not implemented
  PlanarFigureControlPointStyleProperty& operator=(const PlanarFigureControlPointStyleProperty&);

  virtual itk::LightObject::Pointer InternalClone() const;
};

} // end of namespace mitk


#endif
