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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkRdfTriplestoreView.h"

// Qt
#include <QMessageBox>
#include <qcompleter.h>

// Mitk
#include <mitkRdfStore.h>
#include <mitkStringProperty.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateNot.h>

// GDCM
#include <gdcmUUIDGenerator.h>

const std::string QmitkRdfTriplestoreView::VIEW_ID = "org.mitk.views.rdftriplestore";

typedef mitk::RdfStore Store;
typedef mitk::RdfTriple Triple;
typedef mitk::RdfNode Node;
typedef mitk::RdfUri Uri;

void QmitkRdfTriplestoreView::SetFocus()
{
  //m_Controls.buttonPerformGenerate->setFocus();
}

void QmitkRdfTriplestoreView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  m_Controls.lineEditDataNode->setEnabled( false );
  m_Controls.comboBoxProperty->setAutoCompletionCaseSensitivity(Qt::CaseInsensitive);
  m_Controls.comboBoxProperty->setAutoCompletion(true);
  m_Controls.comboBoxProperty->setEditable(false);

  //QStringList s;
  //s << "";
  //s << "hasA";
  //m_Controls.comboBoxProperty->addItems(s);
  m_Controls.comboBoxProperty->addItem("title");
  m_Controls.comboBoxProperty->addItem("source");
  m_Controls.comboBoxProperty->addItem("format");

  // Set up the Triplestore
  m_Store.SetBaseUri(Uri("file:C:/Users/knorr/Desktop/BaseOntologyMitk.rdf"));
  m_Store.Import("file:D:/home/knorr/builds/tripleStore/Ontologies/dcterms.ttl");

  connect( m_Controls.buttonPerformGenerate, SIGNAL(clicked()), this, SLOT(GenerateRdfFile()) );
  connect( m_Controls.buttonPerformImport, SIGNAL(clicked()), this, SLOT(ImportRdfFile()) );
  connect( m_Controls.buttonDataStorageToTriples, SIGNAL(clicked()), this, SLOT(DataStorageToTriples()) );
}

void QmitkRdfTriplestoreView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                                 const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() )
    {
      m_Controls.labelWarning->setVisible( false );
      m_Controls.widgetAddTriple->setEnabled( true );
      m_Controls.lineEditDataNode->setText(QString(node->GetName().c_str()));
      //m_Controls.lineEditProperty->setText(QString(node->GetProperty("path")->GetValueAsString().c_str()));

      for( mitk::PropertyList::PropertyMap::const_iterator i = node->GetData()->GetPropertyList()->GetMap()->begin();
        i != node->GetData()->GetPropertyList()->GetMap()->end(); i++ )
      {
        //MITK_INFO << i->first << " " << i->second->GetValueAsString();
      }

      return;
    }
  }

  m_Controls.labelWarning->setVisible( true );
  m_Controls.widgetAddTriple->setEnabled( false );
}

void QmitkRdfTriplestoreView::GenerateRdfFile()
{
  // Create a new Store
  mitk::RdfStore store;

  // Create a new base URI for the store and set it into the store
  std::string base = "http://www.mitk.org/BaseOntology";
  mitk::RdfUri baseUri(base);

  store.SetBaseUri(baseUri);
  store.Import("file:C:/Users/knorr/Desktop/BaseOntologyMitk.rdf");
  store.AddPrefix("bomrdf", Uri("file:C:/Users/knorr/Desktop/BaseOntologyMitk.rdf"));
  store.AddPrefix("sfm", Uri("file:C:/Users/knorr/Desktop/storeFromMitk.rdf"));

  // Create some nodes
  mitk::RdfNode project1(mitk::RdfUri("sfm:Project1"));
  mitk::RdfNode subject1(mitk::RdfUri("sfm:Subject1"));
  mitk::RdfNode proj(mitk::RdfUri("bomrdf:Project"));
  mitk::RdfNode subj(mitk::RdfUri("bomrdf:Subject"));

  // Create some predicate(/property) nodes
  mitk::RdfNode hasA(mitk::RdfUri("bomrdf:has"));
  mitk::RdfNode name(mitk::RdfUri("bomrdf:name"));
  mitk::RdfNode a(mitk::RdfUri("rdf:type"));

  // Create some triples

  mitk::RdfTriple t1(project1, hasA, subject1);
  mitk::RdfTriple t2(project1, name, "MyProject");
  Triple t3(project1, a, proj);
  Triple t4(subject1, a, subj);
  Triple t5(subject1, name, "MySubject");

  // Add triples to store
  store.Add(t1);
  store.Add(t3);
  store.Add(t2);
  store.Add(t4);
  store.Add(t5);

  // Save the store in a local path
  store.Save("C:/Users/knorr/Desktop/storeFromMitk.rdf");

  //std::string query = "SELECT ?x WHERE {?x bomrdf:name [] .}";

  //Store::ResultMap map = store.Query(query);
  //for (Store::ResultMap::const_iterator i = map.begin(); i != map.end(); i++)
  //{
  //  MITK_INFO << i->first << " " << i->second.value << " " << i->second.type << " " << i->second.datatype;
  //}

  store.CleanUp();
}

