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

#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include "QmitkConnectomicsStatisticsView.h"
#include <QMessageBox>
#include <QStringList>
#include <itkRGBAPixel.h>
#include <mbilog.h>
#include <mitkConnectomicsConstantsManager.h>
#include <mitkConnectomicsStatisticsCalculator.h>
#include <mitkConnectomicsRenderingProperties.h>
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"
#include <mitkNodePredicateDataType.h>
#include <string>

const std::string QmitkConnectomicsStatisticsView::VIEW_ID = "org.mitk.views.connectomicsstatistics";

QmitkConnectomicsStatisticsView::QmitkConnectomicsStatisticsView()
  : QmitkAbstractView()
  , m_Controls( nullptr )
{
}

QmitkConnectomicsStatisticsView::~QmitkConnectomicsStatisticsView()
{
}

void QmitkConnectomicsStatisticsView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {// create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkConnectomicsStatisticsViewControls;
    m_Controls-> setupUi( parent );
    connect( m_Controls-> networkBalloonsNodeLabelsComboBox, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( OnNetworkBalloonsNodeLabelsComboBoxCurrentIndexChanged( ) ) );


    m_Controls->m_NetworkBox->SetDataStorage(this->GetDataStorage());
    mitk::TNodePredicateDataType<mitk::ConnectomicsNetwork>::Pointer isNetwork = mitk::TNodePredicateDataType<mitk::ConnectomicsNetwork>::New();
    m_Controls->m_NetworkBox->SetPredicate( isNetwork );
    connect( (QObject*)(m_Controls->m_NetworkBox), SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));
  }
  this-> WipeDisplay();
}

void QmitkConnectomicsStatisticsView::SetFocus()
{
  m_Controls->networkStatisticsPlainTextEdit->setFocus();
}

void QmitkConnectomicsStatisticsView::WipeDisplay()
{
  m_Controls->inputImageOneLabel->setVisible( false );
  m_Controls->networkStatisticsPlainTextEdit->clear();
  m_Controls->betweennessNetworkHistogramCanvas->SetHistogram(  nullptr );
  m_Controls->degreeNetworkHistogramCanvas->SetHistogram(       nullptr );
  m_Controls->shortestPathNetworkHistogramCanvas->SetHistogram( nullptr );
  m_Controls->betweennessNetworkHistogramCanvas->update();
  m_Controls->degreeNetworkHistogramCanvas->update();
  m_Controls->shortestPathNetworkHistogramCanvas->update();
  m_Controls->betweennessNetworkHistogramCanvas->Clear();
  m_Controls->degreeNetworkHistogramCanvas->Clear();
  m_Controls->shortestPathNetworkHistogramCanvas->Clear();
  m_Controls->betweennessNetworkHistogramCanvas->Replot();
  m_Controls->degreeNetworkHistogramCanvas->Replot();
  m_Controls->shortestPathNetworkHistogramCanvas->Replot();
  m_Controls-> networkBalloonsNodeLabelsComboBox-> QComboBox::clear();
  m_Controls-> networkBalloonsPlainTextEdit-> clear();
}

void QmitkConnectomicsStatisticsView::OnNetworkBalloonsNodeLabelsComboBoxCurrentIndexChanged( )
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();

  if( nodes.size() != 1 ) { return; }

  mitk::DataNode::Pointer node = *nodes.begin();

  if( node.IsNotNull() )
  {
    mitk::ConnectomicsNetwork* network =
        dynamic_cast< mitk::ConnectomicsNetwork* >( node-> GetData() );

    if( network )
    {
      std::string tempCurrentText = m_Controls-> networkBalloonsNodeLabelsComboBox->
                                    QComboBox::currentText().toStdString(); // get text of currently selected item.

      if( tempCurrentText.size() > 3 && tempCurrentText.rfind( ":" ) != tempCurrentText.npos )
      { // update chosenNode property.
        tempCurrentText = tempCurrentText.substr( tempCurrentText.rfind( ":" ) + 2 );
        node-> SetProperty( mitk::connectomicsRenderingNodeChosenNodeName.c_str(),
                            mitk::StringProperty::New( tempCurrentText.c_str() ) );
        auto renderWindowPart = this->GetRenderWindowPart();
        if (renderWindowPart)
        {
          renderWindowPart->ForceImmediateUpdate(); //RequestUpdate() is too slow.
        }
      }

      std::stringstream balloonTextStream;
      node-> Update();

      if( node-> GetProperty( mitk::connectomicsRenderingBalloonTextName.c_str() ) != nullptr &&
          node-> GetProperty( mitk::connectomicsRenderingBalloonNodeStatsName.c_str() ) != nullptr &&
          tempCurrentText != "-1" )
      {
        balloonTextStream << node-> GetProperty( mitk::connectomicsRenderingBalloonTextName.c_str() )
                             -> GetValueAsString()
                          << node-> GetProperty( mitk::connectomicsRenderingBalloonNodeStatsName.c_str() )
                             -> GetValueAsString() << std::endl;
        QString balloonQString ( balloonTextStream.str().c_str() );
        // setPlainText() overwrites, insertPlainText() appends.
        m_Controls-> networkBalloonsPlainTextEdit-> setPlainText( balloonQString );
      }
      if( tempCurrentText == "-1" )
      {
        m_Controls-> networkBalloonsPlainTextEdit-> setPlainText( "" );
      }
    }
  }
  return;
}

