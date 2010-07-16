/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-06-15 17:15:21 +0200 (Di, 15 Jun 2010) $
Version:   $Revision: 23776 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKOVERLAYCONTROLLER_H_HEADER_INCLUDED_C1E77191
#define MITKOVERLAYCONTROLLER_H_HEADER_INCLUDED_C1E77191

// MITK-Stuff
#include "mitkCommon.h"
#include "mitkPropertyList.h"
#include "QmitkOverlay.h"

#include <QObject>

#include <string>


class QmitkRenderWindow;

class QMITK_EXPORT QmitkOverlayController : public QObject
{
  Q_OBJECT 

public:

  QmitkOverlayController( QmitkRenderWindow* rw, mitk::PropertyList* pl = NULL );
  virtual ~QmitkOverlayController();

  void AddOverlay( QmitkOverlay* );

  void AdjustOverlayPosition();

  void SetOverlayVisibility( bool visible );

protected:

  void InitializeOverlayLayout();

  virtual void AlignOverlays();
 
  void InitializeWidget( QmitkOverlay::DisplayPosition pos );

  typedef std::map< QmitkOverlay::DisplayPosition, QWidget* > OverlayPositionMap;
  typedef std::vector< QmitkOverlay* > OverlayVector;

  OverlayVector                 m_AllOverlays;

  OverlayPositionMap            m_PositionedOverlays;

  QmitkRenderWindow*            m_RenderWindow;

  mitk::PropertyList::Pointer   m_PropertyList;

};



#endif /* MITKOVERLAYCONTROLLER_H_HEADER_INCLUDED_C1E77191 */
