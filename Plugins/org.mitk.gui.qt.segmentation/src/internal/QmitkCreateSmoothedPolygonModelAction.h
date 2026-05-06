/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkCreateSmoothedPolygonModelAction_h
#define QmitkCreateSmoothedPolygonModelAction_h

#include "QmitkCreatePolygonModelAction.h"

/**
 * Smoothed variant of QmitkCreatePolygonModelAction. Identical pipeline; the only
 * difference is that vtkSurfaceNets3D smoothing is enabled. Existing as its own
 * class so the smoothing mode is selected by the action's identity in plugin.xml,
 * not by an out-of-band setter on the IContextMenuAction interface.
 */
class MITK_QT_SEGMENTATION QmitkCreateSmoothedPolygonModelAction : public QmitkCreatePolygonModelAction
{
  Q_OBJECT
  Q_INTERFACES(mitk::IContextMenuAction)

public:
  QmitkCreateSmoothedPolygonModelAction();
  ~QmitkCreateSmoothedPolygonModelAction() override;
};

#endif
