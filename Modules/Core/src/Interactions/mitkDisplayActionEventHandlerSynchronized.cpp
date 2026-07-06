/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDisplayActionEventHandlerSynchronized.h>

// itk
#include <itkEventObject.h>

void mitk::DisplayActionEventHandlerSynchronized::SetPredicates(const Predicates& predicates)
{
  m_Predicates = predicates;
}

void mitk::DisplayActionEventHandlerSynchronized::InitActionsImpl(const std::string& prefixFilter /* = "" */)
{
  using namespace DisplayActionEventFunctions;

  // A null predicate marks the dimension as not synchronized; the sender-only
  // action keeps the local gesture working without propagation (see Predicates).
  ConnectDisplayActionEvent(DisplayMoveEvent(nullptr, Vector2D()),
    m_Predicates.pan ? MoveCameraSynchronizedAction(m_Predicates.pan)
                     : MoveSenderCameraAction(prefixFilter));

  ConnectDisplayActionEvent(DisplaySetCrosshairEvent(nullptr, Point3D()),
    m_Predicates.crosshair ? SetCrosshairSynchronizedAction(m_Predicates.crosshair)
                           : SetCrosshairAction(prefixFilter));

  ConnectDisplayActionEvent(DisplayZoomEvent(nullptr, 0.0, Point2D()),
    m_Predicates.zoom ? ZoomCameraSynchronizedAction(m_Predicates.zoom)
                      : ZoomSenderCameraAction(prefixFilter));

  ConnectDisplayActionEvent(DisplayScrollEvent(nullptr, 0, true),
    m_Predicates.slice ? ScrollSliceStepperSynchronizedAction(m_Predicates.slice)
                       : ScrollSliceStepperAction(prefixFilter));

  ConnectDisplayActionEvent(DisplaySetLevelWindowEvent(nullptr, ScalarType(), ScalarType()),
    m_Predicates.levelWindow ? SetLevelWindowSynchronizedAction(m_Predicates.levelWindow)
                             : SetLevelWindowAction(prefixFilter));
}
