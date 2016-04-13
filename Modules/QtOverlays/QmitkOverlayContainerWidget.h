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


#ifndef QmitkOverlayContainerWidget_H_HEADER_INCLUDED_C10DC4EB
#define QmitkOverlayContainerWidget_H_HEADER_INCLUDED_C10DC4EB

// Qt
#include <QWidget>

#include <MitkQtOverlaysExports.h>


/**
* \class  QmitkOverlayContainerWidget
* \brief Widget that overrides the paintEvent method to correctly display
* the Qt based overlays when using the system-environment variable
* QT_DEVIDE_PIXEL_RATIO.
*/


class MITKQTOVERLAYS_EXPORT QmitkOverlayContainerWidget : public QWidget
{
public:

  /**
  * @brief Default Constructor
  **/
  QmitkOverlayContainerWidget( QWidget * parent = 0, Qt::WindowFlags f = 0 );

  /**
  * @brief Default Destructor
  **/
  virtual ~QmitkOverlayContainerWidget();


protected:

  /**
  * @brief overridden version of paintEvent that correctly clears its canvas before painting.
  **/
  virtual void paintEvent( QPaintEvent * event ) override;

};


#endif /* QmitkOverlayContainerWidget_H_HEADER_INCLUDED_C10DC4EB */
