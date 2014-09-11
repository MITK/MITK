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

// Qmitk
#include "QmitkDwiSoftwarePhantomView.h"

// MITK
#include <mitkDiffusionImage.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <itkDwiPhantomGenerationFilter.h>

#define _USE_MATH_DEFINES
#include <math.h>

const std::string QmitkDwiSoftwarePhantomView::VIEW_ID = "org.mitk.views.dwisoftwarephantomview";

QmitkDwiSoftwarePhantomView::QmitkDwiSoftwarePhantomView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
{

}

// Destructor
QmitkDwiSoftwarePhantomView::~QmitkDwiSoftwarePhantomView()
{

}


void QmitkDwiSoftwarePhantomView::CreateQtPartControl( QWidget *parent )
{
    // build up qt view, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkDwiSoftwarePhantomViewControls;
        m_Controls->setupUi( parent );
        m_Controls->m_SignalRegionBox->setVisible(false);

        connect((QObject*) m_Controls->m_GeneratePhantomButton, SIGNAL(clicked()), (QObject*) this, SLOT(GeneratePhantom()));
        connect((QObject*) m_Controls->m_SimulateBaseline, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnSimulateBaselineToggle(int)));
    }
}

QmitkDwiSoftwarePhantomView::GradientListType QmitkDwiSoftwarePhantomView::GenerateHalfShell(int NPoints)
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
std::vector<itk::Vector<double,3> > QmitkDwiSoftwarePhantomView::MakeGradientList()
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

void QmitkDwiSoftwarePhantomView::OnSimulateBaselineToggle(int state)
{
    if (state)
    {
        m_Controls->m_NoiseLabel->setText("Noise Variance:");
        m_Controls->m_NoiseLevel->setValue(1.0/(m_Controls->m_NoiseLevel->value()*m_Controls->m_NoiseLevel->value()));
        m_Controls->m_NoiseLevel->setToolTip("Variance of Rician noise.");
    }
    else
    {
        m_Controls->m_NoiseLabel->setText("SNR:");
        if (m_Controls->m_NoiseLevel->value()>0)
            m_Controls->m_NoiseLevel->setValue(1.0/(sqrt(m_Controls->m_NoiseLevel->value())));
        else
            m_Controls->m_NoiseLevel->setValue(0.0001);
        m_Controls->m_NoiseLevel->setToolTip("Signal to noise ratio (for values > 99, no noise at all is added to the image).");
    }
}

