/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef QmitkCustomWidgetOverlay_H_HEADER_INCLUDED_C10DC4EB
#define QmitkCustomWidgetOverlay_H_HEADER_INCLUDED_C10DC4EB

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
* \warn The custom widgets need to be configured and connected manually.
* Properties cannot be set.
*
* \ingroup Overlays
*/


  class MitkQtOverlays_EXPORT QmitkCustomWidgetOverlay : public QmitkOverlay
  {
  public:

    /**
    * @brief Default Constructor
    **/
    QmitkCustomWidgetOverlay( const char* id );

    /**
    * @brief Default Destructor
    **/
    virtual ~QmitkCustomWidgetOverlay();

    void SetWidget( QWidget* widget );

    QSize GetNeededSize();

  };


#endif /* QmitkCustomWidgetOverlay_H_HEADER_INCLUDED_C10DC4EB */

