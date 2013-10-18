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


// Standard
#include <iostream>
#include <string>
#include <regex>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "XNATProjects.h"

// Qt
#include <QMessageBox>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QBrush>
#include <QListView>

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

// Boost
//#include "boost/smart_ptr.h"

//qRestAPI
//konnte noch nicht eingebunden werden deshalb wird Poco benutzt

const std::string XNATProjects::VIEW_ID = "org.mitk.views.xnatprojects";
const std::string QUERY_PATH = "/data/archive/projects?format=xml";
const std::string QUERY = "";
const int COUNT_PROJECT_COLUMNS_OR_ROWS = 7;

void XNATProjects::SetFocus()
{
  m_Controls.buttonGetAllProjects->setFocus();
}

void XNATProjects::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.buttonGetAllProjects, SIGNAL(clicked()), this, SLOT(GetAllProjects()) );
}

void XNATProjects::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                      const QList<mitk::DataNode::Pointer>& nodes )
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


void XNATProjects::GetAllProjects()
{  
  //***********************************************
  /*******************MockUpCode*******************
  //XNATProjects::model.reset(new StandardItemModel());
  QStandardItemModel* projectModel =  new QStandardItemModel();
  QStandardItemModel* detailModel =  new QStandardItemModel();
  QStandardItemModel* subjectModel =  new QStandardItemModel();
  QStandardItemModel* experimentModel =  new QStandardItemModel();
  //QScopePointer::QScopedPointer<StandardItemModel> projectModelScopedPtr(new QStandardItemModel());
  
  QStringList project(QString("Projects"));
  QStringList detail(QString("Details of the chosen project"));
  QStringList subject(QString("Subjects"));
  QStringList experiment(QString("Experiments"));

  projectModel->setHorizontalHeaderLabels(project);
  detailModel->setHorizontalHeaderLabels(detail);
  subjectModel->setHorizontalHeaderLabels(subject);
  experimentModel->setHorizontalHeaderLabels(experiment);

  QTreeView* projectView = m_Controls.projectTreeView;
  QTreeView* detailView = m_Controls.detailTreeView;
  QTreeView* subjectView = m_Controls.subjectTreeView;
  QTreeView* experimentView = m_Controls.experimentTreeView;

  //QStandardItem* projectRoot = projectModel->invisibleRootItem();
  //QStandardItem* detailRoot = detailModel->invisibleRootItem();
  //QStandardItem* subjectRoot = subjectModel->invisibleRootItem();
  //QStandardItem* experimentRoot = experimentModel->invisibleRootItem();

  projectModel->appendRow(new QStandardItem("prj001"));
  detailModel->appendRow(new QStandardItem("Secondary_ID"));
  subjectModel->appendRow(new QStandardItem("prj001_sub001"));
  experimentModel->appendRow(new QStandardItem("prj001_sub001_exp001"));

  projectView->setModel(projectModel);
  detailView->setModel(detailModel);
  subjectView->setModel(subjectModel);
  experimentView->setModel(experimentModel);

  projectView->show();
  detailView->show();
  subjectView->show();
  experimentView->show();

  **********************************************/

  /* ********************AUSKOMMENTIERT ZU MOCKUP ZWECKEN ************************
  ********************************************************************************
  */// AUSKOMMENTIERT ZU MOCKUP ZWECKEN ******************************************

  // Assign existing treeView to mytree(View)
  QTreeView* myTree = m_Controls.projectTreeView;

  // MEMBER SCOPED POINTER
  projectModel.reset(new QStandardItemModel());
  QStandardItemModel* model = projectModel.data();

  // Normaler RAW Pointer
  //QStandardItemModel* model =  new QStandardItemModel();
   /*QList<QStandardItem*> columns;
   columns.append(new QStandardItem("Projects"));
   columns.append(new QStandardItem("Subjects"));
   columns.append(new QStandardItem("Experiments"));
   model->appendColumn(columns);*/

  // Clear existing treeView
  myTree->setModel(model);
  myTree->show();
  
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

  // Set input URI from MITK user
  std::string uri = m_Controls.inHostAddress->text().toStdString();
  uri += QUERY_PATH;
  Poco::URI projectURI = Poco::URI(uri);

  // Regular Expression for uri
  std::string uriregex = "^(https?)://([a-zA-Z0-9\.]+):([0-9]+)(/[^ /]+)*$";

  // Validate address
  if ( ! std::tr1::regex_match( m_Controls.inHostAddress->text().toStdString(), std::tr1::regex(uriregex) ) )
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

  // Set query path
  std::string path = projectURI.getPathAndQuery();

  // Create session and request
  Poco::Net::HTTPClientSession session(projectURI.getHost(), projectURI.getPort());
  Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path, Poco::Net::HTTPMessage::HTTP_1_1);

  // Add username and password (HTTP basic authentication) to the request.
  Poco::Net::HTTPBasicCredentials cred(m_Controls.inUser->text().toStdString(), m_Controls.inPassword->text().toStdString());
  cred.authenticate(request);

  // Send the request
  session.sendRequest(request);

  // Receive the response.
  Poco::Net::HTTPResponse response;
  std::istream& rs = session.receiveResponse(response);

  MITK_INFO << "HTTP Statuscode: " << response.getStatus();

  // Checks th HTTP Statuscode
  if ( response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK )
  {
    MITK_ERROR << "HTTP STATUSCODE NOT OK";
    MITK_ERROR << "Getting all projects failed!";
    return;
  }

  //TEST für ausgabe ohne QTreeView (nur Konsole)
  //Poco::StreamCopier::copyStream(rs, std::cout);

  Poco::XML::InputSource inXML(rs);

  Poco::XML::DOMParser parser;
  Poco::AutoPtr<Poco::XML::Document> pDoc = parser.parse(&inXML);
  Poco::XML::NodeIterator it(pDoc, Poco::XML::NodeFilter::SHOW_ALL);
  Poco::XML::Node* pNode = it.nextNode();


  QStandardItem* rootNode = model->invisibleRootItem();

  // Set header label
  QStringList strlist(QString("Projects"));
  model->setHorizontalHeaderLabels(strlist);

  //defining list
  QList<QStandardItem*> list;

  while (pNode)
  {
    //Converting
    QString text = ConvertFromXMLString(pNode->nodeName());

    if (text == QString("row"))
    {
      pNode = it.nextNode();
      text = ConvertFromXMLString(pNode->nodeName());

      if ( text == QString("cell") ){
        pNode = it.nextNode();
        text = QString("ID: ") + ConvertFromXMLString(pNode->nodeValue()).toStdString().c_str();
        QStandardItem* id = new QStandardItem(text);
        rootNode->appendRow(id);

        //defining Items
        QStandardItem* details = new QStandardItem("Details");
        QStandardItem* secondaryId = new QStandardItem("secondary_ID");
        QStandardItem* name = new QStandardItem("name");
        QStandardItem* description = new QStandardItem("description");
        QStandardItem* piFirstname = new QStandardItem("pi_firstname");
        QStandardItem* piLastname = new QStandardItem("pi_lastname");
        QStandardItem* uri = new QStandardItem("URI");

        //filling list
        list.append(secondaryId);
        list.append(name);
        list.append(description);
        list.append(piFirstname);
        list.append(piLastname);
        list.append(uri);

        //refering
        id->appendRow(details);
        details->appendRows(list);

        //clear list
        list.clear();

        //next cell
        pNode = it.nextNode();
        QString wstr = ConvertFromXMLString(pNode->nodeName());

        for ( int i = 0; i < 6; i++ )
        { 
          if ( wstr == QString("cell") )
          {
            pNode = it.nextNode();

            //breaks if a cell has no value and node is "cell" again
            if ( ConvertFromXMLString(pNode->nodeName()) == QString("cell") )
              continue;            

            text = ConvertFromXMLString(pNode->nodeValue());
            QStandardItem* item0 = new QStandardItem();
            QStandardItem* item1 = new QStandardItem();
            QStandardItem* item2 = new QStandardItem();
            QStandardItem* item3 = new QStandardItem();
            QStandardItem* item4 = new QStandardItem();
            QStandardItem* item5 = new QStandardItem();

            //generating and refering
            switch(i)
            {
            case 0:
              item0->setText(text);
              secondaryId->appendRow(item0);
              break;
            case 1:
              item1->setText(text);
              name->appendRow(item1);
              break;
            case 2:
              item2->setText(text);
              description->appendRow(item2);
              break;
            case 3:
              item3->setText(text);
              piFirstname->appendRow(item3);
              break;
            case 4:
              item4->setText(text);
              piLastname->appendRow(item4);
              break;
            case 5:
              item5->setText(text);
              uri->appendRow(item5);
              break;
            default:
              MITK_ERROR << "Fatal Error! Please call your Administrator!";
              break;
            }
            //next cell
            pNode = it.nextNode();
            wstr = ConvertFromXMLString(pNode->nodeName());
          }
        }
        //clear list
        list.clear();
      }
    }    
    if ( !(ConvertFromXMLString(pNode->nodeName()) == QString("row")) )
      pNode = it.nextNode();
  }
  MITK_INFO << "Project tree has been drawn";

  // Set tree visible
  myTree->setModel(model);
  myTree->show();  
}

