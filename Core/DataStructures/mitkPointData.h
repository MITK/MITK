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

#ifndef MITK_POINT_DATA_H
#define MITK_POINT_DATA_H

#include <mitkBaseData.h>

namespace mitk {

class PointData : public mitk::BaseData
{
public:
  mitkClassMacro( PointData, BaseData );

  itkNewMacro( Self );

  itkGetMacro( Point3D, mitk::Point3D );

  itkSetMacro( Point3D, mitk::Point3D );

  itkGetMacro( Point2D, mitk::Point2D );

  itkSetMacro( Point2D, mitk::Point2D );

  virtual void UpdateOutputInformation();
  
  virtual void SetRequestedRegionToLargestPossibleRegion();
  
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  
  virtual bool VerifyRequestedRegion();
 
  virtual void SetRequestedRegion(itk::DataObject *data);

  virtual void CopyInformation(const itk::DataObject *data);

protected:

  PointData();

  virtual ~PointData();

  mitk::Point3D m_Point3D;

  mitk::Point2D m_Point2D;

};


}


#endif
