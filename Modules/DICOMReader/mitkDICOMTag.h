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

#ifndef mitkTag_h
#define mitkTag_h

#include "mitkVector.h"

#include "DICOMReaderExports.h"

namespace mitk
{

class DICOMReader_EXPORT DICOMTag
{
  public:

    DICOMTag(unsigned int group, unsigned int element);
    DICOMTag(const DICOMTag& other);
    DICOMTag& operator=(const DICOMTag& other);
    bool operator==(const DICOMTag& other) const;
    bool operator<(const DICOMTag& other) const;

    unsigned int GetGroup() const;
    unsigned int GetElement() const;

    std::string GetName() const;
    void Print(std::ostream& os) const;

  protected:

    std::string toHexString(unsigned int i) const;

    unsigned int m_Group;
    unsigned int m_Element;
};

typedef std::vector<DICOMTag> DICOMTagList;

/**
  \brief Convert DICOM string describing a point two Vector3D.

  DICOM tags like ImageOrientationPatient contain two vectors as float numbers separated by backslashes:
  \verbatim
  42.7131\13.77\0.7\137.76\0.3
  \endverbatim
 */
void DICOMStringToOrientationVectors(const std::string& s, Vector3D& right, Vector3D& up, bool& successful);

bool DICOMStringToSpacing(const std::string& s, ScalarType& spacingX, ScalarType& spacingY);


/**
  \brief Convert DICOM string describing a point to Point3D.

  DICOM tags like ImagePositionPatient contain a position as float numbers separated by backslashes:
  \verbatim
  42.7131\13.77\0.7
  \endverbatim
 */
mitk::Point3D DICOMStringToPoint3D(const std::string& s, bool& successful);

}

#endif
