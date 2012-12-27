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

  \sa QmitkAbstractView
  \ingroup org_mitk_gui_qt_meshdecimation_internal
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

