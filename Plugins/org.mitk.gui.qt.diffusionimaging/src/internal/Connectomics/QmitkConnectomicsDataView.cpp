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
#include "QmitkConnectomicsDataView.h"
#include "QmitkStdMultiWidget.h"

// ####### Qt includes #######
#include <QMessageBox>

// ####### ITK includes #######
#include <itkRGBAPixel.h>

// ####### MITK includes #######

#include <mitkConnectomicsConstantsManager.h>
#include "mitkConnectomicsSyntheticNetworkGenerator.h"
#include "mitkConnectomicsSimulatedAnnealingManager.h"
#include "mitkConnectomicsSimulatedAnnealingPermutationModularity.h"
#include "mitkConnectomicsSimulatedAnnealingCostFunctionModularity.h"
//#include <itkConnectomicsNetworkToConnectivityMatrixImageFilter.h>

// Includes for image casting between ITK and MITK
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"

const std::string QmitkConnectomicsDataView::VIEW_ID = "org.mitk.views.connectomicsdata";

QmitkConnectomicsDataView::QmitkConnectomicsDataView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
    , m_ConnectomicsNetworkCreator( mitk::ConnectomicsNetworkCreator::New() )
    , m_demomode( false )
    , m_currentIndex( 0 )
{
}

QmitkConnectomicsDataView::~QmitkConnectomicsDataView()
{
}


void QmitkConnectomicsDataView::CreateQtPartControl( QWidget *parent )
{
    // build up qt view, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkConnectomicsDataViewControls;
        m_Controls->setupUi( parent );

        QObject::connect( m_Controls->networkifyPushButton, SIGNAL(clicked()), this, SLOT(OnNetworkifyPushButtonClicked()) );
        QObject::connect( m_Controls->syntheticNetworkCreationPushButton, SIGNAL(clicked()), this, SLOT(OnSyntheticNetworkCreationPushButtonClicked()) );
        QObject::connect( (QObject*)( m_Controls->syntheticNetworkComboBox ), SIGNAL(currentIndexChanged (int)),  this, SLOT(OnSyntheticNetworkComboBoxCurrentIndexChanged(int)) );
    }

    // GUI is different for developer and demo mode
    m_demomode = true;
    if( m_demomode )
    {
        this->m_Controls->networkifyPushButton->show();
        this->m_Controls->networkifyPushButton->setText( "Create Network" );
        this->m_Controls->syntheticNetworkOptionsGroupBox->show();
        //--------------------------- fill comboBox---------------------------
        this->m_Controls->syntheticNetworkComboBox->insertItem(0,"Regular lattice");
        this->m_Controls->syntheticNetworkComboBox->insertItem(1,"Heterogenic sphere");
        this->m_Controls->syntheticNetworkComboBox->insertItem(2,"Random network");
    }
    else
    {
        this->m_Controls->networkifyPushButton->show();
        this->m_Controls->networkifyPushButton->setText( "Networkify" );
        this->m_Controls->syntheticNetworkOptionsGroupBox->show();
        //--------------------------- fill comboBox---------------------------
        this->m_Controls->syntheticNetworkComboBox->insertItem(0,"Regular lattice");
        this->m_Controls->syntheticNetworkComboBox->insertItem(1,"Heterogenic sphere");
        this->m_Controls->syntheticNetworkComboBox->insertItem(2,"Random network");
        this->m_Controls->syntheticNetworkComboBox->insertItem(3,"Scale free network");
        this->m_Controls->syntheticNetworkComboBox->insertItem(4,"Small world network");
    }

    this->WipeDisplay();
}


void QmitkConnectomicsDataView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;
}


void QmitkConnectomicsDataView::StdMultiWidgetNotAvailable()
{
    m_MultiWidget = NULL;
}

void QmitkConnectomicsDataView::WipeDisplay()
{
    m_Controls->lblWarning->setVisible( true );
    m_Controls->inputImageOneNameLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
    m_Controls->inputImageOneNameLabel->setVisible( false );
    m_Controls->inputImageOneLabel->setVisible( false );
    m_Controls->inputImageTwoNameLabel->setText( mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_DASH );
    m_Controls->inputImageTwoNameLabel->setVisible( false );
    m_Controls->inputImageTwoLabel->setVisible( false );
}

