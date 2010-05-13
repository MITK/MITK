/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-14 19:45:53 +0200 (Mo, 14 Apr 2008) $
Version:   $Revision: 14081 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkPlanarFigureControlPointStyleProperty_h
#define mitkPlanarFigureControlPointStyleProperty_h

#include "mitkEnumerationProperty.h"

#include "PlanarFigureExports.h"

namespace mitk
{

/**
 * \brief Defines the rendering style of control points for PlanarFigure objects.
 * \sa mitk::PlanarFigureMapper2D
 * \ingroup DataManagement
 * 
 * Used by PlanarFigureMapper2D to determine which of several control point shapes to use.
 * Currently this is basically the choice between squares and circles. If more options
 * are implemented, this class should be enhanced.
 *
 * After construction, the default shape is a square.
 *
 */
class PlanarFigure_EXPORT PlanarFigureControlPointStyleProperty : public EnumerationProperty
{
public:

  mitkClassMacro( PlanarFigureControlPointStyleProperty, EnumerationProperty );
  
  itkNewMacro(PlanarFigureControlPointStyleProperty);
  
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
};

} // end of namespace mitk


#endif
