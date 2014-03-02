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

#ifndef MITK_NAMED_POINT_H
#define MITK_NAMED_POINT_H

#include <mitkVector.h>
#include <string>

namespace mitk {

class NamedPoint{

  std::string m_Name;
  mitk::Point3D m_Point;

public:

  /**
    *
    */
  NamedPoint() {
  };

  /**
    *
    */
  NamedPoint( const NamedPoint& namedPoint )
    : m_Name( namedPoint.m_Name ), m_Point( namedPoint.m_Point ) {

  };

  /**
    *
    */
  NamedPoint( const std::string name, const Point3D& point )
    : m_Name( name ), m_Point( point ) {

  };


  /**
    *
    */
  const std::string& GetName() const {

    return m_Name;
  };

  /**
    *
    */
  void SetName( const std::string& name ) {

    m_Name = name;
  };

  /**
    *
    */
  const Point3D& GetPoint() const {

    return m_Point;
  };

  /**
    *
    */
  void SetPoint( const Point3D& point ) {

    m_Point = point;
  };
};

} // mitk
#endif // MITK_NAMED_POINT_H
