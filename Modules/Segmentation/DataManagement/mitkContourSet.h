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


#ifndef _MITK_CONTOUR_SET_H_
#define _MITK_CONTOUR_SET_H_

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkBaseData.h"
#include "mitkContour.h"
#include  <map>

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

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef std::map<unsigned long,Contour::Pointer>              ContourVectorType;
  typedef ContourVectorType::iterator                           ContourIterator;
  typedef itk::BoundingBox<unsigned long, 3, ScalarType>        BoundingBoxType;

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
  virtual void UpdateOutputInformation() override;

  /**
  * intherited from parent
  */
  virtual void SetRequestedRegionToLargestPossibleRegion() override;

  /**
  * intherited from parent
  */
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

  /**
  * intherited from parent
  */
  virtual bool VerifyRequestedRegion() override;

  /**
  * intherited from parent
  */
  virtual void SetRequestedRegion( const itk::DataObject *data) override;

protected:
  ContourSet();
  virtual ~ContourSet();

private:

  /**
  * the bounding box of the contour
  */
  BoundingBoxType::Pointer m_BoundingBox;

  ContourVectorType m_ContourVector;

  unsigned int m_NumberOfContours;

};

} // namespace mitk

#endif //_MITK_CONTOUR_SET_H_
