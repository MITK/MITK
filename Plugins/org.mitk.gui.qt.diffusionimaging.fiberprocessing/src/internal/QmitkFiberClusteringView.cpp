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
#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateAnd.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <itkTractClusteringFilter.h>
#include <mitkFiberBundle.h>
#include <mitkClusteringMetricEuclideanMean.h>
#include <mitkClusteringMetricEuclideanMax.h>
#include <mitkClusteringMetricEuclideanStd.h>
#include <mitkClusteringMetricAnatomic.h>
#include <mitkClusteringMetricScalarMap.h>
#include <mitkClusteringMetricInnerAngles.h>
#include <mitkClusteringMetricLength.h>
#include <QMessageBox>

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

    m_Controls->m_InCentroidsBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_InCentroidsBox->SetPredicate(isFib);
    m_Controls->m_InCentroidsBox->SetZeroEntryText("--");


    mitk::TNodePredicateDataType<mitk::Image>::Pointer imageP = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateDimension::Pointer dimP = mitk::NodePredicateDimension::New(3);

    m_Controls->m_MapBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_MapBox->SetPredicate(mitk::NodePredicateAnd::New(imageP, dimP));

    m_Controls->m_ParcellationBox->SetDataStorage(this->GetDataStorage());
    m_Controls->m_ParcellationBox->SetPredicate(mitk::NodePredicateAnd::New(imageP, dimP));

    m_Controls->m_MetricBox6->setVisible(false);
    m_Controls->m_MetricWeight6->setVisible(false);

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
  if (m_Controls->m_InCentroidsBox->GetSelectedNode().IsNotNull())
  {
    mitk::FiberBundle::Pointer in_centroids = dynamic_cast<mitk::FiberBundle*>(m_Controls->m_InCentroidsBox->GetSelectedNode()->GetData());
    clusterer->SetInCentroids(in_centroids);
  }

  std::vector< mitk::ClusteringMetric* > metrics;
  if (m_Controls->m_MetricBox1->isChecked())
  {
    mitk::ClusteringMetricEuclideanMean* metric = new mitk::ClusteringMetricEuclideanMean();
    metric->SetScale(m_Controls->m_MetricWeight1->value());
    metrics.push_back(metric);
  }
  if (m_Controls->m_MetricBox2->isChecked())
  {
    mitk::ClusteringMetricEuclideanStd* metric = new mitk::ClusteringMetricEuclideanStd();
    metric->SetScale(m_Controls->m_MetricWeight2->value());
    metrics.push_back(metric);
  }
  if (m_Controls->m_MetricBox3->isChecked())
  {
    mitk::ClusteringMetricEuclideanMax* metric = new mitk::ClusteringMetricEuclideanMax();
    metric->SetScale(m_Controls->m_MetricWeight3->value());
    metrics.push_back(metric);
  }
  if (m_Controls->m_MetricBox6->isChecked())
  {
    mitk::ClusteringMetricInnerAngles* metric = new mitk::ClusteringMetricInnerAngles();
    metric->SetScale(m_Controls->m_MetricWeight6->value());
    metrics.push_back(metric);
  }
  if (m_Controls->m_MetricBox7->isChecked())
  {
    mitk::ClusteringMetricLength* metric = new mitk::ClusteringMetricLength();
    metric->SetScale(m_Controls->m_MetricWeight7->value());
    metrics.push_back(metric);
  }

  if (m_Controls->m_ParcellationBox->GetSelectedNode().IsNotNull() && m_Controls->m_MetricBox4->isChecked())
  {
    mitk::Image::Pointer mitk_map = dynamic_cast<mitk::Image*>(m_Controls->m_ParcellationBox->GetSelectedNode()->GetData());

    ShortImageType::Pointer itk_map = ShortImageType::New();
    mitk::CastToItkImage(mitk_map, itk_map);
    mitk::ClusteringMetricAnatomic* metric = new mitk::ClusteringMetricAnatomic();
    metric->SetParcellations({itk_map});
    metric->SetScale(m_Controls->m_MetricWeight4->value());
    metrics.push_back(metric);
  }
  if (m_Controls->m_MapBox->GetSelectedNode().IsNotNull() && m_Controls->m_MetricBox5->isChecked())
  {
    mitk::Image::Pointer mitk_map = dynamic_cast<mitk::Image*>(m_Controls->m_MapBox->GetSelectedNode()->GetData());

    FloatImageType::Pointer itk_map = FloatImageType::New();
    mitk::CastToItkImage(mitk_map, itk_map);
    mitk::ClusteringMetricScalarMap* metric = new mitk::ClusteringMetricScalarMap();
    metric->SetImages({itk_map});
    metric->SetScale(m_Controls->m_MetricWeight5->value());
    metrics.push_back(metric);
  }

  if (metrics.empty())
  {
    QMessageBox::warning(nullptr, "Warning", "No metric selected!");
    return;
  }

  clusterer->SetMetrics(metrics);
  clusterer->SetMergeDuplicateThreshold(m_Controls->m_MergeDuplicatesBox->value());
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
    if (m_Controls->m_InCentroidsBox->GetSelectedNode().IsNotNull())
      this->GetDataStorage()->Add(new_node, m_Controls->m_InCentroidsBox->GetSelectedNode());
    else
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
