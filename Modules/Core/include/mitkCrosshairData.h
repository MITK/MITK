/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCrosshairData_h
#define mitkCrosshairData_h

#include <mitkBaseData.h>
#include <mitkPoint.h>

namespace mitk
{
  class MITKCORE_EXPORT CrosshairData : public BaseData
  {
  public:

    mitkClassMacro(CrosshairData, BaseData);
    itkFactorylessNewMacro(Self);

    itkGetConstMacro(Position, Point3D);
    itkSetMacro(Position, Point3D);

    void SetRequestedRegionToLargestPossibleRegion() override;
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    bool VerifyRequestedRegion() override;
    void SetRequestedRegion(const itk::DataObject* data) override;

  protected:

    CrosshairData();
    ~CrosshairData() override;

    Point3D m_Position;

  };
}

#endif
