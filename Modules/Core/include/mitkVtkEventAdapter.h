/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkEventAdapter_h
#define mitkVtkEventAdapter_h

#include <mitkInteractionKeyEvent.h>
#include <mitkMouseMoveEvent.h>
#include <mitkMousePressEvent.h>
#include <mitkMouseReleaseEvent.h>
#include <mitkMouseWheelEvent.h>

#include <vtkRenderWindowInteractor.h>

namespace mitk
{
  class BaseRenderer;

  /**
   * \brief Generates MITK interaction events from VTK render window interactor events.
   *
   * This class is the non-Qt-dependent counterpart to QmitkEventAdapter.
   * It provides static functions to convert VTK events (mouse press, move,
   * release, wheel, key) into the corresponding MITK interaction event objects.
   *
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT VtkEventAdapter
  {
  public:
    /**
     * \brief Adapt a VTK mouse press event to a MITK MousePressEvent.
     *
     * \param[in] sender The BaseRenderer associated with the render window.
     * \param[in] vtkCommandEventId The VTK command event identifier.
     * \param[in] rwi The VTK render window interactor providing event data.
     * \return A new MousePressEvent instance.
     */
    static mitk::MousePressEvent::Pointer AdaptMousePressEvent(mitk::BaseRenderer *sender,
                                                               unsigned long vtkCommandEventId,
                                                               vtkRenderWindowInteractor *rwi);

    /**
     * \brief Adapt a VTK mouse move event to a MITK MouseMoveEvent.
     *
     * \param[in] sender The BaseRenderer associated with the render window.
     * \param[in] vtkCommandEventId The VTK command event identifier.
     * \param[in] rwi The VTK render window interactor providing event data.
     * \return A new MouseMoveEvent instance.
     */
    static mitk::MouseMoveEvent::Pointer AdaptMouseMoveEvent(mitk::BaseRenderer *sender,
                                                             unsigned long vtkCommandEventId,
                                                             vtkRenderWindowInteractor *rwi);

    /**
     * \brief Adapt a VTK mouse release event to a MITK MouseReleaseEvent.
     *
     * \param[in] sender The BaseRenderer associated with the render window.
     * \param[in] vtkCommandEventId The VTK command event identifier.
     * \param[in] rwi The VTK render window interactor providing event data.
     * \return A new MouseReleaseEvent instance.
     */
    static mitk::MouseReleaseEvent::Pointer AdaptMouseReleaseEvent(mitk::BaseRenderer *sender,
                                                                   unsigned long vtkCommandEventId,
                                                                   vtkRenderWindowInteractor *rwi);

    /**
     * \brief Adapt a VTK mouse wheel event to a MITK MouseWheelEvent.
     *
     * \param[in] sender The BaseRenderer associated with the render window.
     * \param[in] vtkCommandEventId The VTK command event identifier.
     * \param[in] rwi The VTK render window interactor providing event data.
     * \return A new MouseWheelEvent instance.
     */
    static mitk::MouseWheelEvent::Pointer AdaptMouseWheelEvent(mitk::BaseRenderer *sender,
                                                               unsigned long vtkCommandEventId,
                                                               vtkRenderWindowInteractor *rwi);

    /**
     * \brief Adapt a VTK key press event to a MITK InteractionKeyEvent.
     *
     * \param[in] sender The BaseRenderer associated with the render window.
     * \param[in] vtkCommandEventId The VTK command event identifier.
     * \param[in] rwi The VTK render window interactor providing event data.
     * \return A new InteractionKeyEvent instance, or nullptr if the event type is wrong.
     */
    static mitk::InteractionKeyEvent::Pointer AdaptInteractionKeyEvent(mitk::BaseRenderer *sender,
                                                                       unsigned long vtkCommandEventId,
                                                                       vtkRenderWindowInteractor *rwi);

    /**
     * \brief Map tracking mouse button state per renderer.
     *
     * VTK events do not provide button state for MouseMove events, so this
     * map records button state from press/release events and provides it
     * for subsequent move events.
     */
    static std::map<BaseRenderer *, int> buttonStateMap;
  };
}

#endif