void QmitkConnectomicsDataView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    this->WipeDisplay();

    // Valid options are either
    // 1 image (parcellation)
    //
    // 1 image (parcellation)
    // 1 fiber bundle
    //
    // 1 network
    if( nodes.size() < 2 )
    {
        return;
    }

    m_SelectedParcellationImage = NULL;
    m_SelectedFiberBundles.clear();

    bool alreadyImageSelected( false ), currentFormatUnknown( true );
    // iterate all selected objects, adjust warning visibility
    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
         it != nodes.end();
         ++it )
    {
        mitk::DataNode::Pointer node = *it;
        currentFormatUnknown = true;

        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            currentFormatUnknown = false;
            if( alreadyImageSelected )
            {
                this->WipeDisplay();
                return;
            }

            m_SelectedParcellationImage = node;

            alreadyImageSelected = true;
            m_Controls->lblWarning->setVisible( false );
            m_Controls->inputImageOneNameLabel->setText(node->GetName().c_str());
            m_Controls->inputImageOneNameLabel->setVisible( true );
            m_Controls->inputImageOneLabel->setVisible( true );
        }

        if( node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
        {
            currentFormatUnknown = false;
            // a fiber bundle has to be in conjunction with a parcellation
            if( nodes.size() < 2 )
            {
                this->WipeDisplay();
                return;
            }

            m_SelectedFiberBundles.push_back(node);
            m_Controls->lblWarning->setVisible( false );
            m_Controls->inputImageTwoNameLabel->setText(node->GetName().c_str());
            m_Controls->inputImageTwoNameLabel->setVisible( true );
            m_Controls->inputImageTwoLabel->setVisible( true );
        }

        { // network section
            mitk::ConnectomicsNetwork* network = dynamic_cast<mitk::ConnectomicsNetwork*>( node->GetData() );
            if( node.IsNotNull() && network )
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

            }
        } // end network section

        if ( currentFormatUnknown )
        {
            this->WipeDisplay();
            return;
        }
    } // end for loop
}

void QmitkConnectomicsDataView::OnSyntheticNetworkComboBoxCurrentIndexChanged(int currentIndex)
{
    m_currentIndex = currentIndex;

    switch (m_currentIndex) {
    case 0:
        this->m_Controls->parameterOneLabel->setText( "Nodes per side" );
        this->m_Controls->parameterTwoLabel->setText( "Internode distance" );
        this->m_Controls->parameterOneSpinBox->setEnabled( true );
        this->m_Controls->parameterOneSpinBox->setValue( 5 );
        this->m_Controls->parameterTwoDoubleSpinBox->setEnabled( true );
        this->m_Controls->parameterTwoDoubleSpinBox->setMaximum( 999.999 );
        this->m_Controls->parameterTwoDoubleSpinBox->setValue( 10.0 );
        break;
    case 1:
        this->m_Controls->parameterOneLabel->setText( "Number of nodes" );
        this->m_Controls->parameterTwoLabel->setText( "Radius" );
        this->m_Controls->parameterOneSpinBox->setEnabled( true );
        this->m_Controls->parameterOneSpinBox->setValue( 1000 );
        this->m_Controls->parameterTwoDoubleSpinBox->setEnabled( true );
        this->m_Controls->parameterTwoDoubleSpinBox->setMaximum( 999.999 );
        this->m_Controls->parameterTwoDoubleSpinBox->setValue( 50.0 );
        break;
    case 2:
        this->m_Controls->parameterOneLabel->setText( "Number of nodes" );
        this->m_Controls->parameterTwoLabel->setText( "Edge percentage" );
        this->m_Controls->parameterOneSpinBox->setEnabled( true );
        this->m_Controls->parameterOneSpinBox->setValue( 100 );
        this->m_Controls->parameterTwoDoubleSpinBox->setEnabled( true );
        this->m_Controls->parameterTwoDoubleSpinBox->setMaximum( 1.0 );
        this->m_Controls->parameterTwoDoubleSpinBox->setValue( 0.5 );
        break;
    case 3:
        //GenerateSyntheticScaleFreeNetwork( network, 1000 );
        break;
    case 4:
        //GenerateSyntheticSmallWorldNetwork( network, 1000 );
        break;
    default:
        this->m_Controls->parameterOneLabel->setText( "Parameter 1" );
        this->m_Controls->parameterTwoLabel->setText( "Paramater 2" );
        this->m_Controls->parameterOneSpinBox->setEnabled( false );
        this->m_Controls->parameterOneSpinBox->setValue( 0 );
        this->m_Controls->parameterTwoDoubleSpinBox->setEnabled( false );
        this->m_Controls->parameterTwoDoubleSpinBox->setValue( 0.0 );
    }
}

