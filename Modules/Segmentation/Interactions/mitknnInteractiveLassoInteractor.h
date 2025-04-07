/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveLassoInteractor_h
#define mitknnInteractiveLassoInteractor_h

#include <mitknnInteractiveInteractor.h>

namespace mitk::nnInteractive
{
  /** \brief %nnInteractive interactor for drawing contours.
   *
   * Contours are managed as a list of ContourModel data nodes, organized
   * by PromptType. The most recently drawn contour is also available as a
   * binary image mask.
   *
   * Interaction is handled through the AddContourTool.
   */
  class MITKSEGMENTATION_EXPORT LassoInteractor : public Interactor
  {
  public:
    LassoInteractor();
    ~LassoInteractor() override;

    bool HasInteractions() const override;

    const Image* GetLastLassoMask() const;

  private:
    void OnSetToolManager() override;
    void OnHandleEvent(InteractionEvent* event) override;
    void OnEnable() override;
    void OnDisable() override;
    void OnReset() override;

    struct Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
