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
#include "QmitkDwiSoftwarePhantomView.h"

// MITK
#include <mitkImage.h>
#include <mitkDiffusionImage.h>
#include <mitkImageToItk.h>
#include <itkDwiPhantomGenerationFilter.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>


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
    }
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

void QmitkDwiSoftwarePhantomView::GeneratePhantom()
{
    typedef itk::DwiPhantomGenerationFilter< short > FilterType;
    FilterType::GradientListType gradientList;

    m_SignalRegions.clear();
    for (int i=0; i<m_SignalRegionNodes.size(); i++)
    {
        mitk::Image::Pointer mitkBinaryImg = dynamic_cast<mitk::Image*>(m_SignalRegionNodes.at(i)->GetData());
        ItkUcharImgType::Pointer signalRegion = ItkUcharImgType::New();
        mitk::CastToItkImage<ItkUcharImgType>(mitkBinaryImg, signalRegion);
        m_SignalRegions.push_back(signalRegion);
    }

    switch(m_Controls->m_TensorsToDWINumDirsSelect->currentIndex())
    {
    case 0:
        gradientList = MakeGradientList<12>();
        break;
    case 1:
        gradientList = MakeGradientList<42>();
        break;
    case 2:
        gradientList = MakeGradientList<92>();
        break;
    case 3:
        gradientList = MakeGradientList<162>();
        break;
    case 4:
        gradientList = MakeGradientList<252>();
        break;
    case 5:
        gradientList = MakeGradientList<362>();
        break;
    case 6:
        gradientList = MakeGradientList<492>();
        break;
    case 7:
        gradientList = MakeGradientList<642>();
        break;
    case 8:
        gradientList = MakeGradientList<812>();
        break;
    case 9:
        gradientList = MakeGradientList<1002>();
        break;
    default:
        gradientList = MakeGradientList<92>();
    }
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
    filter->SetSignalScale(m_Controls->m_SignalScale->value());
    filter->SetImageRegion(imageRegion);
    filter->SetSpacing(spacing);
    filter->SetSignalRegions(m_SignalRegions);

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
    filter->Update();

    mitk::DiffusionImage<short>::Pointer image = mitk::DiffusionImage<short>::New();
    image->SetVectorImage( filter->GetOutput() );
    image->SetB_Value(bVal);
    image->SetDirections(gradientList);
    image->InitializeFromVectorImage();
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( image );
    node->SetName(m_Controls->m_ImageName->text().toStdString());
    GetDataStorage()->Add(node);
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
    if ( !this->IsVisible() )
    {
        // do nothing if nobody wants to see me :-(
        return;
    }

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
