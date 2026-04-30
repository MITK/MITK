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
  /** \brief Common base class for both 2D and 3D %ROI mapper local storages.
    *
    * Stores per-renderer state shared by ROIMapper2D and ROIMapper3D, including the
    * VTK prop assembly that aggregates all rendered ROI element actors and the last
    * rendered time point for caching purposes.
    *
    * \sa ROIMapper2D, ROIMapper3D
    */
  class ROIMapperLocalStorage : public Mapper::BaseLocalStorage
  {
  public:
    ROIMapperLocalStorage();
    ~ROIMapperLocalStorage() override;

    /**
     * \brief Get the VTK prop assembly containing all rendered ROI actors.
     *
     * \return Pointer to the prop assembly. Never \c nullptr after construction.
     */
    vtkPropAssembly* GetPropAssembly() const;

    /**
     * \brief Set the VTK prop assembly.
     *
     * \param[in] propAssembly The new prop assembly to store.
     */
    void SetPropAssembly(vtkPropAssembly* propAssembly);

    /**
     * \brief Get the time point that was last used for rendering.
     *
     * \return The last rendered time point.
     */
    TimePointType GetLastTimePoint() const;

    /**
     * \brief Set the last rendered time point.
     *
     * \param[in] timePoint The time point value to store.
     */
    void SetLastTimePoint(TimePointType timePoint);

  protected:
    vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
    TimePointType m_LastTimePoint;
  };
}

#endif
