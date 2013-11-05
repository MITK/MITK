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


// Qmitk
#include "XNATProjects.h"

// Standard
#include <iostream>
#include <string>
#include <regex>

// CTK Widgets
//#include "ctkXnatLoginDialog.h"

// CTK Core
#include "ctkXnatServer.h"
#include "ctkXnatConnectionFactory.h"
#include "ctkXnatConnection.h"
#include "ctkXnatProject.h"
#include "ctkXnatSubject.h"
#include "ctkXnatListModel.h"
#include "ctkXnatScanFolder.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qt
#include <QScopedPointer>
#include <QSharedPointer>
#include <QMessageBox>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QBrush>
#include <QListView>
#include <QRegExp>
#include <QModelIndex>

// Poco
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPBasicCredentials.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/SAX/InputSource.h"
#include "Poco/URI.h"
#include "Poco/NumberParser.h"
#include "Poco/SAX/SAXParser.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/NodeIterator.h"
#include "Poco/DOM/NodeFilter.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/DOM/Node.h"
#include "Poco/StreamCopier.h"

const std::string XNATProjects::EDITOR_ID = "org.mitk.editors.xnatprojects";

XNATProjects::XNATProjects() :
  m_ConnectionFactory(new ctkXnatConnectionFactory()),
  m_Connection(0),
  m_ProjectsModel(new ctkXnatListModel()),
  m_SubjectsModel(new ctkXnatListModel()),
  m_ExperimentsModel(new ctkXnatListModel()),
  m_ScansModel(new ctkXnatListModel()),
  m_ResourceModel(new ctkXnatListModel())
{

}

XNATProjects::~XNATProjects()
{
  if ( m_Connection ) delete m_Connection;
  delete m_ConnectionFactory;
  delete m_SubjectsModel;
  delete m_ProjectsModel;
  delete m_ExperimentsModel;
  delete m_ScansModel;
  delete m_ResourceModel;
}

bool XNATProjects::IsDirty() const
{
  return false;
}

bool XNATProjects::IsSaveAsAllowed() const
{
  return false;
}

void XNATProjects::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
   this->SetSite(site);
   this->SetInput(input);
}

void XNATProjects::DoSave()
{
}

void XNATProjects::DoSaveAs()
{
}

void XNATProjects::SetFocus()
{
  m_Controls.buttonGetAllProjects->setFocus();
  m_Controls.buttonLoginDialog->setFocus();
}

void XNATProjects::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  m_Controls.projectTreeView->setModel(m_ProjectsModel);
  m_Controls.subjectTreeView->setModel(m_SubjectsModel);
  m_Controls.experimentTreeView->setModel(m_ExperimentsModel);
  m_Controls.scanTreeView->setModel(m_ScansModel);
  m_Controls.resourceTreeView->setModel(m_ResourceModel);

  connect( m_Controls.buttonGetAllProjects, SIGNAL(clicked()), this, SLOT(GetAllProjects()) );
  connect( m_Controls.projectTreeView, SIGNAL(clicked(QModelIndex)), SLOT(projectSelected(QModelIndex)) );
  connect( m_Controls.subjectTreeView, SIGNAL(clicked(QModelIndex)), SLOT(subjectSelected(QModelIndex)) );
  connect( m_Controls.experimentTreeView, SIGNAL(clicked(QModelIndex)), SLOT(experimentSelected(QModelIndex)) );
  connect( m_Controls.scanTreeView, SIGNAL(clicked(QModelIndex)), SLOT(scanSelected(QModelIndex)) );

  connect( m_Controls.buttonLoginDialog, SIGNAL(clicked()), this, SLOT(OpenLoginDialog()) );
}

void XNATProjects::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  //foreach( mitk::DataNode::Pointer node, nodes )
  //{
  //  if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
  //  {
  //    m_Controls.labelWarning->setVisible( false );
  //    m_Controls.buttonGetAllProjects->setEnabled( true );
  //    return;
  //  }
  //}
  //m_Controls.labelWarning->setVisible( true );
  //m_Controls.buttonGetAllProjects->setEnabled( false );
}

