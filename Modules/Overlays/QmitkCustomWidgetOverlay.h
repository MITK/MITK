/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Thu, 28 May 2009) $
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef QmitkCustomWidgetOverlay_H_HEADER_INCLUDED_C10DC4EB
#define QmitkCustomWidgetOverlay_H_HEADER_INCLUDED_C10DC4EB

// MITK
#include "QmitkOverlay.h"

#include "OverlaysExports.h"


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


  class Overlays_EXPORT QmitkCustomWidgetOverlay : public QmitkOverlay
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

  };


#endif /* QmitkCustomWidgetOverlay_H_HEADER_INCLUDED_C10DC4EB */

