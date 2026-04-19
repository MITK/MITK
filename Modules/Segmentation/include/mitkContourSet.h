/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourSet_h
#define mitkContourSet_h

#include <mitkBaseData.h>
#include <mitkCommon.h>
#include <mitkContour.h>
#include <MitkSegmentationExports.h>
#include <map>

namespace mitk
{
  /**
   * \brief Holds a collection of mitk::Contour objects indexed by ID.
   *
   * \note mitk::ContourModelSet is the preferred alternative for new code.
   * \sa ContourModelSet, Contour
   */
  class MITKSEGMENTATION_EXPORT ContourSet : public BaseData
  {
  public:
    mitkClassMacro(ContourSet, BaseData);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      typedef std::map<unsigned long, Contour::Pointer> ContourVectorType;
    typedef ContourVectorType::iterator ContourIterator;
    typedef itk::BoundingBox<unsigned long, 3, ScalarType> BoundingBoxType;

    /** \brief Clears all contour data. */
    void Initialize() override;

    /**
     * \brief Adds a contour at the given index.
     * \param[in] index The index key for the contour.
     * \param[in] contour The contour to add.
     */
    void AddContour(unsigned int index, mitk::Contour::Pointer contour);

    /**
     * \brief Removes the contour at the given index.
     * \param[in] index The index key of the contour to remove.
     */
    void RemoveContour(unsigned long index);

    /**
     * \brief Returns the number of contours stored in the set.
     * \return The contour count.
     */
    unsigned int GetNumberOfContours();

    /**
     * \brief Returns the map of all contours.
     * \return The contour vector map.
     */
    ContourVectorType GetContours();

    /** \brief Updates the output information (bounding box, etc.). */
    void UpdateOutputInformation() override;

    /** \brief Sets the requested region to the largest possible region. */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /** \brief Checks whether the requested region is outside the buffered region. */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /** \brief Verifies that the requested region is valid. */
    bool VerifyRequestedRegion() override;

    /** \brief Sets the requested region from an itk::DataObject. */
    void SetRequestedRegion(const itk::DataObject *data) override;

  protected:
    ContourSet();
    ~ContourSet() override;

  private:
    /**
    * the bounding box of the contour
    */
    BoundingBoxType::Pointer m_BoundingBox;

    ContourVectorType m_ContourVector;

  };

} // namespace mitk

#endif
