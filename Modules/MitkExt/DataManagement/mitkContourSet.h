/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_CONTOUR_SET_H_
#define _MITK_CONTOUR_SET_H_

#include "mitkCommon.h"
#include "mitkBaseData.h"
#include "mitkContour.h"
#include  <map>

namespace mitk 
{

/**
* This class holds stores vertices for drawing a contour 
*
*/
class MITK_CORE_EXPORT ContourSet : public BaseData
{
public:
  mitkClassMacro(ContourSet, BaseData);

  itkNewMacro(Self);

  typedef std::map<unsigned long,Contour::Pointer>              ContourVectorType;
  typedef ContourVectorType::iterator                           ContourIterator;
  typedef itk::BoundingBox<unsigned long, 3, ScalarType>        BoundingBoxType;

  /**
  * clean up the contour data
  */
  void Initialize();

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
  virtual void UpdateOutputInformation();

  /**
  * intherited from parent
  */
  virtual void SetRequestedRegionToLargestPossibleRegion();

  /**
  * intherited from parent
  */
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

  /**
  * intherited from parent
  */
  virtual bool VerifyRequestedRegion();

  /**
  * intherited from parent
  */
  virtual void SetRequestedRegion(itk::DataObject *data);

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