/**
\brief To get all Projects
*/
void XNATProjects::GetAllProjects()
{
  // Clear existing treeView
  m_SubjectsModel->setRootObject(ctkXnatObject::Pointer());
  m_ExperimentsModel->setRootObject(ctkXnatObject::Pointer());
  m_ScansModel->setRootObject(ctkXnatObject::Pointer());
  m_ResourceModel->setRootObject(ctkXnatObject::Pointer());
  m_Controls.subjectTreeView->reset();
  m_Controls.experimentTreeView->reset();
  m_Controls.scanTreeView->reset();
  m_Controls.resourceTreeView->reset();

  // Validate user input
  if ( m_Controls.inHostAddress->text().isEmpty() || m_Controls.inUser->text().isEmpty() || m_Controls.inPassword->text().isEmpty() ) {
    if ( m_Controls.inHostAddress->text().isEmpty() )
    {
      MITK_INFO << "No host address!";
      m_Controls.inHostAddress->setStyleSheet("QLineEdit{ background-color: rgb(255,0,0) }");
    }

    if ( m_Controls.inUser->text().isEmpty() ) {
      MITK_INFO << "No user !";
      m_Controls.inUser->setStyleSheet("QLineEdit{ background-color: rgb(255,0,0) }");
    }

    if ( m_Controls.inPassword->text().isEmpty() ) {
      MITK_INFO << "No password!";
      m_Controls.inPassword->setStyleSheet("QLineEdit{ background-color: rgb(255,0,0) }");
    }
    return;
  }
  else
  {
    m_Controls.inHostAddress->setStyleSheet("QLineEdit{ background-color: rgb(255,255,255) }");
    m_Controls.inUser->setStyleSheet("QLineEdit{ background-color: rgb(255,255,255) }");
    m_Controls.inPassword->setStyleSheet("QLineEdit{ background-color: rgb(255,255,255) }");
  }

  // Regular Expression for uri
  QRegExp uriregex("^(https?)://([a-zA-Z0-9\\.]+):([0-9]+)(/[^ /]+)*$");

  // Validate address
  if ( ! uriregex.exactMatch(m_Controls.inHostAddress->text()) )
  {
    MITK_INFO << m_Controls.inHostAddress->text().toStdString();
    MITK_INFO << "Host address not valid";
    m_Controls.inHostAddress->setStyleSheet("QLineEdit{ background-color: rgb(255,0,0) }");
    return;
  }
  else
  {
    m_Controls.inHostAddress->setStyleSheet("QLineEdit{ background-color: rgb(255,255,255) }");
  }

  // create ctkXnatConnection
  m_Connection = m_ConnectionFactory->makeConnection(m_Controls.inHostAddress->text(), m_Controls.inUser->text(), m_Controls.inPassword->text());
  ctkXnatServer::Pointer server(ctkXnatServer::Create(m_Connection));

  // fetch projects from server
  server->fetch();

  // list projects
  m_ProjectsModel->setRootObject(server);

  // Set tree visible
  m_Controls.projectTreeView->reset();
}

/**
\brief ******** SEARCH ********
*/
void XNATProjects::SearchWithXmlFilePath(QString filepath)
{
  /*
  bla.. blaa... blaaa....
  */
}

/**
\brief Converts a Poco::XML::XMLString to QString
*/
QString XNATProjects::ConvertFromXMLString(const Poco::XML::XMLString xmlstr)
{
  const char* cstr = (Poco::XML::fromXMLString(xmlstr).c_str());
  QString qstr = cstr;
  return qstr;
}

/**
\brief *****DOES NOT THAT WHAT IT IS THERE FOR*****
*/
void XNATProjects::OpenLoginDialog()
{
}

/**
\brief *****fetchs subjects from chosen project*****
*/
void XNATProjects::projectSelected(const QModelIndex& index)
{
  QVariant variant = m_ProjectsModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    ctkXnatObject::Pointer project = variant.value<ctkXnatObject::Pointer>();
    project->fetch();
    m_SubjectsModel->setRootObject(project);
    m_Controls.subjectTreeView->reset();
  }
}

/**
\brief *****fetchs experiments from chosen subject*****
*/
void XNATProjects::subjectSelected(const QModelIndex& index)
{
  QVariant variant = m_SubjectsModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    ctkXnatObject::Pointer subject = variant.value<ctkXnatObject::Pointer>();
    subject->fetch();
    m_ExperimentsModel->setRootObject(subject);
    m_Controls.experimentTreeView->reset();
  }
}

/**
\brief *****fetchs data from chosen experiment*****
*/
void XNATProjects::experimentSelected(const QModelIndex& index)
{
  QVariant variant = m_ExperimentsModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    ctkXnatObject::Pointer experiment = variant.value<ctkXnatObject::Pointer>();
    experiment->fetch();
    ctkXnatObject::Pointer scanfolder =  experiment->children().takeFirst();
    scanfolder->fetch();
    m_ScansModel->setRootObject(scanfolder);
    m_Controls.scanTreeView->reset();
    MITK_INFO << scanfolder->children().takeFirst()->uri().toStdString();
  }
}

/**
\brief *****fetchs download data from chosen experiment*****
*/
void XNATProjects::scanSelected(const QModelIndex& index)
{
  QVariant variant = m_ScansModel->data(index, Qt::UserRole);
  if (variant.isValid())
  {
    ctkXnatObject::Pointer scan = variant.value<ctkXnatObject::Pointer>();
    scan->fetch();
    m_ResourceModel->setRootObject(scan);
    m_Controls.resourceTreeView->reset();
    MITK_INFO << scan->children().takeAt(2)->uri().toStdString();
  }
}
