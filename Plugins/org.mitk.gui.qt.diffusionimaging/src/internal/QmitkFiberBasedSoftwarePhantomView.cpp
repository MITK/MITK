/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
 Version:   $Revision: 21975 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

//misc
#define _USE_MATH_DEFINES
#include <math.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkFiberBasedSoftwarePhantomView.h"

// MITK
#include <mitkImage.h>
#include <mitkDiffusionImage.h>
#include <mitkImageToItk.h>
#include <itkDwiPhantomGenerationFilter.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkDataStorage.h>
#include <itkFibersFromPointsFilter.h>
#include <itkTractsToDWIImageFilter.h>
#include <mitkTensorImage.h>

#include <QMessageBox>

#define _USE_MATH_DEFINES
#include <math.h>

const std::string QmitkFiberBasedSoftwarePhantomView::VIEW_ID = "org.mitk.views.FiberBasedSoftwarePhantomView";

QmitkFiberBasedSoftwarePhantomView::QmitkFiberBasedSoftwarePhantomView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
    , m_SelectedImage( NULL )
    , m_SelectedBundle( NULL )
{

}

// Destructor
QmitkFiberBasedSoftwarePhantomView::~QmitkFiberBasedSoftwarePhantomView()
{

}


void QmitkFiberBasedSoftwarePhantomView::CreateQtPartControl( QWidget *parent )
{
    // build up qt view, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkFiberBasedSoftwarePhantomViewControls;
        m_Controls->setupUi( parent );

        connect((QObject*) m_Controls->m_GenerateImageButton, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateImage()));
        connect((QObject*) m_Controls->m_GenerateFibersButton, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateFibers()));
        connect((QObject*) m_Controls->m_FiberButton, SIGNAL(clicked()), (QObject*) this, SLOT(OnAddBundle()));
        connect((QObject*) m_Controls->m_CircleButton, SIGNAL(clicked()), (QObject*) this, SLOT(OnDrawCircle()));
    }
}

QmitkFiberBasedSoftwarePhantomView::GradientListType QmitkFiberBasedSoftwarePhantomView::GenerateHalfShell(int NPoints)
{
    NPoints *= 2;
    vnl_vector<double> theta; theta.set_size(NPoints);

    vnl_vector<double> phi; phi.set_size(NPoints);

    double C = sqrt(4*M_PI);

    phi(0) = 0.0;
    phi(NPoints-1) = 0.0;

    for(int i=0; i<NPoints; i++)
    {
        theta(i) = acos(-1.0+2.0*i/(NPoints-1.0)) - M_PI / 2.0;
        if( i>0 && i<NPoints-1)
        {
            phi(i) = (phi(i-1) + C /
                      sqrt(NPoints*(1-(-1.0+2.0*i/(NPoints-1.0))*(-1.0+2.0*i/(NPoints-1.0)))));
            // % (2*DIST_POINTSHELL_PI);
        }
    }

    GradientListType pointshell;

    int numB0 = NPoints/10;
    if (numB0==0)
        numB0=1;
    GradientType g;
    g.Fill(0.0);
    for (int i=0; i<numB0; i++)
        pointshell.push_back(g);

    for(int i=0; i<NPoints; i++)
    {
        g[2] = sin(theta(i));
        if (g[2]<0)
            continue;
        g[0] = cos(theta(i)) * cos(phi(i));
        g[1] = cos(theta(i)) * sin(phi(i));
        pointshell.push_back(g);
    }

    return pointshell;
}

template<int ndirs>
std::vector<itk::Vector<double,3> > QmitkFiberBasedSoftwarePhantomView::MakeGradientList()
{
    std::vector<itk::Vector<double,3> > retval;
    vnl_matrix_fixed<double, 3, ndirs>* U =
            itk::PointShell<ndirs, vnl_matrix_fixed<double, 3, ndirs> >::DistributePointShell();


    // Add 0 vector for B0
    int numB0 = ndirs/10;
    if (numB0==0)
        numB0=1;
    itk::Vector<double,3> v;
    v.Fill(0.0);
    for (int i=0; i<numB0; i++)
    {
        retval.push_back(v);
    }

    for(int i=0; i<ndirs;i++)
    {
        itk::Vector<double,3> v;
        v[0] = U->get(0,i); v[1] = U->get(1,i); v[2] = U->get(2,i);
        retval.push_back(v);
    }

    return retval;
}

void QmitkFiberBasedSoftwarePhantomView::OnAddBundle()
{
    if (m_SelectedImage.IsNull())
        return;

    mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedImage);

    mitk::FiberBundleX::Pointer bundle = mitk::FiberBundleX::New();
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( bundle );
    QString name = QString("Bundle%1").arg(children->size());
    node->SetName(name.toStdString());
    GetDataStorage()->Add(node, m_SelectedImage);
}

void QmitkFiberBasedSoftwarePhantomView::OnDrawCircle()
{
    if (m_SelectedBundle.IsNull())
        return;
    mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedBundle);

    mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( figure );

    QString name = QString("Fiducial%1").arg(children->size());
    node->SetName(name.toStdString());
    GetDataStorage()->Add(node, m_SelectedBundle);

    mitk::PlanarFigureInteractor::Pointer figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());
    if(figureInteractor.IsNull())
        figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", node);
    mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);

}

