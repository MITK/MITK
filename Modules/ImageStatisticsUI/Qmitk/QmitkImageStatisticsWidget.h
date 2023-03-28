/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkImageStatisticsWidget_h
#define QmitkImageStatisticsWidget_h

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

  /*! /brief Set flag to ignore zero valued voxels */
  void SetIgnoreZeroValueVoxel(bool _arg);
  /*! /brief Get status of zero value voxel ignoring. */
  bool GetIgnoreZeroValueVoxel() const;

  /*! /brief Set bin size for histogram resolution.*/
  void SetHistogramNBins(unsigned int nbins);
  /*! /brief Get bin size for histogram resolution.*/
  unsigned int GetHistogramNBins() const;

signals:
  void IgnoreZeroValuedVoxelStateChanged(int status);

private:
  void CreateConnections();
  void OnDataAvailable();

  /** \brief  Saves the image statistics to the clipboard */
  void OnClipboardButtonClicked();

  Ui::QmitkImageStatisticsControls m_Controls;
  QmitkImageStatisticsTreeModel *m_imageStatisticsModel;
  QSortFilterProxyModel *m_ProxyModel;
};
#endif
