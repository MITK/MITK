/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveScribbleInteractor_h
#define mitknnInteractiveScribbleInteractor_h

#include <array>

#include <mitknnInteractiveInteractor.h>

namespace mitk
{
  class Image;
}

namespace mitk::nnInteractive
{
  /** \brief %nnInteractive interactor for freehand brushstrokes.
   *
   * Each completed brushstroke is stored as a small 2D uint8 mitk::Image
   * positioned at the stroke's slicing plane and wrapped in a DataNode that
   * gets added to the data storage for persistent cross-slice
   * visualization. Strokes are organized by PromptType (each completed node
   * keeps the color of the prompt type active when it was drawn). The most
   * recent brushstroke is also exposed as a pointer to the 2D mask plus an
   * axis-aligned interaction bounding box in nnInteractive coordinates so
   * the Python call can skip the full 3D volume round-trip.
   *
   * Interaction is handled through an internal brush state machine that
   * paints directly into a 2D uint8 Image -- no 3D working segmentation is
   * involved. Left mouse button display interaction (crosshair navigation)
   * is blocked while the interactor is enabled so that brush strokes do
   * not move the crosshair.
   *
   * \sa Interactor, nnInteractiveTool
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

    /** \brief Returns the binary mask slice of the most recent brushstroke.
     *
     * The mask is a uint8 image whose extent matches the corresponding
     * interaction bounding box (one voxel thick along the slicing axis).
     * Intended to be passed alongside GetLastScribbleBoundingBox() to
     * session.add_scribble_interaction via nnInteractiveTool.
     *
     * \return Pointer to the mask Image, or \c nullptr if no brushstroke has
     *         been drawn yet.
     */
    const Image* GetLastScribbleMask() const;

    /** \brief Returns the interaction bounding box for the most recent
     *         brushstroke.
     *
     * The box is in nnInteractive's coordinate order (reverse of MITK index
     * order): \c {{z_min,z_max}, {y_min,y_max}, {x_min,x_max}}.
     *
     * \return Pointer to the bounding box, or \c nullptr if no brushstroke
     *         has been drawn yet.
     */
    const std::array<std::array<int, 2>, 3>* GetLastScribbleBoundingBox() const;

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
