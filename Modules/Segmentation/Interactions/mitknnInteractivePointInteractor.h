/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractivePointInteractor_h
#define mitknnInteractivePointInteractor_h

#include <mitknnInteractiveInteractor.h>
#include <optional>

namespace mitk::nnInteractive
{
  /** \brief %nnInteractive interactor for placing individual points.
   *
   * Points are managed in up to two PointSet data nodes, separated by
   * prompt type.
   *
   * Interaction is handled through the PointSetDataInteractor.
   */
  class MITKSEGMENTATION_EXPORT PointInteractor : public Interactor
  {
  public:
    PointInteractor();
    ~PointInteractor() override;

    bool HasInteractions() const override;

    std::optional<Point3D> GetLastPoint() const;

  private:
    void OnEnable() override;
    void OnDisable() override;
    void OnReset() override;

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
