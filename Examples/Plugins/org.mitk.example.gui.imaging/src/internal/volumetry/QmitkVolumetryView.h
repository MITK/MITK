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

#ifndef _QMITKVOLUMETRYVIEW_H_INCLUDED
#define _QMITKVOLUMETRYVIEW_H_INCLUDED

#include <QmitkAbstractView.h>

#include "mitkWeakPointer.h"

namespace Ui {
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

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes) override;

  mitk::Image* GetImage() const;

  mitk::DataNode* GetOverlayNode() const;

  void CreateOverlayChild();

  void UpdateSliderLabel();

  void UpdateSlider();

  const mitk::DataNode* GetImageNode() const;

private slots:

  void OnCalculateVolume();

  void OnTimeSeriesButtonClicked();

  void OnThresholdSliderChanged( int value );

  void OnSaveCsvButtonClicked();

private:

  Ui::QmitkVolumetryViewControls* m_Controls;

  /// store weak pointer of the DataNode
  mitk::WeakPointer<mitk::DataNode> m_SelectedDataNode;

  mitk::DataNode::Pointer m_OverlayNode;

  mitk::DataStorage::Pointer m_DataStorage;

  QWidget* m_ParentWidget;
};




#endif // _QMITKVOLUMETRYVIEW_H_INCLUDED

