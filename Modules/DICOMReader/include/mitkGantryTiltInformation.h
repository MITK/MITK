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

#ifndef mitkGantryTiltInformation_h
#define mitkGantryTiltInformation_h

#include "mitkPoint.h"
#include "mitkVector.h"
#include "mitkPoint.h"

namespace mitk
{

/**
  \ingroup DICOMReaderModule
  \brief Gantry tilt analysis result.

  Takes geometry information for two slices of a DICOM series and
  calculates whether these fit into an orthogonal block or not.
  If NOT, they can either be the result of an acquisition with
  gantry tilt OR completly broken by some shearing transformation.

  Most calculations are done in the constructor, results can then
  be read via the remaining methods.

  This class is a helper to DICOMITKSeriesGDCMReader and can
  not be used outside of \ref DICOMReaderModule
 */
class GantryTiltInformation
{
  public:

    // two types to avoid any rounding errors
    typedef  itk::Point<double,3> Point3Dd;
    typedef itk::Vector<double,3> Vector3Dd;

    /**
      \brief Just so we can create empty instances for assigning results later.
     */
    GantryTiltInformation();

    void Print(std::ostream& os) const;

    /**
      \brief THE constructor, which does all the calculations.

      Determining the amount of tilt is done by checking the distances
      of origin1 from planes through origin2. Two planes are considered:
      - normal vector along normal of slices (right x up): gives the slice distance
      - normal vector along orientation vector "up": gives the shift parallel to the plane orientation

      The tilt angle can then be calculated from these distances

      \param origin1 origin of the first slice
      \param origin2 origin of the second slice
      \param right right/up describe the orientatation of borth slices
      \param up right/up describe the orientatation of borth slices
      \param numberOfSlicesApart how many slices are the given origins apart (1 for neighboring slices)
     */
    GantryTiltInformation( const Point3D& origin1,
        const Point3D& origin2,
        const Vector3D& right,
        const Vector3D& up, unsigned int numberOfSlicesApart);

    /**
      \brief Factory method to create GantryTiltInformation from tag values (strings).

      Parameters as the regular c'tor.
    */
    static
    GantryTiltInformation
    MakeFromTagValues( const std::string& origin1String,
        const std::string& origin2String,
        const std::string orientationString,
        unsigned int numberOfSlicesApart);

    /**
      \brief Whether the slices were sheared.

      True if any of the shifts along right or up vector are non-zero.
     */
    bool IsSheared() const;

    /**
      \brief Whether the shearing is a gantry tilt or more complicated.

      Gantry tilt will only produce shifts in ONE orientation, not in both.

      Since the correction code currently only coveres one tilt direction
      AND we don't know of medical images with two tilt directions, the
      loading code wants to check if our assumptions are true.
     */
    bool IsRegularGantryTilt() const;

    /**
      \brief The offset distance in Y direction for each slice in mm (describes the tilt result).
     */
    double GetMatrixCoefficientForCorrectionInWorldCoordinates() const;


    /**
      \brief The z / inter-slice spacing. Needed to correct ImageSeriesReader's result.
     */
    double GetRealZSpacing() const;

    /**
      \brief The shift between first and last slice in mm.

      Needed to resize an orthogonal image volume.
     */
    double GetTiltCorrectedAdditionalSize(unsigned int imageSizeZ) const;

    /**
      \brief Calculated tilt angle in degrees.
     */
    double GetTiltAngleInDegrees() const;

  private:

    /**
      \brief Projection of point p onto line through lineOrigin in direction of lineDirection.
     */
    Point3D projectPointOnLine( Point3Dd p, Point3Dd lineOrigin, Vector3Dd lineDirection );

    double m_ShiftUp;
    double m_ShiftRight;
    double m_ShiftNormal;
    double m_ITKAssumedSliceSpacing;
    unsigned int m_NumberOfSlicesApart;
};

} // namespace

#endif