void QmitkRdfTriplestoreView::ImportRdfFile()
{
  Store store;
  //store.SetBaseUri(Uri("http://mitk.org/wiki/MITK"));
  store.Import("file:C:/Users/knorr/Desktop/BaseOntologyMitk.rdf#");
  store.Import("file:///D:/home/knorr/builds/tripleStore/Ontologies/dcterms.ttl");
  //store.AddPrefix("bomowl", Uri("file:C:/Users/knorr/Desktop/BaseOntologyMitk.owl#"));

  store.Save("C:/Users/knorr/Desktop/storeFromMitkWithDC.rdf");

  //std::string query = "SELECT ?x FROM <file:C:/Users/knorr/Desktop/storeFromMitkWithDC.rdf> WHERE {?x dc:title ?title .}";

  //Store::ResultMap map = store.Query(query);
  //for (Store::ResultMap::const_iterator i = map.begin(); i != map.end(); i++)
  //{
  //  MITK_INFO << i->first << " " << i->second.value << " " << i->second.type << " " << i->second.datatype;
  //}
}

void QmitkRdfTriplestoreView::DataStorageToTriples()
{
  gdcm::UUIDGenerator generator;

  // Take all BaseData nodes but dont take helper objects
  mitk::NodePredicateNot::Pointer isNotHelperObject =
    mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"));
  //mitk::TNodePredicateDataType<mitk::BaseData>::Pointer isBaseData =
  //  mitk::TNodePredicateDataType<mitk::BaseData>::New();
  //mitk::NodePredicateAnd::Pointer isNotHelperButBaseData =
  //  mitk::NodePredicateAnd::New( isBaseData, isNotHelperObject );

  mitk::DataStorage::SetOfObjects::ConstPointer nodeContainer = GetDataStorage()->GetSubset(isNotHelperObject);

  for( mitk::DataStorage::SetOfObjects::ConstIterator iter = nodeContainer->Begin();
    iter != nodeContainer->End(); iter++ )
  {
    const mitk::DataNode::Pointer node = iter->Value();

    if (!node->GetData()) continue;

    std::string name = node->GetProperty("name")->GetValueAsString();
    std::string nodeUUID = generator.Generate();
    node->SetProperty("uuid", mitk::StringProperty::New(nodeUUID));

    //const mitk::PropertyList::PropertyMap* map = node->GetData()->GetPropertyList()->GetMap();
    //for (mitk::PropertyList::PropertyMap::const_iterator i = map->begin(); i != map->end(); i++)
    //{
    //  MITK_INFO << i->first << " " << i->second;
    //}

    // Get the parent of a DataNode
    mitk::DataStorage::SetOfObjects::ConstPointer sourceContainer = GetDataStorage()->GetSources(node);

    for( mitk::DataStorage::SetOfObjects::ConstIterator sources = nodeContainer->Begin();
      sources != nodeContainer->End(); sources++ )
    {
      const mitk::DataNode::Pointer sourceNode = sources->Value();

      std::string sourceName = sourceNode->GetProperty("name")->GetValueAsString();
      std::string sourceUUID = generator.Generate();
      sourceNode->SetProperty("uuid", mitk::StringProperty::New(sourceUUID));

      Uri first(":" + nodeUUID);
      Uri second(":" + sourceUUID);

      Node sub(first);
      Node pred(Uri("dcterms:source"));
      Node obj(second);

      Triple t(sub, pred, obj);

      m_Store.Add(t);
      std::cout << t;
    }
  }
}
