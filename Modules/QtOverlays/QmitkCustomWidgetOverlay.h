/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCustomWidgetOverlay_h
#define QmitkCustomWidgetOverlay_h

// MITK
#include "QmitkOverlay.h"

#include <MitkQtOverlaysExports.h>

/** \class  QmitkCustomWidgetOverlay
* \brief object representing a custom widget that is handled and positioned
* as an overlay.
*
* A QmitkCustomWidgetOverlay is a generic sub-class of QmitkOverlay. It
* offers the possibility to set the internal m_Widget from the outside.
*
* This offers the possibility to position custom widgets 'on top of' other
* widgets using the positioning mechanism of all overlays.
*
* \warning The custom widgets need to be configured and connected manually.
* Properties cannot be set.
*
* \ingroup Overlays
*/

class MITKQTOVERLAYS_EXPORT QmitkCustomWidgetOverlay : public QmitkOverlay
{
public:
  /**
  * @brief Default Constructor
  **/
  QmitkCustomWidgetOverlay(const char *id);

  /**
  * @brief Default Destructor
  **/
  ~QmitkCustomWidgetOverlay() override;

  void SetWidget(QWidget *widget);

  QSize GetNeededSize() override;
};

#endif
