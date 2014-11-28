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

// MitkRdf
#include <mitkRdfStore.h>
#include <mitkRdfTriple.h>
#include <mitkRdfNode.h>
#include <mitkRdfUri.h>

const std::string QmitkRdfTriplestoreView::VIEW_ID = "org.mitk.views.qmitkrdftriplestoreview";

void QmitkRdfTriplestoreView::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void QmitkRdfTriplestoreView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()) );
}

void QmitkRdfTriplestoreView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                                 const QList<mitk::DataNode::Pointer>& nodes )
{
}

void QmitkRdfTriplestoreView::DoImageProcessing()
{
  typedef mitk::RdfStore Store;
  typedef mitk::RdfTriple Triple;
  typedef mitk::RdfNode Node;
  typedef mitk::RdfUri Uri;

  // Create a new Store
  Store store;

  // Create a new base URI for the store and set it into the store
  std::string base = "http://www.mitk.org/Base";
  Uri baseUri(base);

  // Create some nodes
  mitk::RdfNode project1(mitk::RdfUri("http://www.mitk.org/rdfOntology/resource.rdf#Project1"));
  mitk::RdfNode subject1(mitk::RdfUri("http://www.mitk.org/rdfOntology/resource.rdf#Subject1"));

  // Create some predicate(/property) nodes
  mitk::RdfNode hasA(mitk::RdfUri("http://www.mitk.org/rdfOntology/property.rdf#hasA"));
  mitk::RdfNode name(mitk::RdfUri("http://www.mitk.org/rdfOntology/property.rdf#name"));

  // Create some triples
  mitk::RdfTriple t1(project1, hasA, subject1);
  mitk::RdfTriple t2(project1, name, (std::string) "MyProject");

  // Add triples to store
  store.Add(t1);
  store.Add(t1);

  // Save the store in a local path
  store.Save("C:/Users/knorr/Desktop/storeFromMitk.rdf");
}
