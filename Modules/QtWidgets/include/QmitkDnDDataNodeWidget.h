/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDnDDataNodeWidget_h
#define QmitkDnDDataNodeWidget_h

#include "MitkQtWidgetsExports.h"

// mitk core module
#include <mitkDataNode.h>

// qt
#include <QFrame>

class QDragEnterEvent;
class QDropEvent;

/**
* @brief A drag 'n' drop widget (QFrame) that checks the mime data of the incoming
*        event. If the drag event is coming from another widget and the mime data
*        is of type 'application/x-qmitk-datanode-ptrs' the node(s) will be dropped
*        and a signal is emitted.
*        A class including this DnD-widget can than handle the vector of dropped nodes
*        appropriately.
*/
class MITKQTWIDGETS_EXPORT QmitkDnDDataNodeWidget : public QFrame
{
  Q_OBJECT

public:

  QmitkDnDDataNodeWidget(QWidget* parent = nullptr);
  ~QmitkDnDDataNodeWidget() override;

Q_SIGNALS:

  /**
  * @brief The signal will be emitted if the drag events are accepted.
  *        It contains the vector of nodes that have been dropped.
  */
  void NodesDropped(std::vector<mitk::DataNode*> nodes);

private:

  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

};

#endif
