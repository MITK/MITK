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

#ifndef _MITK_RENDERING_MODE_PROPERTY__H_
#define _MITK_RENDERING_MODE_PROPERTY__H_

#include "mitkEnumerationProperty.h"

namespace mitk
{

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4522)
#endif

/**
 * Encapsulates the enumeration for rendering modes. Valid values are:
 * \li LEVELWINDOW_COLOR: Level window and color will be applied to the image.
 * \li LOOKUPTABLE_LEVELWINDOW_COLOR: A lookup table, level window and color will be applied to the image.
 * \li COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR: A colortransferfunction, level window and color will be applied to the image.
 * \li LOOKUPTABLE_COLOR: A lookup table and color will be applied to the image.
 * \li COLORTRANSFERFUNCTION_COLOR: A colortransferfunction and color will be applied to the image.
 *
 * The order is given by the names (e.g. LOOKUPTABLE_COLOR applies first a lookup table and next a color).
 * Currently, there is no GUI (in mitkWorkbench) support for controlling lookup tables or transfer functions.
 * This has to be done by the programmer. Color and level window are controled by color widget and level window slider.
 * Currently, the color is always applied. We do not set the color to white, if the user changes the mode. We assume
 * that users who change the mode know that a previously set color will still be applied (on top of the mode).
 */

class MITK_CORE_EXPORT RenderingModeProperty : public EnumerationProperty
{
public:

  mitkClassMacro( RenderingModeProperty, EnumerationProperty );

  itkNewMacro(RenderingModeProperty);

  mitkNewMacro1Param(RenderingModeProperty, const IdType&);

  mitkNewMacro1Param(RenderingModeProperty, const std::string&);

  enum ImageRenderingMode
  {
    LEVELWINDOW_COLOR = 0,
    LOOKUPTABLE_LEVELWINDOW_COLOR = 1,
    COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR = 2,
    LOOKUPTABLE_COLOR = 3,
    COLORTRANSFERFUNCTION_COLOR = 4,
    ISODOSESHADER_COLOR = 5
  //  Default = LEVELWINDOW_COLOR;
  };

  /**
   * Returns the current rendering mode
   */
  virtual int GetRenderingMode();

  using BaseProperty::operator=;

 protected:

  /** Sets rendering type to default (VTK_RAY_CAST_COMPOSITE_FUNCTION).
   */
  RenderingModeProperty( );

  /**
   * Constructor. Sets rendering type to the given value.
   */
  RenderingModeProperty( const IdType& value );

  /**
   * Constructor. Sets rendering type to the given value.
   */
  RenderingModeProperty( const std::string& value );

  /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid rendering types.
   */
  virtual bool AddEnum( const std::string& name, const IdType& id );

  /**
   * Adds the default enumeration types.
   */
  virtual void AddRenderingModes();

private:

  // purposely not implemented
  RenderingModeProperty& operator=(const RenderingModeProperty&);

  virtual itk::LightObject::Pointer InternalClone() const;
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

} // end of namespace mitk

#endif
