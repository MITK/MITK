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
   * Scribbles are managed as labels of a common MultiLabelSegmentation,
   * organized by PromptType. Each prompt type is assigned its own label
   * within the segmentation node. The most recent brushstroke is available
   * as a 3D binary image mask for use as nnInteractive model input.
   *
   * Interaction is handled through an internal wrapper around the
   * DrawPaintbrushTool. Unlike the other interactors, ScribbleInteractor
   * does not block left mouse button display interaction, as the underlying
   * tool uses its own state machine for mouse event handling.
   *
   * \sa Interactor, DrawPaintbrushTool, MultiLabelSegmentation,
   *     nnInteractiveTool
   */
  class MITKPYTHONSEGMENTATION_EXPORT ScribbleInteractor : public Interactor
  {
  public:
    /** \brief Constructs a ScribbleInteractor with InteractionType::Scribble.
     */
    ScribbleInteractor();

    /** \brief Destructor. Calls OnReset() to clean up data nodes.
     */
    ~ScribbleInteractor() override;

    /** \brief Checks whether any scribbles have been drawn.
     *
     * \return \c true if any label in the scribble segmentation node contains
     *         non-empty pixel data, \c false otherwise.
     */
    bool HasInteractions() const override;

    /** \brief Returns the 3D binary mask of the most recent brushstroke.
     *
     * The mask is a full 3D image containing only the pixels drawn during
     * the most recent brushstroke, regardless of the current prompt type.
     *
     * \return Pointer to the mask Image, or \c nullptr if no brushstroke has
     *         been drawn yet.
     */
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