/**
\brief ******** SEARCH ********
*/
void XNATProjects::SearchWithXmlFilePath(QString filepath){
/*

  // Search-URI
  Poco::URI searchUri("http://localhost:12345/xnat/data/search");
  searchUri.setQuery("format=xml&body=" + filepath);

  // Create session and request
  Poco::Net::HTTPClientSession session(projectURI.getHost(), projectURI.getPort());
  Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, searchUri.getPathAndQuery();, Poco::Net::HTTPMessage::HTTP_1_1);

  // Add username and password (HTTP basic authentication) to the request.
  Poco::Net::HTTPBasicCredentials cred("admin", "admin");
  cred.authenticate(request);

  // Fill request-body with xml-file
  request.setContentType("text/xml; charset=utf-8");
  request.setContentLength();
  request << 

  // Send the request
  session.sendRequest(request);
  
  // Receive the response.
  Poco::Net::HTTPResponse response;
  std::istream& rs = session.receiveResponse(response);

  std::ifstream fis("C:/Users/knorr/Desktop/einfacheSubjectIdAbfrage.xml");

  Poco::XML::InputSource inXML(fis);

  Poco::XML::DOMParser parser;
  Poco::AutoPtr<Poco::XML::Document> pDoc = parser.parse(&inXML);
  Poco::XML::Document(pDoc.get());
  Poco::XML::NodeIterator it(pDoc, Poco::XML::NodeFilter::SHOW_ALL);
  Poco::XML::Node* pNode = it.nextNode();
  
  while(pNode){
    request
    it.nextNode();
  }
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