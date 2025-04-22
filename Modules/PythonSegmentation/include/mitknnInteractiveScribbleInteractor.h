/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveScribbleInteractor_h
#define mitknnInteractiveScribbleInteractor_h

#include <mitknnInteractiveInteractor.h>

namespace mitk
{
  class Image;
}

namespace mitk::nnInteractive
{
  /** \brief %nnInteractive interactor for freehand brushstrokes.
   *
   * Scribbles are managed as labels of a common MultiLabelSegmentation, organized
   * by PromptType.
   *
   * Interaction is handled through the DrawPaintbrushTool.
   */
  class MITKPYTHONSEGMENTATION_EXPORT ScribbleInteractor : public Interactor
  {
  public:
    ScribbleInteractor();
    ~ScribbleInteractor() override;

    bool HasInteractions() const override;

    const Image* GetLastScribbleMask() const;

  private:
    void OnSetToolManager() override;
    void OnHandleEvent(InteractionEvent* event) override;
    void OnEnable() override;
    void OnDisable() override;
    void OnReset() override;

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
