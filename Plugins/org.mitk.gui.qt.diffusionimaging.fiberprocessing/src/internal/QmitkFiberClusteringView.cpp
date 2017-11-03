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

#include "QmitkFiberClusteringView.h"

#include <mitkNodePredicateDataType.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <itkTractClusteringFilter.h>
#include <mitkFiberBundle.h>

const std::string QmitkFiberClusteringView::VIEW_ID = "org.mitk.views.fiberclustering";
using namespace mitk;

QmitkFiberClusteringView::QmitkFiberClusteringView()
  : QmitkAbstractView()
  , m_Controls( nullptr )
{

}

// Destructor
QmitkFiberClusteringView::~QmitkFiberClusteringView()
{

}

void QmitkFiberClusteringView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkFiberClusteringViewControls;
    m_Controls->setupUi( parent );

    connect( m_Controls->m_StartButton, SIGNAL(clicked()), this, SLOT(StartClustering()) );
    connect( m_Controls->m_TractBox, SIGNAL(currentIndexChanged(int)), this, SLOT(FiberSelectionChanged()) );

    mitk::TNodePredicateDataType<mitk::FiberBundle>::Pointer isFib = mitk::TNodePredicateDataType<mitk::FiberBundle>::New();
    m_Controls->m_TractBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_TractBox->SetPredicate(isFib);

    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    m_Controls->m_MapBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_MapBox->SetPredicate(isImage);
    m_Controls->m_MapBox->SetZeroEntryText("--");

    FiberSelectionChanged();
  }

}

void QmitkFiberClusteringView::SetFocus()
{
  FiberSelectionChanged();
}

void QmitkFiberClusteringView::FiberSelectionChanged()
{
  if (m_Controls->m_TractBox->GetSelectedNode().IsNull())
    m_Controls->m_StartButton->setEnabled(false);
  else
    m_Controls->m_StartButton->setEnabled(true);
}

void QmitkFiberClusteringView::StartClustering()
{
  if (m_Controls->m_TractBox->GetSelectedNode().IsNull())
    return;

  mitk::DataNode::Pointer node = m_Controls->m_TractBox->GetSelectedNode();
  float clusterSize = m_Controls->m_ClusterSizeBox->value();

  mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());

  float max_d = 0;
  int i=1;
  std::vector< float > distances;
  while (max_d < fib->GetGeometry()->GetDiagonalLength()/2)
  {
    distances.push_back(clusterSize*i);
    max_d = clusterSize*i;
    ++i;
  }

  itk::TractClusteringFilter::Pointer clusterer = itk::TractClusteringFilter::New();
  clusterer->SetDistances(distances);
  clusterer->SetTractogram(fib);
  switch (m_Controls->m_MetricBox->currentIndex())
  {
  case 0:
    clusterer->SetMetric(itk::TractClusteringFilter::Metric::MDF);
    break;
  case 1:
    clusterer->SetMetric(itk::TractClusteringFilter::Metric::MDF_VAR);
    break;
  case 2:
    clusterer->SetMetric(itk::TractClusteringFilter::Metric::MAX_MDF);
    break;
  }

  if (m_Controls->m_MapBox->GetSelectedNode().IsNotNull())
  {
    mitk::Image::Pointer mitk_map = dynamic_cast<mitk::Image*>(m_Controls->m_MapBox->GetSelectedNode()->GetData());
    if (mitk_map->GetDimension()==3)
    {
      FloatImageType::Pointer itk_map = FloatImageType::New();
      mitk::CastToItkImage(mitk_map, itk_map);
      clusterer->SetScalarMap(itk_map);
    }
  }
  clusterer->SetMergeDuplicateThreshold(m_Controls->m_MergeDuplicatesBox->value());
  clusterer->SetScale(m_Controls->m_MapScaleBox->value());
  clusterer->SetNumPoints(m_Controls->m_FiberPointsBox->value());
  clusterer->SetMaxClusters(m_Controls->m_MaxClustersBox->value());
  clusterer->SetMinClusterSize(m_Controls->m_MinFibersBox->value());
  clusterer->Update();
  std::vector<mitk::FiberBundle::Pointer> tracts = clusterer->GetOutTractograms();
  std::vector<mitk::FiberBundle::Pointer> centroids = clusterer->GetOutCentroids();

  unsigned int c = 0;
  for (auto f : tracts)
  {
    mitk::DataNode::Pointer new_node = mitk::DataNode::New();
    new_node->SetData(f);
    new_node->SetName("Cluster_" + boost::lexical_cast<std::string>(c));
    this->GetDataStorage()->Add(new_node, node);
    node->SetVisibility(false);

    if (m_Controls->m_CentroidsBox->isChecked())
    {
      mitk::DataNode::Pointer new_node2 = mitk::DataNode::New();
      new_node2->SetData(centroids.at(c));
      new_node2->SetName("Centroid_" + boost::lexical_cast<std::string>(c));
      this->GetDataStorage()->Add(new_node2, new_node);
    }
    ++c;
  }
}

void QmitkFiberClusteringView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& )
{

}
