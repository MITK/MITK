/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourSet_h
#define mitkContourSet_h

#include "mitkBaseData.h"
#include "mitkCommon.h"
#include "mitkContour.h"
#include <MitkSegmentationExports.h>
#include <map>

namespace mitk
{
  /**
  * This class holds stores vertices for drawing a contour
  *
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

    /**
    * clean up the contour data
    */
    void Initialize() override;

    /**
    * add a contour
    */
    void AddContour(unsigned int index, mitk::Contour::Pointer contour);

    /**
    * add a contour
    */
    void RemoveContour(unsigned long index);

    /**
    * returns the number of points stored in the contour
    */
    unsigned int GetNumberOfContours();

    /**
    * returns the container of the contour points
    */
    ContourVectorType GetContours();

    /**
    * intherited from parent
    */
    void UpdateOutputInformation() override;

    /**
    * intherited from parent
    */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
    * intherited from parent
    */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
    * intherited from parent
    */
    bool VerifyRequestedRegion() override;

    /**
    * intherited from parent
    */
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
