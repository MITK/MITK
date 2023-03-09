/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointSetShapeProperty_h
#define mitkPointSetShapeProperty_h

#include "mitkEnumerationProperty.h"

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * Encapsulates the enumeration for point set shapes. Valid values are:
   * \li NONE
   * \li VERTEX
   * \li DASH
   * \li CROSS (default)
   * \li THICK_CROSS
   * \li TRIANGLE
   * \li SQUARE
   * \li CIRCLE
   * \li DIAMOND
   * \li ARROW
   * \li THICK_ARROW
   * \li HOOKED_ARROW
   *
   * This class provides different shapes for the point set rendering (unselected points).
   *
   */

  class MITKCORE_EXPORT PointSetShapeProperty : public EnumerationProperty
  {
  public:
    mitkClassMacro(PointSetShapeProperty, EnumerationProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      mitkNewMacro1Param(PointSetShapeProperty, const IdType &);

    mitkNewMacro1Param(PointSetShapeProperty, const std::string &);

    enum PointSetShape
    {
      NONE = 0,
      VERTEX = 1,
      DASH = 2,
      CROSS = 3,
      THICK_CROSS = 4,
      TRIANGLE = 5,
      SQUARE = 6,
      CIRCLE = 7,
      DIAMOND = 8,
      ARROW = 9,
      THICK_ARROW = 10,
      HOOKED_ARROW = 11
      //  Default = CROSS;
    };

    /**
     * Returns the current shape
     */
    virtual int GetPointSetShape() const;

    using BaseProperty::operator=;

  protected:
    /** Sets rendering type to default (VTK_RAY_CAST_COMPOSITE_FUNCTION).
     */
    PointSetShapeProperty();

    /**
     * Constructor. Sets rendering type to the given value.
     */
    PointSetShapeProperty(const IdType &value);

    /**
     * Constructor. Sets rendering type to the given value.
     */
    PointSetShapeProperty(const std::string &value);

    /**
     * this function is overridden as protected, so that the user may not add
     * additional invalid rendering types.
     */
    bool AddEnum(const std::string &name, const IdType &id) override;

    /**
     * Adds the default enumeration types.
     */
    virtual void AddPointSetShapes();

  private:
    // purposely not implemented
    PointSetShapeProperty &operator=(const PointSetShapeProperty &);

    itk::LightObject::Pointer InternalClone() const override;
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // end of namespace mitk

#endif
