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


#ifndef QMITKXNATEDITOR_h
#define QMITKXNATEDITOR_h

#include <berryIReusableEditor.h>
#include <berryQtEditorPart.h>
#include <berryISelectionListener.h>
#include "berryIEditorInput.h"
#include "berryIEditorSite.h"

#include <QmitkAbstractView.h>

#include "ui_QmitkXnatEditorControls.h"

#include "ctkXnatListModel.h"
#include "ctkXnatSession.h"

/*!
\brief QmitkXnatEditor

\warning This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkXnatEditor : public berry::QtEditorPart, virtual public berry::IReusableEditor
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  QmitkXnatEditor();
  ~QmitkXnatEditor();

  static const std::string EDITOR_ID;
  static const QString DOWNLOAD_PATH;

  void CreateQtPartControl(QWidget *parent);

  void DoSave(/*IProgressMonitor monitor*/);
  void DoSaveAs();
  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);
  bool IsDirty() const;
  bool IsSaveAsAllowed() const;

  virtual void SetInput(berry::IEditorInput::Pointer input);
  //virtual void AddPropertyListener(berry::IPropertyChangeListener::Pointer listener);
  //virtual void CreatePartControl(void* parent);
  const char* GetClassNameA() const;

  void CheckUserInput();
  void UpdateList();

  protected slots:

    void OnObjectActivated(const QModelIndex& index);
    void OnDataModelButtonClicked();
    void OnProjectButtonClicked();
    void OnSubjectButtonClicked();
    void OnExperimentButtonClicked();
    void OnKindOfDataButtonClicked();
    void OnSessionButtonClicked();
    void OnResourceButtonClicked();

protected:

  virtual void SetFocus();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes );

  Ui::QmitkXnatEditorControls m_Controls;

private:

  int ParentChecker(ctkXnatObject* child);
  int m_ParentCount;
  void ToHigherLevel();

  ctkXnatListModel* m_ListModel;
  ctkXnatSession* m_Session;

  berry::ISelectionListener::Pointer m_SelectionListener;
  void SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection);
};

#endif // QMITKXNATEDITOR_h