void QmitkDwiSoftwarePhantomView::GeneratePhantom()
{
    typedef itk::DwiPhantomGenerationFilter< short > FilterType;
    FilterType::GradientListType gradientList;

    m_SignalRegions.clear();
    for (int i=0; i<m_SignalRegionNodes.size(); i++)
    {
        mitk::Image::Pointer mitkBinaryImg = dynamic_cast<mitk::Image*>(m_SignalRegionNodes.at(i)->GetData());
        ItkUcharImgType::Pointer signalRegion = ItkUcharImgType::New();
        mitk::CastToItkImage(mitkBinaryImg, signalRegion);
        m_SignalRegions.push_back(signalRegion);
    }

    gradientList = GenerateHalfShell(m_Controls->m_NumGradientsBox->value());

    //    switch(m_Controls->m_NumGradientsBox->value())
    //    {
    //    case 0:
    //        gradientList = MakeGradientList<12>();
    //        break;
    //    case 1:
    //        gradientList = MakeGradientList<42>();
    //        break;
    //    case 2:
    //        gradientList = MakeGradientList<92>();
    //        break;
    //    case 3:
    //        gradientList = MakeGradientList<162>();
    //        break;
    //    case 4:
    //        gradientList = MakeGradientList<252>();
    //        break;
    //    case 5:
    //        gradientList = MakeGradientList<362>();
    //        break;
    //    case 6:
    //        gradientList = MakeGradientList<492>();
    //        break;
    //    case 7:
    //        gradientList = MakeGradientList<642>();
    //        break;
    //    case 8:
    //        gradientList = MakeGradientList<812>();
    //        break;
    //    case 9:
    //        gradientList = MakeGradientList<1002>();
    //        break;
    //    default:
    //        gradientList = MakeGradientList<92>();
    //    }

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
    filter->SetNoiseVariance(m_Controls->m_NoiseLevel->value());
    filter->SetImageRegion(imageRegion);
    filter->SetSpacing(spacing);
    filter->SetSignalRegions(m_SignalRegions);
    filter->SetGreyMatterAdc(m_Controls->m_GmAdc->value());

    std::vector< float >                            tensorFA;
    std::vector< float >                            tensorADC;
    std::vector< float >                            tensorWeight;
    std::vector< vnl_vector_fixed<double, 3> >      tensorDirection;
    for (int i=0; i<m_SpinFa.size(); i++)
    {
        tensorFA.push_back(m_SpinFa.at(i)->value());
        tensorADC.push_back(m_SpinAdc.at(i)->value());
        vnl_vector_fixed<double, 3> dir;
        dir[0] = m_SpinX.at(i)->value();
        dir[1] = m_SpinY.at(i)->value();
        dir[2] = m_SpinZ.at(i)->value();
        dir.normalize();
        tensorDirection.push_back(dir);
        tensorWeight.push_back(m_SpinWeight.at(i)->value());
    }
    filter->SetTensorFA(tensorFA);
    filter->SetTensorADC(tensorADC);
    filter->SetTensorWeight(tensorWeight);
    filter->SetTensorDirection(tensorDirection);
    if (!m_Controls->m_SimulateBaseline->isChecked())
        filter->SetSimulateBaseline(false);
    else
        filter->SetSimulateBaseline(true);
    filter->Update();

    mitk::DiffusionImage<short>::Pointer image = mitk::DiffusionImage<short>::New();
    image->SetVectorImage( filter->GetOutput() );
    image->SetReferenceBValue(bVal);
    image->SetDirections(gradientList);
    image->InitializeFromVectorImage();
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( image );
    node->SetName(m_Controls->m_ImageName->text().toStdString());
    GetDataStorage()->Add(node);

    mitk::BaseData::Pointer basedata = node->GetData();
    if (basedata.IsNotNull())
    {
        mitk::RenderingManager::GetInstance()->InitializeViews(
                    basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    if (m_Controls->m_OutputNumDirectionsBox->isChecked())
    {
        ItkUcharImgType::Pointer numDirImage = filter->GetNumDirectionsImage();
        mitk::Image::Pointer image = mitk::Image::New();
        image->InitializeByItk( numDirImage.GetPointer() );
        image->SetVolume( numDirImage->GetBufferPointer() );
        mitk::DataNode::Pointer node2 = mitk::DataNode::New();
        node2->SetData(image);
        QString name(m_Controls->m_ImageName->text());
        name += "_NumDirections";
        node2->SetName(name.toStdString().c_str());
        GetDataStorage()->Add(node2);
    }

    if (m_Controls->m_OutputSnrImageBox->isChecked())
    {
        ItkFloatImgType::Pointer snrImage = filter->GetSNRImage();
        mitk::Image::Pointer image = mitk::Image::New();
        image->InitializeByItk( snrImage.GetPointer() );
        image->SetVolume( snrImage->GetBufferPointer() );
        mitk::DataNode::Pointer node2 = mitk::DataNode::New();
        node2->SetData(image);
        QString name(m_Controls->m_ImageName->text());
        name += "_SNR";
        node2->SetName(name.toStdString().c_str());
        GetDataStorage()->Add(node2);
    }

    if (m_SignalRegionNodes.size()==0)
        return;

    if (m_Controls->m_OutputDirectionImagesBox->isChecked())
    {
        typedef FilterType::ItkDirectionImageContainer ItkDirectionImageContainer;
        ItkDirectionImageContainer::Pointer container = filter->GetDirectionImageContainer();
        for (int i=0; i<container->Size(); i++)
        {
            FilterType::ItkDirectionImage::Pointer itkImg = container->GetElement(i);
            mitk::Image::Pointer img = mitk::Image::New();
            img->InitializeByItk( itkImg.GetPointer() );
            img->SetVolume( itkImg->GetBufferPointer() );
            mitk::DataNode::Pointer node = mitk::DataNode::New();
            node->SetData(img);
            QString name(m_Controls->m_ImageName->text());
            name += "_Direction";
            name += QString::number(i+1);
            node->SetName(name.toStdString().c_str());
            GetDataStorage()->Add(node);
        }
    }

    if (m_Controls->m_OutputVectorFieldBox->isChecked())
    {
        mitk::BaseGeometry::Pointer geometry = image->GetGeometry();
        mitk::Vector3D outImageSpacing = geometry->GetSpacing();
        float minSpacing = 1;
        if(outImageSpacing[0]<outImageSpacing[1] && outImageSpacing[0]<outImageSpacing[2])
            minSpacing = outImageSpacing[0];
        else if (outImageSpacing[1] < outImageSpacing[2])
            minSpacing = outImageSpacing[1];
        else
            minSpacing = outImageSpacing[2];

        mitk::FiberBundleX::Pointer directions = filter->GetOutputFiberBundle();
        directions->SetGeometry(geometry);
        mitk::DataNode::Pointer node = mitk::DataNode::New();
        node->SetData(directions);
        QString name(m_Controls->m_ImageName->text());
        name += "_VectorField";
        node->SetName(name.toStdString().c_str());
        node->SetProperty("Fiber2DSliceThickness", mitk::FloatProperty::New(minSpacing));
        node->SetProperty("Fiber2DfadeEFX", mitk::BoolProperty::New(false));
        GetDataStorage()->Add(node);
    }
}

void QmitkDwiSoftwarePhantomView::UpdateGui()
{
    if (!m_SignalRegionNodes.empty())
    {
        m_Controls->m_SignalRegionBox->setVisible(true);
        m_Controls->m_Instruction->setVisible(false);
    }
    else
    {
        m_Controls->m_SignalRegionBox->setVisible(false);
        m_Controls->m_Instruction->setVisible(true);
    }

    QLayout* layout = m_Controls->m_SignalRegionBox->layout();

    for (int i=0; i<m_Labels.size(); i++)
    {
        delete m_Labels.at(i);
    }
    for (int i=0; i<m_SpinFa.size(); i++)
    {
        delete m_SpinFa.at(i);
        delete m_SpinAdc.at(i);
        delete m_SpinX.at(i);
        delete m_SpinY.at(i);
        delete m_SpinZ.at(i);
        delete m_SpinWeight.at(i);
    }
    m_Labels.clear();
    m_SpinFa.clear();
    m_SpinAdc.clear();
    m_SpinX.clear();
    m_SpinY.clear();
    m_SpinZ.clear();
    m_SpinWeight.clear();

    if (layout)
        delete layout;

    QGridLayout* newlayout = new QGridLayout();

    m_Controls->m_SignalRegionBox->setLayout(newlayout);

    if (!m_SignalRegionNodes.empty())
    {
        QLabel* label1 = new QLabel("Image");
        newlayout->addWidget(label1,0,0);
        m_Labels.push_back(label1);
        QLabel* label2 = new QLabel("FA");
        newlayout->addWidget(label2,0,1);
        m_Labels.push_back(label2);
        QLabel* label3 = new QLabel("ADC");
        newlayout->addWidget(label3,0,2);
        m_Labels.push_back(label3);
        QLabel* label4 = new QLabel("X");
        newlayout->addWidget(label4,0,03);
        m_Labels.push_back(label4);
        QLabel* label5 = new QLabel("Y");
        newlayout->addWidget(label5,0,4);
        m_Labels.push_back(label5);
        QLabel* label6 = new QLabel("Z");
        newlayout->addWidget(label6,0,5);
        m_Labels.push_back(label6);
        QLabel* label7 = new QLabel("Weight");
        newlayout->addWidget(label7,0,6);
        m_Labels.push_back(label7);
    }
    for (int i=0; i<m_SignalRegionNodes.size(); i++)
    {
        QLabel* label = new QLabel(m_SignalRegionNodes.at(i)->GetName().c_str());
        newlayout->addWidget(label,i+1,0);
        m_Labels.push_back(label);

        QDoubleSpinBox* spinFa = new QDoubleSpinBox();
        spinFa->setValue(0.7);
        spinFa->setMinimum(0);
        spinFa->setMaximum(1);
        spinFa->setSingleStep(0.1);
        newlayout->addWidget(spinFa,i+1,1);
        m_SpinFa.push_back(spinFa);

        QDoubleSpinBox* spinAdc = new QDoubleSpinBox();
        newlayout->addWidget(spinAdc,i+1,2);
        spinAdc->setMinimum(0);
        spinAdc->setMaximum(1);
        spinAdc->setSingleStep(0.001);
        spinAdc->setDecimals(3);
        spinAdc->setValue(0.001); ///// ???????????????????????????
        m_SpinAdc.push_back(spinAdc);

        QDoubleSpinBox* spinX = new QDoubleSpinBox();
        newlayout->addWidget(spinX,i+1,3);
        spinX->setValue(1);
        spinX->setMinimum(-1);
        spinX->setMaximum(1);
        spinX->setSingleStep(0.1);
        m_SpinX.push_back(spinX);

        QDoubleSpinBox* spinY = new QDoubleSpinBox();
        newlayout->addWidget(spinY,i+1,4);
        spinY->setMinimum(-1);
        spinY->setMaximum(1);
        spinY->setSingleStep(0.1);
        m_SpinY.push_back(spinY);

        QDoubleSpinBox* spinZ = new QDoubleSpinBox();
        newlayout->addWidget(spinZ,i+1,5);
        spinZ->setMinimum(-1);
        spinZ->setMaximum(1);
        spinZ->setSingleStep(0.1);
        m_SpinZ.push_back(spinZ);

        QDoubleSpinBox* spinWeight = new QDoubleSpinBox();
        newlayout->addWidget(spinWeight,i+1,6);
        spinWeight->setMinimum(0);
        spinWeight->setMaximum(1);
        spinWeight->setSingleStep(0.1);
        spinWeight->setValue(1.0);
        m_SpinWeight.push_back(spinWeight);
    }
}

void QmitkDwiSoftwarePhantomView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;
}

void QmitkDwiSoftwarePhantomView::StdMultiWidgetNotAvailable()
{
    m_MultiWidget = NULL;
}

void QmitkDwiSoftwarePhantomView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    m_SignalRegionNodes.clear();

    // iterate all selected objects, adjust warning visibility
    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;

        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            bool isBinary = false;
            node->GetPropertyValue<bool>("binary", isBinary);
            if (isBinary)
                m_SignalRegionNodes.push_back(node);
        }
    }
    UpdateGui();
}
