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


#ifndef QmitkDicomEditor_h
#define QmitkDicomEditor_h

#include <berryISelectionListener.h>

#include <berryQtEditorPart.h>
#include <berryIPartListener.h>
#include <QTextEdit>

#include <mitkIDataStorageReference.h>
#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>

#include "ui_QmitkDicomEditorControls.h"
#include <ctkFileDialog.h>

/*!
  \brief QmitkDicomEditor

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
*/
class QmitkDicomEditor : public berry::QtEditorPart, virtual public berry::IPartListener
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    berryObjectMacro(QmitkDicomEditor);
    static const std::string EDITOR_ID;

    QmitkDicomEditor();
    QmitkDicomEditor(const QmitkDicomEditor& other)
    {
        Q_UNUSED(other)
            throw std::runtime_error("Copy constructor not implemented");
    }
    virtual ~QmitkDicomEditor();

    void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);

    void SetFocus();
    void DoSave() {}
    void DoSaveAs() {}
    bool IsDirty() const { return false; }
    bool IsSaveAsAllowed() const { return false; }

  protected slots:

    void OnImportDirectory(QString directory);
      
    /// \brief Called when series in TreeView is double clicked.
    void OnSeriesModelSelected(QModelIndex index);

    /// \brief Called when Import CD or Import Folder was clicked.
    void OnFolderCDImport();

    /// \brief Called when Query Retrieve or Import Folder was clicked.
    void OnQueryRetrieve();

    /// \brief Called when LocalStorageButton was clicked.
    void OnLocalStorage();

  protected:

    ctkFileDialog* m_ImportDialog;

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    void CreateQtPartControl(QWidget *parent);

    Events::Types GetPartEventTypes() const;

    Ui::QmitkDicomEditorControls m_Controls;
};

#endif // QmitkDicomEditor_h

