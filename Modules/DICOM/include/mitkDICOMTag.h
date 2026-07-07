/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTag_h
#define mitkDICOMTag_h

#include <mitkPoint.h>
#include <mitkVector.h>

#include <MitkDICOMExports.h>

namespace mitk
{

/**
  \ingroup DICOMModule
  \brief Representation of a DICOM tag.

  This class is just meant to combine group and element
  numbers for better readability and make handling tags
  more easy by offering comparison methods.
*/
class MITKDICOM_EXPORT DICOMTag
{
  public:

    /**
     * \brief Construct a DICOMTag from group and element numbers.
     * \param[in] group The DICOM tag group number (e.g. 0x0010).
     * \param[in] element The DICOM tag element number (e.g. 0x0010).
     */
    DICOMTag(unsigned int group, unsigned int element);

    /** \brief Copy constructor. */
    DICOMTag(const DICOMTag& other);

    /** \brief Assignment operator. */
    DICOMTag& operator=(const DICOMTag& other);

    /**
     * \brief Equality comparison operator.
     * \param[in] other The tag to compare with.
     * \return true if group and element are identical.
     */
    bool operator==(const DICOMTag& other) const;

    /**
     * \brief Less-than comparison operator (for sorting/containers).
     * \param[in] other The tag to compare with.
     * \return true if this tag is less than the other (group first, then element).
     */
    bool operator<(const DICOMTag& other) const;

    /**
     * \brief Get the DICOM group number.
     * \return The group number.
     */
    unsigned int GetGroup() const;

    /**
     * \brief Get the DICOM element number.
     * \return The element number.
     */
    unsigned int GetElement() const;

    /**
     * \brief Return the name of this tag (e.g. "SeriesDescription" instead of "(0008,103e)").
     * \return A human-readable tag name from the DICOM dictionary.
     */
    std::string GetName() const;

    /**
     * \brief Print "(group-id,element-id) name" to the given stream.
     * \param[in,out] os The output stream to print to.
     */
    void Print(std::ostream& os) const;

  private:

    std::string toHexString(unsigned int i) const;

    unsigned int m_Group;
    unsigned int m_Element;
};

/** \brief A list of DICOMTag instances. */
typedef std::vector<DICOMTag> DICOMTagList;

/**
  \brief Convert DICOM string describing a point to two Vector3D instances.

  DICOM tags like ImageOrientationPatient contain two vectors as float numbers separated by backslashes:
  \verbatim
  42.7131\13.77\0.7\137.76\0.3
  \endverbatim
 */
/**
 * \brief Parse a DICOM orientation string into two direction vectors.
 * \param[in] s The DICOM string (e.g. from ImageOrientationPatient).
 * \param[out] right The "right" direction vector.
 * \param[out] up The "up" direction vector.
 * \param[out] successful Set to true if parsing succeeded, false otherwise.
 */
void DICOMStringToOrientationVectors(const std::string& s, Vector3D& right, Vector3D& up, bool& successful);

/**
 * \brief Parse a DICOM spacing string into X and Y spacing values.
 * \param[in] s The DICOM string (e.g. from PixelSpacing).
 * \param[out] spacingX The spacing in X direction in mm.
 * \param[out] spacingY The spacing in Y direction in mm.
 * \return true if parsing succeeded, false otherwise.
 */
bool DICOMStringToSpacing(const std::string& s, ScalarType& spacingX, ScalarType& spacingY);


/**
  \brief Convert DICOM string describing a point to Point3D.

  DICOM tags like ImagePositionPatient contain a position as float numbers separated by backslashes:
  \verbatim
  42.7131\13.77\0.7
  \endverbatim
 */
Point3D DICOMStringToPoint3D(const std::string& s, bool& successful);

}

#endif
