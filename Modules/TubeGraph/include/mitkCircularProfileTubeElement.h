/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_CircularProfileTubeElement_H
#define _MITK_CircularProfileTubeElement_H

#include <MitkTubeGraphExports.h>

#include "mitkTubeElement.h"

namespace mitk
{
  /**
  * \brief Class for elements which describes tubular structur with a circular cross section.
  */
  class MITKTUBEGRAPH_EXPORT CircularProfileTubeElement : virtual public TubeElement
  {
  public:
    CircularProfileTubeElement();
    CircularProfileTubeElement(float x, float y, float z, float d = 0.0f);
    CircularProfileTubeElement(const Point3D, float d = 0.0f);
    ~CircularProfileTubeElement() override;

    /**
    * Set the 3D position of the element.
    */
    void SetCoordinates(Point3D coordinates) override;

    /**
    * Returns the 3D position of the element.
    */
    const Point3D &GetCoordinates() const override;

    /**
    * Set the diameter of the circle.
    */
    void SetDiameter(float d);

    /**
    * Returns the diameter of the circle.
    */
    float GetDiameter() const;

    /**
    * Comparison operation between this object and the given object.
    * @param right The object to compare with.
    * @return true, if the object is the same;false, if not.
    */
    bool operator==(const TubeElement &right) const override;

  private:
    Point3D m_coordinates;
    float m_diameter;

  }; // class

} // namespace
#endif
