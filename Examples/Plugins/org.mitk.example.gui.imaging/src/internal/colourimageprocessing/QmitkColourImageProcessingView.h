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

#ifndef _QMITKCOLOURIMAGEPROCESSINGVIEW_H_INCLUDED
#define _QMITKCOLOURIMAGEPROCESSINGVIEW_H_INCLUDED

#include <QmitkAbstractView.h>

#include <mitkWeakPointer.h>

namespace Ui {
class QmitkColourImageProcessingViewControls;
}

class QmitkColourImageProcessingView : public QmitkAbstractView
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkColourImageProcessingView();

private:

  virtual void SetFocus() override;

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes) override;

private slots:

 void OnConvertToRGBAImage();
 void OnConvertImageMaskColorToRGBAImage();
 void OnCombineRGBA();
 void OnChangeColor();


private:

  Ui::QmitkColourImageProcessingViewControls* m_Controls;

  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;
  mitk::WeakPointer<mitk::DataNode> m_SelectedNode2;

  int m_Color[3];
};

#endif // _QMITKCOLOURIMAGEPROCESSINGVIEW_H_INCLUDED
