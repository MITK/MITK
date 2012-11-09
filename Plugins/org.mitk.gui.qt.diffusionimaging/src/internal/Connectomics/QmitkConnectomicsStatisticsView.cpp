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
  m_Controls->numberOfVerticesLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->numberOfEdgesLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->numberOfSelfLoopsLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->averageDegreeLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->connectionDensityLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->efficiencyLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
  m_Controls->globalClusteringLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
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

        int noVertices = network->GetNumberOfVertices();
        int noEdges = network->GetNumberOfEdges();
        int noSelfLoops = network->GetNumberOfSelfLoops();
        double averageDegree = network->GetAverageDegree();
        double connectionDensity = network->GetConnectionDensity();
        double globalClustering = network->GetGlobalClusteringCoefficient();

        m_Controls->numberOfVerticesLabel->setText( QString::number( noVertices ) );
        m_Controls->numberOfEdgesLabel->setText( QString::number( noEdges ) );
        m_Controls->numberOfSelfLoopsLabel->setText( QString::number( noSelfLoops ) );
        m_Controls->averageDegreeLabel->setText( QString::number( averageDegree ) );
        m_Controls->connectionDensityLabel->setText( QString::number( connectionDensity ) );
        m_Controls->globalClusteringLabel->setText( QString::number( globalClustering ) );

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

          double efficiency = histogramContainer->GetShortestPathHistogram()->GetEfficiency();

          m_Controls->efficiencyLabel->setText( QString::number( efficiency ) );
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
