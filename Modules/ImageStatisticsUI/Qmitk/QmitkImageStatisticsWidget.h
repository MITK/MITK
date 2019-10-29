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
#ifndef QmitkImageStatisticsWidget_H__INCLUDED
#define QmitkImageStatisticsWidget_H__INCLUDED

#include <MitkImageStatisticsUIExports.h>
#include <ui_QmitkImageStatisticsWidget.h>

#include <mitkDataStorage.h>
#include <mitkDataNode.h>

class QSortFilterProxyModel;
class QmitkImageStatisticsTreeModel;

class MITKIMAGESTATISTICSUI_EXPORT QmitkImageStatisticsWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkImageStatisticsWidget(QWidget *parent = nullptr);

  /**Documentation
  Set the data storage the model should fetch its statistic objects from.
  @pre data storage must be valid
  */
  void SetDataStorage(mitk::DataStorage *newDataStorage);

  void SetImageNodes(const std::vector<mitk::DataNode::ConstPointer> &nodes);
  void SetMaskNodes(const std::vector<mitk::DataNode::ConstPointer> &nodes);
  void Reset();

private:
  void CreateConnections();
  void OnDataAvailable();

  /** \brief  Saves the image statistics to the clipboard */
  void OnClipboardButtonClicked();

private:
  Ui::QmitkImageStatisticsControls m_Controls;
  QmitkImageStatisticsTreeModel *m_imageStatisticsModel;
  QSortFilterProxyModel *m_ProxyModel;
};
#endif // QmitkImageStatisticsWidget_H__INCLUDED
