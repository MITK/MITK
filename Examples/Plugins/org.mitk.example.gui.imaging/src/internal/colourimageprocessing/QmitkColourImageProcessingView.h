/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkColourImageProcessingView_h
#define QmitkColourImageProcessingView_h

#include <QmitkAbstractView.h>

#include <mitkWeakPointer.h>

namespace Ui
{
  class QmitkColourImageProcessingViewControls;
}

class QmitkColourImageProcessingView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkColourImageProcessingView();

private:
  void SetFocus() override;

  void CreateQtPartControl(QWidget *parent) override;

  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

private slots:

  void OnConvertToRGBAImage();
  void OnConvertImageMaskColorToRGBAImage();
  void OnCombineRGBA();
  void OnChangeColor();

private:
  Ui::QmitkColourImageProcessingViewControls *m_Controls;

  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;
  mitk::WeakPointer<mitk::DataNode> m_SelectedNode2;

  int m_Color[3];
};

#endif
