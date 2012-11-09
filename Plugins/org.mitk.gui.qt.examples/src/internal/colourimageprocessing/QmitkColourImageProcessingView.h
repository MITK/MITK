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

#include <QmitkFunctionality.h>
#include <org_mitk_gui_qt_examples_Export.h>
#include <string>
#include <mitkWeakPointer.h>

#include "mitkDataStorage.h"

#include <berryISelectionListener.h>
#include <mitkDataNodeSelection.h>

#include <mitkTransferFunctionProperty.h>

#include "ui_QmitkColourImageProcessingViewControls.h"

class QmitkColourImageProcessingView : public QmitkFunctionality
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkColourImageProcessingView();

   QmitkColourImageProcessingView(const QmitkColourImageProcessingView& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }

  virtual ~QmitkColourImageProcessingView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void SelectionChanged(berry::IWorkbenchPart::Pointer part, berry::ISelection::ConstPointer selection);

protected slots:

 void OnConvertToRGBAImage();
 void OnConvertImageMaskColorToRGBAImage();
 void OnCombineRGBA();
 void OnChangeColor();


protected:

  Ui::QmitkColourImageProcessingViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  berry::ISelectionListener::Pointer m_SelectionListener;



private:

  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;
  mitk::WeakPointer<mitk::DataNode> m_SelectedNode2;
  int m_Color[3];
};

#endif // _QMITKCOLOURIMAGEPROCESSINGVIEW_H_INCLUDED

