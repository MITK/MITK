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

#ifndef QmlMitkFourRenderWindowWidget_h
#define QmlMitkFourRenderWindowWidget_h

#include <QQuickItem>

#include "QmlMitkRenderWindowItem.h"

#include "mitkDataStorage.h"

#include "MitkQmlItemsExports.h"

/**
  \brief QML replacement for QmitkStdMultiWidget.

  A proof-of-concept "multi-widget". Currently exclusively
  for use in the QuickRender demo application.

  \warning Subject to change.
*/
class MITKQMLITEMS_EXPORT QmlMitkFourRenderWindowWidget : public QQuickItem
{
    Q_OBJECT

public:

  QmlMitkFourRenderWindowWidget(QQuickItem* parent = 0);
  virtual ~QmlMitkFourRenderWindowWidget();

  void SetDataStorage( mitk::DataStorage::Pointer storage );

signals:

public slots:

protected slots:

protected:

  virtual void SetupWidget( QQuickItem* parent );

  virtual void InitializeMoveZoomInteraction();

  mitk::DataStorage::Pointer m_DataStorage;

private slots:

private:

  QQuickItem* m_ChildrenContainer;

  QmlMitkRenderWindowItem* m_RenderItemAxial;
  QmlMitkRenderWindowItem* m_RenderItemSagittal;
  QmlMitkRenderWindowItem* m_RenderItemFrontal;
  QmlMitkRenderWindowItem* m_RenderItem3D;

};

#endif
