/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIMapperLocalStorage_h
#define mitkROIMapperLocalStorage_h

#include <mitkMapper.h>

template <class T>
class vtkSmartPointer;

class vtkPropAssembly;

namespace mitk
{
  /** \brief Common base class for both 2-d and 3-d %ROI mapper local storages.
    */
  class ROIMapperLocalStorage : public Mapper::BaseLocalStorage
  {
  public:
    ROIMapperLocalStorage();
    ~ROIMapperLocalStorage() override;

    vtkPropAssembly* GetPropAssembly() const;
    void SetPropAssembly(vtkPropAssembly* propAssembly);

    TimePointType GetLastTimePoint() const;
    void SetLastTimePoint(TimePointType timePoint);

  protected:
    vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
    TimePointType m_LastTimePoint;
  };
}

#endif
