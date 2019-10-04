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

#include <mitkDicomSeriesReader.h>

namespace mitk
{

DicomSeriesReader::GantryTiltInformation::GantryTiltInformation()
: m_ShiftUp(0.0)
, m_ShiftRight(0.0)
, m_ShiftNormal(0.0)
, m_ITKAssumedSliceSpacing(0.0)
, m_NumberOfSlicesApart(1)
{
}


#define doublepoint(x) \
  Point3Dd x; \
  x[0] = x ## f[0]; \
  x[1] = x ## f[1]; \
  x[2] = x ## f[2];


#define doublevector(x) \
  Vector3Dd x; \
  x[0] = x ## f[0]; \
  x[1] = x ## f[1]; \
  x[2] = x ## f[2];

DicomSeriesReader::GantryTiltInformation::GantryTiltInformation(
    const Point3D& origin1f, const Point3D& origin2f,
    const Vector3D& rightf, const Vector3D& upf,
    unsigned int numberOfSlicesApart)
: m_ShiftUp(0.0)
, m_ShiftRight(0.0)
, m_ShiftNormal(0.0)
, m_NumberOfSlicesApart(numberOfSlicesApart)
{
  assert(numberOfSlicesApart);

  doublepoint(origin1);
  doublepoint(origin2);
  doublevector(right);
  doublevector(up);

  // determine if slice 1 (imagePosition1 and imageOrientation1) and slice 2 can be in one orthogonal slice stack:
  // calculate a line from origin 1, directed along the normal of slice (calculated as the cross product of orientation 1)
  // check if this line passes through origin 2

  /*
     Determine if line (imagePosition2 + l * normal) contains imagePosition1.
     Done by calculating the distance of imagePosition1 from line (imagePosition2 + l *normal)

     E.g. http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html

     squared distance = | (pointAlongNormal - origin2) x (origin2 - origin1) | ^ 2
     /
     |pointAlongNormal - origin2| ^ 2

     ( x meaning the cross product )
  */

  Vector3Dd normal = itk::CrossProduct(right, up);

  double numerator = itk::CrossProduct( normal, origin2 - origin1 ).GetSquaredNorm();
  double denominator = normal.GetSquaredNorm();

  double distance = sqrt(numerator / denominator);

  if ( distance > 0.001 ) // mitk::eps is too small; 1/1000 of a mm should be enough to detect tilt
  {
    MITK_DEBUG << "  Series seems to contain a tilted (or sheared) geometry";
    MITK_DEBUG << "  Distance of expected slice origin from actual slice origin: " << distance;
    MITK_DEBUG << "    ==> storing this shift for later analysis:";
    MITK_DEBUG << "    v right: " << right;
    MITK_DEBUG << "    v up: " << up;
    MITK_DEBUG << "    v normal: " << normal;

    Point3Dd projectionRight = projectPointOnLine( origin1, origin2, right );
    Point3Dd projectionNormal = projectPointOnLine( origin1, origin2, normal );

    m_ShiftRight = (projectionRight - origin2).GetNorm();
    m_ShiftNormal = (projectionNormal - origin2).GetNorm();

    /*
       now also check to which side the image is shifted.

       Calculation e.g. from
       http://mathworld.wolfram.com/Point-PlaneDistance.html
    */

    Point3Dd  testPoint = origin1;
    Vector3Dd planeNormal = up;

    double signedDistance = (
                             planeNormal[0] * testPoint[0]
                           + planeNormal[1] * testPoint[1]
                           + planeNormal[2] * testPoint[2]
                           - (
                               planeNormal[0] * origin2[0]
                             + planeNormal[1] * origin2[1]
                             + planeNormal[2] * origin2[2]
                             )
                           )
                           /
                           sqrt( planeNormal[0] * planeNormal[0]
                               + planeNormal[1] * planeNormal[1]
                               + planeNormal[2] * planeNormal[2]
                               );

    m_ShiftUp = signedDistance;

    m_ITKAssumedSliceSpacing = (origin2 - origin1).GetNorm();
    // How do we now this is assumed? See header documentation for ITK code references
    //double itkAssumedSliceSpacing = sqrt( m_ShiftUp * m_ShiftUp + m_ShiftNormal * m_ShiftNormal );

    MITK_DEBUG << "    shift normal: " << m_ShiftNormal;
    MITK_DEBUG << "    shift normal assumed by ITK: " << m_ITKAssumedSliceSpacing;
    MITK_DEBUG << "    shift up: " << m_ShiftUp;
    MITK_DEBUG << "    shift right: " << m_ShiftRight;

    MITK_DEBUG << "    tilt angle (deg): " << atan( m_ShiftUp / m_ShiftNormal ) * 180.0 / 3.1415926535;
  }
}

Point3D
DicomSeriesReader::GantryTiltInformation::projectPointOnLine( Point3Dd p, Point3Dd lineOrigin, Vector3Dd lineDirection )
{
  /**
    See illustration at http://mo.mathematik.uni-stuttgart.de/inhalt/aussage/aussage472/

    vector(lineOrigin,p) = normal * ( innerproduct((p - lineOrigin),normal) / squared-length(normal) )
  */

  Vector3Dd lineOriginToP = p - lineOrigin;
  double innerProduct = lineOriginToP * lineDirection;

  double factor = innerProduct / lineDirection.GetSquaredNorm();
  Point3Dd projection = lineOrigin + factor * lineDirection;

  return projection;
}

double
DicomSeriesReader::GantryTiltInformation::GetTiltCorrectedAdditionalSize() const
{
  return fabs(m_ShiftUp);
}

double
DicomSeriesReader::GantryTiltInformation::GetTiltAngleInDegrees() const
{
  return atan( fabs(m_ShiftUp) / m_ShiftNormal ) * 180.0 / 3.1415926535;
}

double
DicomSeriesReader::GantryTiltInformation::GetMatrixCoefficientForCorrectionInWorldCoordinates() const
{
  // so many mm need to be shifted per slice!
  return m_ShiftUp / static_cast<double>(m_NumberOfSlicesApart);
}

double
DicomSeriesReader::GantryTiltInformation::GetRealZSpacing() const
{
  return m_ShiftNormal / static_cast<double>(m_NumberOfSlicesApart);
}


bool
DicomSeriesReader::GantryTiltInformation::IsSheared() const
{
  return (   fabs(m_ShiftRight) > 0.001
          ||    fabs(m_ShiftUp) > 0.001);
}


bool
DicomSeriesReader::GantryTiltInformation::IsRegularGantryTilt() const
{
  return (   fabs(m_ShiftRight) < 0.001
          &&    fabs(m_ShiftUp) > 0.001);
}

} // end namespace mitk
