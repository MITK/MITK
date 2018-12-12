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

#ifndef QMITK_NODE_SELECTION_BUTTON_H
#define QMITK_NODE_SELECTION_BUTTON_H

#include "org_mitk_gui_qt_common_Export.h"

// mitk core
#include <mitkDataNode.h>

// qt
#include <QPushButton>
#include <QPixmap>

/**
* @class QmitkNodeSelectionButton
* @brief Button class that can be used to display informations about a given node.
*        If the given node is a nullptr the node info text will be shown.
*        The node info can be formated text (e.g. HTML code; like the tooltip text).
*/
class MITK_QT_COMMON QmitkNodeSelectionButton : public QPushButton
{
  Q_OBJECT

public:
  explicit QmitkNodeSelectionButton(QWidget *parent = nullptr);
  ~QmitkNodeSelectionButton();

  mitk::DataNode::Pointer GetSelectedNode() const;

public Q_SLOTS:
  virtual void SetSelectedNode(mitk::DataNode* node);
  virtual void SetNodeInfo(QString info);

protected:
  void paintEvent(QPaintEvent *p) override;

  mitk::DataNode::Pointer m_SelectedNode;
  QString m_Info;
  bool m_OutdatedThumbnail;
  QPixmap m_Thumbnail;
};

#endif // QMITK_NODE_SELECTION_BUTTON_H