void QmitkFiberBasedSoftwarePhantomView::GenerateFibers()
{
    if (m_SelectedBundle.IsNull())
        return;

    vector< vector< mitk::PlanarCircle::Pointer > > fiducials;
    for (int i=0; i<m_SelectedBundles.size(); i++)
    {
        mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedBundles.at(i));
        vector< mitk::PlanarCircle::Pointer > fib;
        for( mitk::DataStorage::SetOfObjects::const_iterator it = children->begin(); it != children->end(); ++it )
        {
            mitk::DataNode::Pointer node = *it;

            if ( node.IsNotNull() && dynamic_cast<mitk::PlanarCircle*>(node->GetData()) )
                fib.push_back(dynamic_cast<mitk::PlanarCircle*>(node->GetData()));
        }
        if (fib.size()>1)
            fiducials.push_back(fib);
        if (fib.size()<3)
        {
            QMessageBox::information(0, "Fiber generation not possible:", "At least 3 fiducials per bundle needed!");
            return;
        }
    }

    itk::FibersFromPointsFilter::Pointer filter = itk::FibersFromPointsFilter::New();
    filter->SetFiducials(fiducials);
    filter->SetDensity(m_Controls->m_FiberDensityBox->value());
    filter->Update();
    vector< mitk::FiberBundleX::Pointer > fiberBundles = filter->GetFiberBundles();

    mitk::FiberBundleX::Pointer newBundle = mitk::FiberBundleX::New();
    for (int i=0; i<fiberBundles.size(); i++)
    {
        m_SelectedBundles.at(i)->SetData( fiberBundles.at(i) );
        m_SelectedBundles.at(i)->SetVisibility(false);

        newBundle = newBundle->AddBundle(dynamic_cast<mitk::FiberBundleX*>(fiberBundles.at(i).GetPointer()));
    }

    if (newBundle->GetNumFibers()<=0)
    {
        QMessageBox::warning(0, "Error:", "Generated fiber bundle contains no fibers!");
        return;
    }

    mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
    fbNode->SetData(newBundle);
    fbNode->SetName("Synthetic_Bundle");
    GetDataStorage()->Add(fbNode);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberBasedSoftwarePhantomView::GenerateImage()
{
    if (m_SelectedBundle.IsNull())
        return;

    mitk::FiberBundleX::Pointer fiberBundle = dynamic_cast<mitk::FiberBundleX*>(m_SelectedBundle->GetData());
    if (fiberBundle->GetNumFibers()<=0)
    {
        QMessageBox::information(0, "Image generation not possible:", "Generated fiber bundle contains no fibers!");
        return;
    }

    typedef itk::TractsToDWIImageFilter FilterType;
    FilterType::GradientListType gradientList = GenerateHalfShell(m_Controls->m_NumGradientsBox->value());;
    double bVal = m_Controls->m_TensorsToDWIBValueEdit->value();
    itk::ImageRegion<3> imageRegion;
    imageRegion.SetSize(0, m_Controls->m_SizeX->value());
    imageRegion.SetSize(1, m_Controls->m_SizeY->value());
    imageRegion.SetSize(2, m_Controls->m_SizeZ->value());
    mitk::Vector3D spacing;
    spacing[0] = m_Controls->m_SpacingX->value();
    spacing[1] = m_Controls->m_SpacingY->value();
    spacing[2] = m_Controls->m_SpacingZ->value();

    FilterType::Pointer filter = FilterType::New();

    filter->SetGradientList(gradientList);
    filter->SetBValue(bVal);
    filter->SetSNR(m_Controls->m_NoiseLevel->value());
    filter->SetImageRegion(imageRegion);
    filter->SetSpacing(spacing);
    filter->SetGreyMatterAdc(m_Controls->m_GmAdc->value());
    filter->SetFiberBundle(fiberBundle);
    filter->SetMaxFA(m_Controls->m_MaxFaBox->value());
    filter->Update();

    mitk::DiffusionImage<short>::Pointer image = mitk::DiffusionImage<short>::New();
    image->SetVectorImage( filter->GetOutput() );
    image->SetB_Value(bVal);
    image->SetDirections(gradientList);
    image->InitializeFromVectorImage();
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( image );
    node->SetName(m_Controls->m_ImageName->text().toStdString());
    GetDataStorage()->Add(node, m_SelectedBundle);

    mitk::BaseData::Pointer basedata = node->GetData();
    if (basedata.IsNotNull())
    {
        mitk::RenderingManager::GetInstance()->InitializeViews(
                    basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void QmitkFiberBasedSoftwarePhantomView::UpdateGui()
{
    if (m_SelectedImage.IsNotNull())
        m_Controls->m_FiberButton->setEnabled(true);
    else
        m_Controls->m_FiberButton->setEnabled(false);

    if (m_SelectedBundle.IsNotNull())
    {
        m_Controls->m_GenerateFibersButton->setEnabled(true);
        m_Controls->m_CircleButton->setEnabled(true);
        m_Controls->m_GenerateImageButton->setEnabled(true);
    }
    else
    {
        m_Controls->m_GenerateFibersButton->setEnabled(false);
        m_Controls->m_CircleButton->setEnabled(false);
        m_Controls->m_GenerateImageButton->setEnabled(false);
    }
}

void QmitkFiberBasedSoftwarePhantomView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;
}

void QmitkFiberBasedSoftwarePhantomView::StdMultiWidgetNotAvailable()
{
    m_MultiWidget = NULL;
}

void QmitkFiberBasedSoftwarePhantomView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    m_SelectedBundles.clear();
    m_SelectedBundle = NULL;
    m_SelectedImage = NULL;

    // iterate all selected objects, adjust warning visibility
    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;

        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            m_SelectedImage = node;
        }
        else if ( node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
        {
            m_SelectedBundle = node;
            m_SelectedBundles.push_back(node);
        }
    }
    UpdateGui();
}
