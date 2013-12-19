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

// ####### Blueberry includes #######
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// ####### Qmitk includes #######
#include "QmitkConnectomicsStatisticsView.h"
#include "QmitkStdMultiWidget.h"

// ####### Qt includes #######
#include <QMessageBox>

// ####### ITK includes #######
#include <itkRGBAPixel.h>

// ####### MITK includes #######

#include <mitkConnectomicsConstantsManager.h>
#include <mitkConnectomicsStatisticsCalculator.h>

// Includes for image casting between ITK and MITK
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"

const std::string QmitkConnectomicsStatisticsView::VIEW_ID = "org.mitk.views.connectomicsstatistics";

QmitkConnectomicsStatisticsView::QmitkConnectomicsStatisticsView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
, m_currentIndex( 0 )
{
}

QmitkConnectomicsStatisticsView::~QmitkConnectomicsStatisticsView()
{
}


void QmitkConnectomicsStatisticsView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkConnectomicsStatisticsViewControls;
    m_Controls->setupUi( parent );

  }

  this->WipeDisplay();
}


void QmitkConnectomicsStatisticsView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkConnectomicsStatisticsView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkConnectomicsStatisticsView::WipeDisplay()
{
  m_Controls->lblWarning->setVisible( true );
  m_Controls->inputImageOneNameLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->inputImageOneNameLabel->setVisible( false );
  m_Controls->inputImageOneLabel->setVisible( false );
  m_Controls->networkStatisticsPlainTextEdit->clear();
  m_Controls->betweennessNetworkHistogramCanvas->SetHistogram(   NULL );
  m_Controls->degreeNetworkHistogramCanvas->SetHistogram(       NULL );
  m_Controls->shortestPathNetworkHistogramCanvas->SetHistogram( NULL );
  m_Controls->betweennessNetworkHistogramCanvas->update();
  m_Controls->degreeNetworkHistogramCanvas->update();
  m_Controls->shortestPathNetworkHistogramCanvas->update();
  m_Controls->betweennessNetworkHistogramCanvas->Clear();
  m_Controls->degreeNetworkHistogramCanvas->Clear();
  m_Controls->shortestPathNetworkHistogramCanvas->Clear();
  m_Controls->betweennessNetworkHistogramCanvas->Replot();
  m_Controls->degreeNetworkHistogramCanvas->Replot();
  m_Controls->shortestPathNetworkHistogramCanvas->Replot();
}

void QmitkConnectomicsStatisticsView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  this->WipeDisplay();

  // Valid options are either
  // 1 image (parcellation)
  //
  // 1 image (parcellation)
  // 1 fiber bundle
  //
  // 1 network
  if( nodes.size() > 2 )
  {
    return;
  }

  bool currentFormatUnknown(true);

  // iterate all selected objects, adjust warning visibility
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
       it != nodes.end();
       ++it )
  {
    mitk::DataNode::Pointer node = *it;
    currentFormatUnknown = true;

    if( node.IsNotNull()  )
    { // network section
      mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( node->GetData() );
      if( network )
      {
        currentFormatUnknown = false;
        if( nodes.size() != 1 )
        {
          // only valid option is a single network
          this->WipeDisplay();
          return;
        }
        m_Controls->lblWarning->setVisible( false );
        m_Controls->inputImageOneNameLabel->setText(node->GetName().c_str());
        m_Controls->inputImageOneNameLabel->setVisible( true );
        m_Controls->inputImageOneLabel->setVisible( true );

        {
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
          m_Controls->networkStatisticsPlainTextEdit->setPlainText( statisticsString );
        }

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
      }
    } // end network section

    if ( currentFormatUnknown )
    {
      this->WipeDisplay();
      return;
    }
  } // end for loop
}
