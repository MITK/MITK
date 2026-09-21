/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrosshairRotationMode_h
#define QmitkCrosshairRotationMode_h

#include <QMetaType>

/**
 * \brief How the left mouse button manipulates the crosshair planes.
 *
 * The modes are offered by the crosshair menu of a render window and are
 * realized by the MITK interaction schemes. They have no counterpart in the
 * PACS schemes, so selecting one of them leaves PACS mode.
 */
enum class QmitkCrosshairRotationMode
{
  None = 0,
  Single,
  Coupled,
  Swivel
};

Q_DECLARE_METATYPE(QmitkCrosshairRotationMode)

#endif
