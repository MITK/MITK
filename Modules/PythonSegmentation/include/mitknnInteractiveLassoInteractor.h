/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveLassoInteractor_h
#define mitknnInteractiveLassoInteractor_h

#include <array>

#include <mitknnInteractiveInteractor.h>

namespace mitk
{
  class Image;
}

namespace mitk::nnInteractive
{
  /** \brief %nnInteractive interactor for drawing contours.
   *
   * Each completed contour is stored as a ContourModel DataNode (vector
   * geometry, cheap to keep around) for persistent visualization across
   * slice scrolling and interactor switches; contours are organized by
   * PromptType. The most recently drawn contour is also exposed as a
   * small bounding-box-sized 3D uint8 mask plus the corresponding
   * axis-aligned interaction bounding box in nnInteractive coordinates,
   * so the Python call can skip the full 3D volume round-trip.
   *
   * Interaction is handled through an internal contour state machine that
   * builds a 3D-world-coordinate ContourModel as the user drags. On
   * release the contour is rasterized via VTK reslicing into a small
   * bounding-box-sized mask -- no 3D working segmentation is involved.
   * Left mouse button display interaction (crosshair navigation) is
   * blocked while the interactor is enabled so the crosshair does not
   * move during drawing.
   *
   * \sa Interactor, ContourModel, nnInteractiveTool
   */
  class MITKPYTHONSEGMENTATION_EXPORT LassoInteractor : public Interactor
  {
  public:
    /** \brief Constructs a LassoInteractor with InteractionType::Lasso.
     */
    LassoInteractor();

    /** \brief Destructor. Calls OnReset() to clean up data nodes.
     */
    ~LassoInteractor() override;

    /** \brief Checks whether any contours have been drawn.
     *
     * \return \c true if at least one completed contour exists for any prompt
     *         type, \c false otherwise.
     */
    bool HasInteractions() const override;

    /** \brief Returns the binary mask slice of the most recently drawn contour.
     *
     * The mask is a uint8 image whose extent matches the corresponding
     * interaction bounding box (one voxel thick along the slicing axis).
     *
     * \return Pointer to the mask Image, or \c nullptr if no contour has been
     *         drawn yet.
     */
    const Image* GetLastLassoMask() const;

    /** \brief Returns the interaction bounding box for the most recently
     *         drawn contour.
     *
     * The box is in nnInteractive's coordinate order (reverse of MITK index
     * order): \c {{z_min,z_max}, {y_min,y_max}, {x_min,x_max}}.
     *
     * \return Pointer to the bounding box, or \c nullptr if no contour has
     *         been drawn yet.
     */
    const std::array<std::array<int, 2>, 3>* GetLastLassoBoundingBox() const;

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
