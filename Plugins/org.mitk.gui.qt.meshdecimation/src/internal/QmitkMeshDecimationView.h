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

#ifndef QmitkMeshDecimationView_h
#define QmitkMeshDecimationView_h

#include <berryISelectionListener.h>
#include <mitkWeakPointer.h>
#include <QmitkAbstractView.h>
#include <QMessageBox>
#include <QTimer>
#include "ui_QmitkMeshDecimationView.h"

/*!
  \brief QmitkMeshDecimationView 

  \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkMeshDecimationView : public QmitkAbstractView
{  
// this is needed for all Qt objects that should have a Qt meta-object
// (everything that derives from QObject and wants to have signal/slots)
Q_OBJECT
public:
  static const std::string VIEW_ID;

  QmitkMeshDecimationView();
  virtual ~QmitkMeshDecimationView();

  virtual void CreateQtPartControl(QWidget *parent);
  virtual void SetFocus();

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&);
protected slots:
  void Decimate();  
  void MessageBoxTimer();
protected:
  void DisableGui();
  void UpdateNodeLabel();
  void ShowAutoCloseErrorDialog( const QString& error );
  void ShowAutoCloseMessageDialog( const QString& message );

  Ui::QmitkMeshDecimationView* m_Controls;
  mitk::WeakPointer<mitk::DataNode> m_Node;
  QWidget* m_Parent;
  QMessageBox* m_MessageBox;
  int m_MessageBoxDisplayTime;
  QString m_MessageBoxText;
  QTimer *m_MessageBoxTimer;
};



#endif // _QMITKMESHSMOOTHINGVIEW_H_INCLUDED

