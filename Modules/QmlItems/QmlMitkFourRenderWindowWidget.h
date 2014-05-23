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
class MitkQmlItems_EXPORT QmlMitkFourRenderWindowWidget : public QQuickItem
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
