/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveBoxInteractor_h
#define mitknnInteractiveBoxInteractor_h

#include <mitknnInteractiveInteractor.h>

namespace mitk
{
  class PlanarFigure;
}

namespace mitk::nnInteractive
{
  /** \brief %nnInteractive interactor for drawing rectangular boxes.
   *
   * Boxes are managed as a list of PlanarRectangle data nodes, organized by
   * PromptType. The currently drawn PlanarRectangle is stored in a separate
   * data node until it is completed.
   *
   * Interaction is handled through the PlanarFigureInteractor.
   */
  class MITKSEGMENTATION_EXPORT BoxInteractor : public Interactor
  {
  public:
    BoxInteractor();
    ~BoxInteractor() override;

    bool HasInteractions() const override;

    const PlanarFigure* GetLastBox() const;

  private:
    void OnEnable() override;
    void OnDisable() override;
    void OnReset() override;

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
