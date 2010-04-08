/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QmitkPointSetInteraction_H__INCLUDED)
#define QmitkPointSetInteraction_H__INCLUDED

#include <berryISelectionListener.h>
#include <QmitkFunctionality.h>
#include <mitkWeakPointer.h>
#include <mitkDataNode.h>
#include <mitkInteractor.h>

namespace Ui
{
class QmitkPointSetInteractionControls;
};

/*!
\brief QmitkPointSetInteractionView 
*/
class QmitkPointSetInteractionView : public QObject, public QmitkFunctionality
{  
  Q_OBJECT
  
public:
  QmitkPointSetInteractionView(QObject *parent=0);
  virtual ~QmitkPointSetInteractionView();
  virtual void CreateQtPartControl(QWidget *parent);
  void OnSelectionChanged(std::vector<mitk::DataNode*> nodes);
  void Activated();
  void Deactivated();
  void NodeChanged(const mitk::DataNode* node);
  bool IsExclusiveFunctionality() const;
  virtual void StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget);
  virtual void StdMultiWidgetClosed(QmitkStdMultiWidget& stdMultiWidget);
protected slots:
  void OnAddPointSetClicked();
protected: 
  Ui::QmitkPointSetInteractionControls * m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_SelectedPointSetNode;
  mitk::WeakPointer<mitk::Interactor> m_SelectedPointSetInteractor;
};
#endif // !defined(QmitkPointSetInteraction_H__INCLUDED)
