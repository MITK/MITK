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


#ifndef XNATProjects_h
#define XNATProjects_h

#include <berryQtEditorPart.h>
#include <berryISelectionListener.h>
#include "berryIEditorInput.h"
#include "berryIEditorSite.h"

#include <QmitkAbstractView.h>

#include "ui_XNATProjectsControls.h"

#include "Poco/DOM/Node.h"
#include "Poco/URI.h"

#include <QScopedPointer>
#include <QStandardItemModel>
#include <QMap>

#include "ctkXnatListModel.h"
#include "ctkXnatConnectionFactory.h"
#include "ctkXnatConnection.h"
#include "ctkXnatScanResource.h"

/*!
\brief XNATProjects

\warning This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class XNATProjects : public berry::QtEditorPart
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  XNATProjects();
  ~XNATProjects();
  static const std::string EDITOR_ID;
  static const QString DOWNLOAD_PATH;
  void CreateQtPartControl(QWidget *parent);
  void SearchWithXmlFilePath(QString filepath);

  void DoSave(/*IProgressMonitor monitor*/);
  void DoSaveAs();
  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);
  bool IsDirty() const;
  bool IsSaveAsAllowed() const;

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void GetAllProjects();
    void DownloadResource();
    void DownloadFile();

    void ProjectSelected(const QModelIndex& index);
    void SubjectSelected(const QModelIndex& index);
    void ExperimentSelected(const QModelIndex& index);
    void ScanSelected(const QModelIndex& index);
    void ResourceSelected(const QModelIndex& index);

protected:

  virtual void SetFocus();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes );

  Ui::XNATProjectsControls m_Controls;


private:
  //QMap::QMap<QString, ctkXnatLoginProfile*> m_Profiles;
  QString ConvertFromXMLString(const Poco::XML::XMLString);
  bool DownloadExists(QString filename);

  ctkXnatConnectionFactory* m_ConnectionFactory;
  ctkXnatConnection* m_Connection;
  ctkXnatListModel* m_ProjectsModel;
  ctkXnatListModel* m_SubjectsModel;
  ctkXnatListModel* m_ExperimentsModel;
  ctkXnatListModel* m_ScansModel;
  ctkXnatListModel* m_ResourceModel;
  ctkXnatListModel* m_FileModel;
};

#endif // XNATProjects_h
