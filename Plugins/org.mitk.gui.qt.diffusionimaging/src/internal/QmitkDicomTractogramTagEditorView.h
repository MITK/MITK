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

#ifndef QmitkDicomTractogramTagEditorView_h
#define QmitkDicomTractogramTagEditorView_h

#include <QmitkAbstractView.h>
#include "ui_QmitkDicomTractogramTagEditorViewControls.h"
#include <mitkFiberBundle.h>

/*!
\brief View for tensor based deterministic streamline fiber tracking.
*/
class QmitkDicomTractogramTagEditorView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkDicomTractogramTagEditorView();
  virtual ~QmitkDicomTractogramTagEditorView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

protected slots:

  void OnTractSelectionChanged();
  void CopyProperties();
  void OnItemChanged(QTableWidgetItem * item);

protected:

  void UpdateGui();

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkDicomTractogramTagEditorViewControls* m_Controls;
  std::vector< std::string > m_TagList;
  std::vector< std::string > m_ImageTagList;
  mitk::Image::Pointer m_Image;

protected slots:

private:


};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

