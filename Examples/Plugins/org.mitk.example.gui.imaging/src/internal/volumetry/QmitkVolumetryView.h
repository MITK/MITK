/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _QMITKVOLUMETRYVIEW_H_INCLUDED
#define _QMITKVOLUMETRYVIEW_H_INCLUDED

#include <QmitkAbstractView.h>

#include "mitkWeakPointer.h"

namespace Ui
{
  class QmitkVolumetryViewControls;
}

/**
 * \brief QmitkVolumetryView
 *
 * \sa QmitkAbstractView
 */
class QmitkVolumetryView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkVolumetryView();

private:
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  mitk::Image *GetImage() const;

  mitk::DataNode *GetOverlayNode() const;

  void CreateOverlayChild();

  void UpdateSliderLabel();

  void UpdateSlider();

  const mitk::DataNode *GetImageNode() const;

private slots:

  void OnCalculateVolume();

  void OnTimeSeriesButtonClicked();

  void OnThresholdSliderChanged(int value);

  void OnSaveCsvButtonClicked();

private:
  Ui::QmitkVolumetryViewControls *m_Controls;

  /// store weak pointer of the DataNode
  mitk::WeakPointer<mitk::DataNode> m_SelectedDataNode;

  mitk::DataNode::Pointer m_OverlayNode;

  mitk::DataStorage::Pointer m_DataStorage;

  QWidget *m_ParentWidget;
};

#endif // _QMITKVOLUMETRYVIEW_H_INCLUDED
