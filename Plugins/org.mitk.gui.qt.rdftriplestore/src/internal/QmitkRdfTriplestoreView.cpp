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

// MitkRdf
#include <mitkRdfStore.h>

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
  connect( m_Controls.buttonPerformGenerate, SIGNAL(clicked()), this, SLOT(GenerateRdfFile()) );
  connect( m_Controls.buttonPerformImport, SIGNAL(clicked()), this, SLOT(ImportRdfFile()) );

  m_Controls.lineEditDataNode->setEnabled( false );
  m_Controls.comboBoxProperty->setAutoCompletionCaseSensitivity(Qt::CaseInsensitive);
  m_Controls.comboBoxProperty->setAutoCompletion(true);
  m_Controls.comboBoxProperty->setEditable(false);

  QStringList s;
  s << "";
  s << "hasA";
  m_Controls.comboBoxProperty->addItems(s);
  m_Controls.comboBoxProperty->addItem("title");
  m_Controls.comboBoxProperty->addItem("source");
  m_Controls.comboBoxProperty->addItem("format");
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

      for( mitk::PropertyList::PropertyMap::const_iterator i = node->GetPropertyList()->GetMap()->begin();
        i != node->GetPropertyList()->GetMap()->end(); i++ )
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

  // Create some nodes
  mitk::RdfNode project1(mitk::RdfUri("Project1"));
  mitk::RdfNode subject1(mitk::RdfUri("Subject1"));

  // Create some predicate(/property) nodes
  mitk::RdfNode hasA(mitk::RdfUri("hasA"));
  mitk::RdfNode name(mitk::RdfUri("name"));

  // Create some triples
  mitk::RdfTriple t1(project1, hasA, subject1);
  mitk::RdfTriple t2(project1, name, mitk::RdfNode("MyProject"));

  // Add triples to store
  store.Add(t1);
  store.Add(t1);
  store.Add(t2);

  // Save the store in a local path
  store.Save("C:/Users/knorr/Desktop/storeFromMitk.rdf");
  store.CleanUp();
}

void QmitkRdfTriplestoreView::ImportRdfFile()
{
  Store store;
  //store.SetBaseUri(Uri("http://mitk.org/wiki/MITK"));
  store.Import("file:C:/Users/knorr/Desktop/BaseOntologyMitk.rdf");
  store.Import("file:C:/Users/knorr/Desktop/BaseOntologyMITK.owl");
  //store.Import("http://dublincore.org/documents/2012/06/14/dcmi-terms/?v=elements#");


  store.Save("C:/Users/knorr/Desktop/storeFromMitkWithDC.rdf");
}
