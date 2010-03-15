/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKCOLOURIMAGEPROCESSINGVIEW_H_INCLUDED
#define _QMITKCOLOURIMAGEPROCESSINGVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>
#include <mitkWeakPointer.h>

#include "mitkDataStorage.h"

#include <berryISelectionListener.h>
#include <mitkDataNodeSelection.h>

#include <mitkTransferFunctionProperty.h>

#include "ui_QmitkColourImageProcessingViewControls.h"

class QmitkColourImageProcessingView : public QObject, public QmitkFunctionality
{

  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkColourImageProcessingView();
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

