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
  /**
   * \brief Data class that stores the 3D position for a crosshair.
   *
   * CrosshairData is a simple BaseData subclass that holds a single 3D point
   * representing the crosshair position. It is used by CrosshairManager
   * and rendered by CrosshairVtkMapper2D.
   *
   * This class does not support the requested region concept; all region
   * methods are implemented as no-ops.
   *
   * \sa CrosshairManager
   * \sa CrosshairVtkMapper2D
   */
  class MITKCORE_EXPORT CrosshairData : public BaseData
  {
  public:

    mitkClassMacro(CrosshairData, BaseData);
    itkFactorylessNewMacro(Self);

    itkGetConstMacro(Position, Point3D);
    itkSetMacro(Position, Point3D);

    /** \brief Empty implementation; CrosshairData does not support the requested region concept. */
    void SetRequestedRegionToLargestPossibleRegion() override;

    /**
     * \brief Empty implementation; CrosshairData does not support the requested region concept.
     *
     * \return Always false.
     */
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

    /**
     * \brief Empty implementation; CrosshairData does not support the requested region concept.
     *
     * \return Always true.
     */
    bool VerifyRequestedRegion() override;

    /** \brief Empty implementation; CrosshairData does not support the requested region concept. */
    void SetRequestedRegion(const itk::DataObject* data) override;

  protected:

    /** \brief Constructor. */
    CrosshairData();

    /** \brief Destructor. */
    ~CrosshairData() override;

    Point3D m_Position;

  };
}

#endif