void QmitkConnectomicsStatisticsView::UpdateGui()
{
  this->WipeDisplay();

  // iterate all selected objects, adjust warning visibility
  if (m_Controls->m_NetworkBox->GetSelectedNode().IsNotNull())
  {
    mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( m_Controls->m_NetworkBox->GetSelectedNode()->GetData() );

    m_Controls->inputImageOneLabel->setVisible( true );

    std::stringstream statisticsStream;

    mitk::ConnectomicsStatisticsCalculator::Pointer calculator = mitk::ConnectomicsStatisticsCalculator::New();

    calculator->SetNetwork( network );
    calculator->Update();

    statisticsStream << "# Vertices: " << calculator->GetNumberOfVertices() << "\n";
    statisticsStream << "# Edges: " << calculator->GetNumberOfEdges() << "\n";
    statisticsStream << "Average Degree: " << calculator->GetAverageDegree() << "\n";
    statisticsStream << "Density: " << calculator->GetConnectionDensity() << "\n";
    statisticsStream << "Small Worldness: " << calculator->GetSmallWorldness() << "\n";
    statisticsStream << "Average Path Length: " << calculator->GetAveragePathLength() << "\n";
    statisticsStream << "Efficiency: " << (1 / calculator->GetAveragePathLength() ) << "\n";
    statisticsStream << "# Connected Components: " << calculator->GetNumberOfConnectedComponents() << "\n";
    statisticsStream << "Average Component Size: " << calculator->GetAverageComponentSize() << "\n";
    statisticsStream << "Largest Component Size: " << calculator->GetLargestComponentSize() << "\n";
    statisticsStream << "Average Clustering Coefficient: " << calculator->GetAverageClusteringCoefficientsC() << "\n";
    statisticsStream << "Average Vertex Betweenness Centrality: " << calculator->GetAverageVertexBetweennessCentrality() << "\n";
    statisticsStream << "Average Edge Betweenness Centrality: " << calculator->GetAverageEdgeBetweennessCentrality() << "\n";
    statisticsStream << "# Isolated Points: " << calculator->GetNumberOfIsolatedPoints() << "\n";
    statisticsStream << "# End Points: " << calculator->GetNumberOfEndPoints() << "\n";
    statisticsStream << "Diameter: " << calculator->GetDiameter() << "\n";
    statisticsStream << "Radius: " << calculator->GetRadius() << "\n";
    statisticsStream << "Average Eccentricity: " << calculator->GetAverageEccentricity() << "\n";
    statisticsStream << "# Central Points: " << calculator->GetNumberOfCentralPoints() << "\n";

    QString statisticsString( statisticsStream.str().c_str() );
    m_Controls-> networkStatisticsPlainTextEdit-> setPlainText( statisticsString );

    mitk::ConnectomicsNetwork::Pointer connectomicsNetwork( network );
    mitk::ConnectomicsHistogramsContainer *histogramContainer = histogramCache[ connectomicsNetwork ];
    if(histogramContainer)
    {
      m_Controls->betweennessNetworkHistogramCanvas->SetHistogram(  histogramContainer->GetBetweennessHistogram() );
      m_Controls->degreeNetworkHistogramCanvas->SetHistogram(       histogramContainer->GetDegreeHistogram() );
      m_Controls->shortestPathNetworkHistogramCanvas->SetHistogram( histogramContainer->GetShortestPathHistogram() );
      m_Controls->betweennessNetworkHistogramCanvas->DrawProfiles();
      m_Controls->degreeNetworkHistogramCanvas->DrawProfiles();
      m_Controls->shortestPathNetworkHistogramCanvas->DrawProfiles();
    }

    // For the balloon overlay:
    if( m_Controls->m_NetworkBox->GetSelectedNode()->GetProperty( mitk::connectomicsRenderingBalloonAllNodeLabelsName.c_str() ) != nullptr )
    { // QComboBox with node label names and numbers.
      QString allNodesLabel = m_Controls->m_NetworkBox->GetSelectedNode()->
                              GetProperty( mitk::connectomicsRenderingBalloonAllNodeLabelsName.c_str() )->
                              GetValueAsString().c_str();
      QStringList allNodesLabelList = allNodesLabel.simplified().split( "," );
      allNodesLabelList.sort( Qt::CaseInsensitive );
      m_Controls-> networkBalloonsNodeLabelsComboBox-> QComboBox::addItem( "no node chosen: -1" );
      m_Controls-> networkBalloonsNodeLabelsComboBox-> QComboBox::addItems( allNodesLabelList );
    }
  }
}

void QmitkConnectomicsStatisticsView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& )
{
}