void QmitkConnectomicsDataView::OnSyntheticNetworkCreationPushButtonClicked()
{
    // warn if trying to create a very big network
    // big network is a network with > 5000 nodes (estimate)
    // this might fill up the memory to the point it freezes
    int numberOfNodes( 0 );
    switch (m_currentIndex) {
    case 0:
        numberOfNodes = this->m_Controls->parameterOneSpinBox->value()
                * this->m_Controls->parameterOneSpinBox->value()
                * this->m_Controls->parameterOneSpinBox->value();
        break;
    case 1:
        numberOfNodes = this->m_Controls->parameterOneSpinBox->value();
        break;
    case 2:
        numberOfNodes = this->m_Controls->parameterOneSpinBox->value();
        break;
    case 3:
        // not implemented yet
        break;
    case 4:
        // not implemented yet
        break;
    default:
        break;

    }

    if( numberOfNodes > 5000 )
    {
        QMessageBox msgBox;
        msgBox.setText("Trying to generate very large network.");
        msgBox.setIcon( QMessageBox::Warning );
        msgBox.setInformativeText("You are trying to generate a network with more than 5000 nodes, this is very resource intensive and might lead to program instability. Proceed with network generation?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();

        switch (ret) {
        case QMessageBox::Yes:
            // continue
            break;
        case QMessageBox::No:
            // stop
            return;
            break;

        default:
            // should never be reached
            break;
        }
    }

    // proceed
    mitk::ConnectomicsSyntheticNetworkGenerator::Pointer generator = mitk::ConnectomicsSyntheticNetworkGenerator::New();

    mitk::DataNode::Pointer networkNode = mitk::DataNode::New();
    int parameterOne = this->m_Controls->parameterOneSpinBox->value();
    double parameterTwo = this->m_Controls->parameterTwoDoubleSpinBox->value();
    //add network to datastorage
    networkNode->SetData( generator->CreateSyntheticNetwork( m_currentIndex, parameterOne, parameterTwo ) );
    networkNode->SetName( mitk::ConnectomicsConstantsManager::CONNECTOMICS_PROPERTY_DEFAULT_CNF_NAME );
    if( generator->WasGenerationSuccessfull() )
    {
        this->GetDefaultDataStorage()->Add( networkNode );
    }
    else
    {
        MITK_WARN << "Problem occured during synthetic network generation.";
    }

    return;
}

void QmitkConnectomicsDataView::OnNetworkifyPushButtonClicked()
{
    if ( m_SelectedParcellationImage.IsNull() || m_SelectedFiberBundles.empty() )
    {
        QMessageBox::information( NULL, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_CONNECTOMICS_CREATION, mitk::ConnectomicsConstantsManager::CONNECTOMICS_GUI_SELECTION_WARNING);
        return;
    }

    mitk::Image::Pointer image = dynamic_cast< mitk::Image* >(m_SelectedParcellationImage->GetData());
    for (int i=0; i<m_SelectedFiberBundles.size(); i++)
    {
        mitk::FiberBundleX* fiberBundle = dynamic_cast<mitk::FiberBundleX*>( m_SelectedFiberBundles.at(i)->GetData() );

        m_ConnectomicsNetworkCreator= mitk::ConnectomicsNetworkCreator::New();
        m_ConnectomicsNetworkCreator->SetSegmentation( image );
        m_ConnectomicsNetworkCreator->SetFiberBundle( fiberBundle );
        m_ConnectomicsNetworkCreator->CalculateCenterOfMass();
        m_ConnectomicsNetworkCreator->SetEndPointSearchRadius( 15 );
        m_ConnectomicsNetworkCreator->CreateNetworkFromFibersAndSegmentation();
        mitk::DataNode::Pointer networkNode = mitk::DataNode::New();

        //add network to datastorage
        QString name(m_SelectedFiberBundles.at(i)->GetName().c_str());
        name += "_Connectome";
        networkNode->SetData( m_ConnectomicsNetworkCreator->GetNetwork() );
        networkNode->SetName( name.toStdString().c_str() );
        this->GetDefaultDataStorage()->Add( networkNode, m_SelectedParcellationImage );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

}
