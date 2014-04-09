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
 * Encapsulates the enumeration for rendering modes. The property human-readable name (which is
 * used in the mitkWorkbench inside the Property View) is "Image Rendering.Mode". This property
 * affects rendering of images and is used inside the mitkImageVtkMapper2D to define which
 * rendering mode is applied to images.
 * Valid values are:
 *
 * \li LEVELWINDOW_COLOR is DEPRECATED and mapped to LOOKUPTABLE_LEVELWINDOW_COLOR.
 * \li LOOKUPTABLE_LEVELWINDOW_COLOR: A lookup table, level window and color will be applied to the image.
 * As lookup table, the table object supplied by the property "LookupTable" will be used. As default,
 * we apply a GRAYSCALE mitk::LookupTable. Note, if you want to use a former rainbow-like lookup table,
 * set the mitk::LookupTableProperty to use a LEGACY_RAINBOW_COLOR type mitk::LookupTable.
 * Our default setup using the GRAYSCALE mitk::LookupTable for a test image looks like this:
 * \image html ExampleLevelWindowColor.png
 * This image can be reproduced with the mitkImageVtkMapper2DColorTest or mitkImageVtkMapper2DLevelWindowTest.
 * If "Image Rendering.Mode" is set to LOOKUPTABLE_LEVELWINDOW_COLOR inside the mitkWorkbench, the level window
 * slider will change the rendering of the image. That means it will change the values of an internally used
 * default lookup table. Note, the level window slider changes the property "levelwindow" which modifies the range
 * of the internally used lookup table.
 * This mode will apply the "color" property. The default color is white. If you change the "color"
 * property to yellow, the test image will be rendered like this:
 * \image html ExampleColorYellow.png
 * This image can be reproduced with the mitkImageVtkMapper2DColorTest.
 * Our test image with a lookup table mapping everything from red to blue looks like this:
 * \image html ExampleLookupTable.png
 * This image can be reproduced with the mitkImageVtkMapper2DLookupTableTest. Check this test code for an example how
 * to apply a lookup table to an image.
 * \note Changing a lookup table via the "levelwindow" property can be unintuitive and unwanted since the
 * level window slider will overwrite the range of the lookup table. Use LOOKUPTABLE_COLOR if you
 * don't want your lookuptable to be influenced by the "levelwindow" property.
 *
 * \li COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR: A color transfer function, level window and color will be applied to the image.
 * Very similar mode to LOOKUPTABLE_LEVELWINDOW_COLOR. Instead of the lookup table a color transfer function will be used.
 * Color transfer functions are useful to colorize floating point images and allow sometimes more flexibility than
 * a lookup table. The "Image Rendering.Transfer Function" property defines the transfer function. Our test image
 * with a transfer function mapping everything from to red, green and blue looks like this:
 * \image html ExampleTransferFunction.png
 * This image can be reproduced with the mitkImageVtkMapper2DTransferFunctionTest. Check the test code for
 * an example how to define a transfer function for an image. This transfer function
 * will be influenced by the property "levelwindow" and the actor will be colored by the
 * "color" property.
 * \note Changing a transfer function table via the "levelwindow" property can be unintuitive and unwanted since
 * the level window slider will overwrite the.Use COLORTRANSFERFUNCTION_COLOR if you don't want your transfer
 * function to be influenced by the level window.
 *
 * \li LOOKUPTABLE_COLOR: A lookup table and color will be applied to the image.
 * Similar mode to LOOKUPTABLE_LEVELWINDOW_COLOR, except that the "levelwindow" property will not
 * modify the range of the lookup table.
 *
 * \li COLORTRANSFERFUNCTION_COLOR: A color trans ferfunction and color will be applied to the image.
 * Similar mode to COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR, except that the "levelwindow" property will not
 * modify the range of the transfer function.
 *
 * The order is given by the names (e.g. LOOKUPTABLE_COLOR applies first a lookup table and next a color).
 * Currently, there is no GUI (in mitkWorkbench) support for controlling lookup tables or transfer functions.
 * This has to be done by the programmer. Color and level window are controlled by color widget and level window slider.
 * Currently, the color is always applied. We do not set the color to white, if the user changes the mode. We assume
 * that users who change the mode know that a previously set color will still be applied (on top of the respective mode).
 * See VTK documentation for examples how to use vtkTransferfunction and vtkLookupTable.
 */

class MITK_CORE_EXPORT RenderingModeProperty : public EnumerationProperty
{
public:

  mitkClassMacro( RenderingModeProperty, EnumerationProperty );

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  mitkNewMacro1Param(RenderingModeProperty, const IdType&);

  mitkNewMacro1Param(RenderingModeProperty, const std::string&);

  //Never (!) change this without adaptation of mitkLevelWindowManagerTest::VerifyRenderingModes and mitkLevelWindowManagerTest::TestLevelWindowSliderVisibility !
  enum ImageRenderingMode
  {
    //0 used to be LEVELWINDOW_COLOR which is deprecated now and will be mapped to LOOKUPTABLE_LEVELWINDOW_COLOR.
    //Our default lookup table property is the GRAYSCALE type which represents the
    //former LEVELWINDOW_COLOR mode.
    LOOKUPTABLE_LEVELWINDOW_COLOR = 1,
    COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR = 2,
    LOOKUPTABLE_COLOR = 3,
    COLORTRANSFERFUNCTION_COLOR = 4
  //  Default = LOOKUPTABLE_LEVELWINDOW_COLOR;
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
