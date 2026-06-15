/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneOrientationProperty_h
#define mitkPlaneOrientationProperty_h

#include <mitkEnumerationProperty.h>

namespace mitk
{
  /**
   * \brief Enumeration property controlling orientation arrows on 2D plane representations.
   *
   * Controls whether the 2D line representation of a PlaneGeometry
   * should have small arrows at both ends to indicate the orientation of
   * the plane, and whether the arrows should be oriented in the direction of
   * the plane's normal or against it.
   *
   * Valid values of the enumeration property are:
   * - PLANE_DECORATION_NONE (no arrows)
   * - PLANE_DECORATION_POSITIVE_ORIENTATION (arrows pointing in positive normal direction)
   * - PLANE_DECORATION_NEGATIVE_ORIENTATION (arrows pointing in negative normal direction)
   *
   * \sa PlaneGeometryDataMapper2D::DrawOrientationArrow()
   * \sa EnumerationProperty
   */
  class MITKCORE_EXPORT PlaneOrientationProperty : public EnumerationProperty
  {
  public:
    mitkClassMacro(PlaneOrientationProperty, EnumerationProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    mitkNewMacro1Param(PlaneOrientationProperty, const IdType &);

    mitkNewMacro1Param(PlaneOrientationProperty, const std::string &);

    enum
    {
      PLANE_DECORATION_NONE,
      PLANE_DECORATION_POSITIVE_ORIENTATION,
      PLANE_DECORATION_NEGATIVE_ORIENTATION
    };

    /**
     * Returns the state of plane decoration.
     */
    virtual int GetPlaneDecoration();

    /**
     * Sets the decoration type to no decoration.
     */
    virtual void SetPlaneDecorationToNone();

    /**
     * Sets the decoration type to arrows in positive plane direction.
     */
    virtual void SetPlaneDecorationToPositiveOrientation();

    /**
     * Sets the decoration type to arrows in negative plane direction.
     */
    virtual void SetPlaneDecorationToNegativeOrientation();

    using BaseProperty::operator=;

  protected:
    /**
     * Constructor. Sets the decoration type to none.
     */
    PlaneOrientationProperty();

    /**
     * Constructor. Sets the decoration type to the given value. If it is not
     * valid, the interpolation is set to none
     */
    PlaneOrientationProperty(const IdType &value);

    /**
     * Constructor. Sets the decoration type to the given value. If it is not
     * valid, the representation is set to none
     */
    PlaneOrientationProperty(const std::string &value);

    PlaneOrientationProperty(const PlaneOrientationProperty &other);

    /**
     * this function is overridden as protected, so that the user may not add
     * additional invalid types.
     */
    bool AddEnum(const std::string &name, const IdType &id) override;

    /**
     * Adds the standard enumeration types with corresponding strings.
     */
    virtual void AddDecorationTypes();

    mitkCloneMacro(PlaneOrientationProperty);

  private:
    // purposely not implemented
    PlaneOrientationProperty &operator=(const PlaneOrientationProperty &);
  };


} // end of namespace mitk

#endif
