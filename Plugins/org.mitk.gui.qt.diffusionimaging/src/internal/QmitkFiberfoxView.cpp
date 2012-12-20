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

//misc
#define _USE_MATH_DEFINES
#include <math.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkFiberfoxView.h"

// MITK
#include <mitkImage.h>
#include <mitkDiffusionImage.h>
#include <mitkImageToItk.h>
#include <itkDwiPhantomGenerationFilter.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkDataStorage.h>
#include <itkFibersFromPlanarFiguresFilter.h>
#include <itkTractsToDWIImageFilter.h>
#include <mitkTensorImage.h>
#include <mitkILinkedRenderWindowPart.h>
#include <mitkGlobalInteraction.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkImageGenerator.h>
#include <mitkNodePredicateDataType.h>
#include <mitkTensorModel.h>
#include <mitkBallModel.h>
#include <mitkStickModel.h>
#include <mitkRicianNoiseModel.h>
#include <mitkGibbsRingingArtifact.h>
#include <mitkT2SmearingArtifact.h>

#include <QMessageBox>

#define _USE_MATH_DEFINES
#include <math.h>

const std::string QmitkFiberfoxView::VIEW_ID = "org.mitk.views.fiberfoxview";

QmitkFiberfoxView::QmitkFiberfoxView()
    : QmitkAbstractView()
    , m_Controls( 0 )
    , m_SelectedImage( NULL )
    , m_SelectedBundle( NULL )
{

}

// Destructor
QmitkFiberfoxView::~QmitkFiberfoxView()
{

}

void QmitkFiberfoxView::CreateQtPartControl( QWidget *parent )
{
    // build up qt view, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkFiberfoxViewControls;
        m_Controls->setupUi( parent );

        m_Controls->m_VarianceBox->setVisible(false);
        m_Controls->m_GeometryMessage->setVisible(false);
        m_Controls->m_DiffusionPropsMessage->setVisible(false);
        m_Controls->m_T2bluringParamFrame->setVisible(false);
        m_Controls->m_KspaceParamFrame->setVisible(false);

        connect((QObject*) m_Controls->m_GenerateImageButton, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateImage()));
        connect((QObject*) m_Controls->m_GenerateFibersButton, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateFibers()));
        connect((QObject*) m_Controls->m_CircleButton, SIGNAL(clicked()), (QObject*) this, SLOT(OnDrawROI()));
        connect((QObject*) m_Controls->m_FlipButton, SIGNAL(clicked()), (QObject*) this, SLOT(OnFlipButton()));
        connect((QObject*) m_Controls->m_JoinBundlesButton, SIGNAL(clicked()), (QObject*) this, SLOT(JoinBundles()));
        connect((QObject*) m_Controls->m_VarianceBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnVarianceChanged(double)));
        connect((QObject*) m_Controls->m_DistributionBox, SIGNAL(currentIndexChanged(int)), (QObject*) this, SLOT(OnDistributionChanged(int)));
        connect((QObject*) m_Controls->m_FiberDensityBox, SIGNAL(valueChanged(int)), (QObject*) this, SLOT(OnFiberDensityChanged(int)));
        connect((QObject*) m_Controls->m_FiberSamplingBox, SIGNAL(valueChanged(int)), (QObject*) this, SLOT(OnFiberSamplingChanged(int)));
        connect((QObject*) m_Controls->m_TensionBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnTensionChanged(double)));
        connect((QObject*) m_Controls->m_ContinuityBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnContinuityChanged(double)));
        connect((QObject*) m_Controls->m_BiasBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnBiasChanged(double)));
        connect((QObject*) m_Controls->m_AddT2Smearing, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddT2Smearing(int)));
        connect((QObject*) m_Controls->m_AddGibbsRinging, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnAddGibbsRinging(int)));
        connect((QObject*) m_Controls->m_ConstantRadiusBox, SIGNAL(stateChanged(int)), (QObject*) this, SLOT(OnConstantRadius(int)));
        connect((QObject*) m_Controls->m_CopyBundlesButton, SIGNAL(clicked()), (QObject*) this, SLOT(CopyBundles()));
        connect((QObject*) m_Controls->m_TransformBundlesButton, SIGNAL(clicked()), (QObject*) this, SLOT(TransformBundles()));
    }
}

void QmitkFiberfoxView::OnConstantRadius(int value)
{
    if (value>0 && m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnAddT2Smearing(int value)
{
    if (value>0)
        m_Controls->m_T2bluringParamFrame->setVisible(true);
    else
        m_Controls->m_T2bluringParamFrame->setVisible(false);
}

void QmitkFiberfoxView::OnAddGibbsRinging(int value)
{
    if (value>0)
        m_Controls->m_KspaceParamFrame->setVisible(true);
    else
        m_Controls->m_KspaceParamFrame->setVisible(false);
}

void QmitkFiberfoxView::OnDistributionChanged(int value)
{
    if (value==1)
        m_Controls->m_VarianceBox->setVisible(true);
    else
        m_Controls->m_VarianceBox->setVisible(false);

    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnVarianceChanged(double value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnFiberDensityChanged(int value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnFiberSamplingChanged(int value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnTensionChanged(double value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnContinuityChanged(double value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnBiasChanged(double value)
{
    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::OnFlipButton()
{
    if (m_SelectedFiducial.IsNull())
        return;

    std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it = m_DataNodeToPlanarFigureData.find(m_SelectedFiducial.GetPointer());
    if( it != m_DataNodeToPlanarFigureData.end() )
    {
        QmitkPlanarFigureData& data = it->second;
        data.m_Flipped += 1;
        data.m_Flipped %= 2;
    }

    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

QmitkFiberfoxView::GradientListType QmitkFiberfoxView::GenerateHalfShell(int NPoints)
{
    NPoints *= 2;
    GradientListType pointshell;

    int numB0 = NPoints/10;
    if (numB0==0)
        numB0=1;
    GradientType g;
    g.Fill(0.0);
    for (int i=0; i<numB0; i++)
        pointshell.push_back(g);

    if (NPoints==0)
        return pointshell;

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
std::vector<itk::Vector<double,3> > QmitkFiberfoxView::MakeGradientList()
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

void QmitkFiberfoxView::OnAddBundle()
{
    if (m_SelectedImage.IsNull())
        return;

    mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedImage);

    mitk::FiberBundleX::Pointer bundle = mitk::FiberBundleX::New();
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( bundle );
    QString name = QString("Bundle_%1").arg(children->size());
    node->SetName(name.toStdString());
    m_SelectedBundle = node;
    m_SelectedBundles.push_back(node);
    UpdateGui();

    GetDataStorage()->Add(node, m_SelectedImage);
}

void QmitkFiberfoxView::OnDrawROI()
{
    if (m_SelectedBundle.IsNull())
        OnAddBundle();
    if (m_SelectedBundle.IsNull())
        return;

    mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedBundle);

    mitk::PlanarEllipse::Pointer figure = mitk::PlanarEllipse::New();

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( figure );


    QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
    for( int i=0; i<nodes.size(); i++)
        nodes.at(i)->SetSelected(false);

    m_SelectedFiducial = node;

    QString name = QString("Fiducial_%1").arg(children->size());
    node->SetName(name.toStdString());
    node->SetSelected(true);
    GetDataStorage()->Add(node, m_SelectedBundle);

    this->DisableCrosshairNavigation();

    mitk::PlanarFigureInteractor::Pointer figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());
    if(figureInteractor.IsNull())
        figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", node);
    mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);

    UpdateGui();
}

bool CompareLayer(mitk::DataNode::Pointer i,mitk::DataNode::Pointer j)
{
    int li = -1;
    i->GetPropertyValue("layer", li);
    int lj = -1;
    j->GetPropertyValue("layer", lj);
    return li<lj;
}

void QmitkFiberfoxView::GenerateFibers()
{
    if (m_SelectedBundles.empty())
    {
        if (m_SelectedFiducial.IsNull())
            return;

        mitk::DataStorage::SetOfObjects::ConstPointer parents = GetDataStorage()->GetSources(m_SelectedFiducial);
        for( mitk::DataStorage::SetOfObjects::const_iterator it = parents->begin(); it != parents->end(); ++it )
            if(dynamic_cast<mitk::FiberBundleX*>((*it)->GetData()))
                m_SelectedBundles.push_back(*it);

        if (m_SelectedBundles.empty())
            return;
    }

    vector< vector< mitk::PlanarEllipse::Pointer > > fiducials;
    vector< vector< unsigned int > > fliplist;
    for (int i=0; i<m_SelectedBundles.size(); i++)
    {
        mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedBundles.at(i));
        std::vector< mitk::DataNode::Pointer > childVector;
        for( mitk::DataStorage::SetOfObjects::const_iterator it = children->begin(); it != children->end(); ++it )
            childVector.push_back(*it);
        sort(childVector.begin(), childVector.end(), CompareLayer);

        vector< mitk::PlanarEllipse::Pointer > fib;
        vector< unsigned int > flip;
        float radius = 1;
        int count = 0;
        for( std::vector< mitk::DataNode::Pointer >::const_iterator it = childVector.begin(); it != childVector.end(); ++it )
        {
            mitk::DataNode::Pointer node = *it;

            if ( node.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(node->GetData()) )
            {
                mitk::PlanarEllipse* ellipse = dynamic_cast<mitk::PlanarEllipse*>(node->GetData());
                if (m_Controls->m_ConstantRadiusBox->isChecked())
                {
                    ellipse->SetTreatAsCircle(true);
                    mitk::Point2D c = ellipse->GetControlPoint(0);
                    mitk::Point2D p = ellipse->GetControlPoint(1);
                    mitk::Vector2D v = p-c;
                    if (count==0)
                        radius = v.GetVnlVector().magnitude();
                    else
                    {
                        v.Normalize();
                        v *= radius;
                        ellipse->SetControlPoint(1, c+v);
                    }
                }
                fib.push_back(ellipse);

                std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it = m_DataNodeToPlanarFigureData.find(node.GetPointer());
                if( it != m_DataNodeToPlanarFigureData.end() )
                {
                    QmitkPlanarFigureData& data = it->second;
                    flip.push_back(data.m_Flipped);
                }
                else
                    flip.push_back(0);
            }
            count++;
        }
        if (fib.size()>1)
        {
            fiducials.push_back(fib);
            fliplist.push_back(flip);
        }
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        if (fib.size()<3)
            return;
    }

    itk::FibersFromPlanarFiguresFilter::Pointer filter = itk::FibersFromPlanarFiguresFilter::New();
    filter->SetFiducials(fiducials);
    filter->SetFlipList(fliplist);

    switch(m_Controls->m_DistributionBox->currentIndex()){
    case 0:
        filter->SetFiberDistribution(itk::FibersFromPlanarFiguresFilter::DISTRIBUTE_UNIFORM);
        break;
    case 1:
        filter->SetFiberDistribution(itk::FibersFromPlanarFiguresFilter::DISTRIBUTE_GAUSSIAN);
        filter->SetVariance(m_Controls->m_VarianceBox->value());
        break;
    }

    filter->SetDensity(m_Controls->m_FiberDensityBox->value());
    filter->SetTension(m_Controls->m_TensionBox->value());
    filter->SetContinuity(m_Controls->m_ContinuityBox->value());
    filter->SetBias(m_Controls->m_BiasBox->value());
    filter->SetFiberSampling(m_Controls->m_FiberSamplingBox->value());
    filter->Update();
    vector< mitk::FiberBundleX::Pointer > fiberBundles = filter->GetFiberBundles();

    for (int i=0; i<fiberBundles.size(); i++)
    {
        m_SelectedBundles.at(i)->SetData( fiberBundles.at(i) );
        if (fiberBundles.at(i)->GetNumFibers()>50000)
            m_SelectedBundles.at(i)->SetVisibility(false);
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::GenerateImage()
{
    itk::ImageRegion<3> imageRegion;
    imageRegion.SetSize(0, m_Controls->m_SizeX->value());
    imageRegion.SetSize(1, m_Controls->m_SizeY->value());
    imageRegion.SetSize(2, m_Controls->m_SizeZ->value());
    mitk::Vector3D spacing;
    spacing[0] = m_Controls->m_SpacingX->value();
    spacing[1] = m_Controls->m_SpacingY->value();
    spacing[2] = m_Controls->m_SpacingZ->value();

    mitk::Point3D                       origin; origin.Fill(0.0);
    itk::Matrix<double, 3, 3>           directionMatrix; directionMatrix.SetIdentity();

    if (m_SelectedBundle.IsNull())
    {
        mitk::Image::Pointer image = mitk::ImageGenerator::GenerateGradientImage<unsigned int>(
                    m_Controls->m_SizeX->value(),
                    m_Controls->m_SizeY->value(),
                    m_Controls->m_SizeZ->value(),
                    m_Controls->m_SpacingX->value(),
                    m_Controls->m_SpacingY->value(),
                    m_Controls->m_SpacingZ->value());

        mitk::DataNode::Pointer node = mitk::DataNode::New();
        node->SetData( image );
        node->SetName("Dummy");
        GetDataStorage()->Add(node);
        m_SelectedImage = node;

        mitk::BaseData::Pointer basedata = node->GetData();
        if (basedata.IsNotNull())
        {
            mitk::RenderingManager::GetInstance()->InitializeViews(
                        basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
        UpdateGui();

        return;
    }

    DiffusionSignalModel<double>::GradientListType gradientList;
    double bVal = 1000;
    if (m_SelectedDWI.IsNull())
    {
        gradientList = GenerateHalfShell(m_Controls->m_NumGradientsBox->value());;
        bVal = m_Controls->m_BvalueBox->value();
    }
    else
    {
        mitk::DiffusionImage<short>::Pointer dwi = dynamic_cast<mitk::DiffusionImage<short>*>(m_SelectedDWI->GetData());
        imageRegion = dwi->GetVectorImage()->GetLargestPossibleRegion();
        spacing = dwi->GetVectorImage()->GetSpacing();
        origin = dwi->GetVectorImage()->GetOrigin();
        directionMatrix = dwi->GetVectorImage()->GetDirection();
        bVal = dwi->GetB_Value();
        mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer dirs = dwi->GetDirections();
        for (int i=0; i<dirs->Size(); i++)
        {
            DiffusionSignalModel<double>::GradientType g;
            g[0] = dirs->at(i)[0];
            g[1] = dirs->at(i)[1];
            g[2] = dirs->at(i)[2];
            gradientList.push_back(g);
        }
    }

    // signal models
    mitk::TensorModel<double> extraAxonal;
    extraAxonal.SetGradientList(gradientList);
    extraAxonal.SetBvalue(bVal);
    extraAxonal.SetKernelFA(m_Controls->m_MaxFaBox->value());
    extraAxonal.SetSignalScale(m_Controls->m_FiberS0Box->value());
    extraAxonal.SetRelaxationT2(m_Controls->m_FiberRelaxationT2Box->value());
    //    mitk::StickModel<double> intraAxonal;
    //    intraAxonal.SetGradientList(gradientList);
    //    intraAxonal.SetDiffusivity(m_Controls->m_MaxFaBox->value());
    //    intraAxonal.SetSignalScale(m_Controls->m_FiberS0Box->value());
    //    intraAxonal.SetRelaxationT2(m_Controls->m_FiberRelaxationT2Box->value());

    mitk::BallModel<double> freeDiffusion;
    freeDiffusion.SetGradientList(gradientList);
    freeDiffusion.SetBvalue(bVal);
    freeDiffusion.SetDiffusivity(m_Controls->m_BallD->value());
    freeDiffusion.SetSignalScale(m_Controls->m_NonFiberS0Box->value());
    freeDiffusion.SetRelaxationT2(m_Controls->m_NonFiberRelaxationT2Box->value());
    itk::TractsToDWIImageFilter::DiffusionModelList modelList;
    itk::TractsToDWIImageFilter::KspaceArtifactList artifactList;

    // noise model
    double snr = m_Controls->m_NoiseLevel->value();
    double noiseVariance = 0;
    if (snr <= 0)
        snr = 0.0001;
    if (snr<=99)
    {
        noiseVariance = (double)m_Controls->m_FiberS0Box->value()/snr;
        noiseVariance *= noiseVariance;
    }
    mitk::RicianNoiseModel<double> noiseModel;
    noiseModel.SetNoiseVariance(noiseVariance);

    // artifact models
    mitk::GibbsRingingArtifact<double> gibbsModel;
    if (m_Controls->m_AddGibbsRinging->isChecked())
    {
        gibbsModel.SetKspaceCropping((double)m_Controls->m_KspaceUndersamplingBox->currentText().toInt());
        artifactList.push_back(&gibbsModel);
    }

    mitk::T2SmearingArtifact<double> t2Model;
    if (m_Controls->m_AddT2Smearing->isChecked())
    {
        t2Model.SetReadoutPulseLength(1);
        artifactList.push_back(&t2Model);
    }


    for (int i=0; i<m_SelectedBundles.size(); i++)
    {
        mitk::FiberBundleX::Pointer fiberBundle = dynamic_cast<mitk::FiberBundleX*>(m_SelectedBundles.at(i)->GetData());
        if (fiberBundle->GetNumFibers()<=0)
            continue;

        itk::TractsToDWIImageFilter::Pointer filter = itk::TractsToDWIImageFilter::New();
        filter->SetImageRegion(imageRegion);
        filter->SetSpacing(spacing);
        filter->SetOrigin(origin);
        filter->SetDirectionMatrix(directionMatrix);
        filter->SetFiberBundle(fiberBundle);
        modelList.push_back(&extraAxonal);
        //    modelList.push_back(&intraAxonal);
        filter->SetFiberModels(modelList);
        modelList.clear();
        modelList.push_back(&freeDiffusion);
        filter->SetNonFiberModels(modelList);
        filter->SetNoiseModel(&noiseModel);
        filter->SetKspaceArtifacts(artifactList);
        filter->SetVolumeAccuracy(m_Controls->m_VolumeAccuracyBox->value());
        filter->SetNumberOfRepetitions(m_Controls->m_RepetitionsBox->value());
        filter->SetEnforcePureFiberVoxels(m_Controls->m_EnforcePureFiberVoxelsBox->isChecked());

        if (m_TissueMask.IsNotNull())
        {
            ItkUcharImgType::Pointer mask = ItkUcharImgType::New();
            mitk::CastToItkImage<ItkUcharImgType>(m_TissueMask, mask);
            filter->SetTissueMask(mask);
        }
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

        if (m_Controls->m_KspaceImageBox->isChecked())
        {
            itk::Image<double, 3>::Pointer kspace = filter->GetKspaceImage();
            mitk::Image::Pointer image = mitk::Image::New();
            image->InitializeByItk(kspace.GetPointer());
            image->SetVolume(kspace->GetBufferPointer());

            mitk::DataNode::Pointer node = mitk::DataNode::New();
            node->SetData( image );
            node->SetName("k-space");
            node->SetBoolProperty("use color", false);
            GetDataStorage()->Add(node, m_SelectedBundle);
        }

        mitk::BaseData::Pointer basedata = node->GetData();
        if (basedata.IsNotNull())
        {
            mitk::RenderingManager::GetInstance()->InitializeViews(
                        basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
    }
}

void QmitkFiberfoxView::TransformBundles()
{
    if ( m_SelectedBundles.size()<1 ){
        QMessageBox::information( NULL, "Warning", "Select at least one fiber bundle!");
        MITK_WARN("QmitkFiberProcessingView") << "Select at least one fiber bundle!";
        return;
    }

    std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedBundles.begin();
    for (it; it!=m_SelectedBundles.end(); ++it)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
        fib->RotateAroundAxis(m_Controls->m_XrotBox->value(), m_Controls->m_YrotBox->value(), m_Controls->m_ZrotBox->value());
        fib->TranslateFibers(m_Controls->m_XtransBox->value(), m_Controls->m_YtransBox->value(), m_Controls->m_ZtransBox->value());
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::CopyBundles()
{
    if ( m_SelectedBundles.size()<1 ){
        QMessageBox::information( NULL, "Warning", "Select at least one fiber bundle!");
        MITK_WARN("QmitkFiberProcessingView") << "Select at least one fiber bundle!";
        return;
    }

    std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedBundles.begin();
    for (it; it!=m_SelectedBundles.end(); ++it)
    {
        mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
        mitk::FiberBundleX::Pointer newBundle = fib->GetDeepCopy();
        QString name((*it)->GetName().c_str());
        name += "_copy";

        mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
        fbNode->SetData(newBundle);
        fbNode->SetName(name.toStdString());
        fbNode->SetVisibility(true);
        GetDataStorage()->Add(fbNode);
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::JoinBundles()
{
    if ( m_SelectedBundles.size()<2 ){
        QMessageBox::information( NULL, "Warning", "Select at least two fiber bundles!");
        MITK_WARN("QmitkFiberProcessingView") << "Select at least two fiber bundles!";
        return;
    }

    std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedBundles.begin();
    mitk::FiberBundleX::Pointer newBundle = dynamic_cast<mitk::FiberBundleX*>((*it)->GetData());
    QString name("");
    name += QString((*it)->GetName().c_str());
    ++it;
    for (it; it!=m_SelectedBundles.end(); ++it)
    {
        newBundle = newBundle->AddBundle(dynamic_cast<mitk::FiberBundleX*>((*it)->GetData()));
        name += "+"+QString((*it)->GetName().c_str());
    }

    mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
    fbNode->SetData(newBundle);
    fbNode->SetName(name.toStdString());
    fbNode->SetVisibility(true);
    GetDataStorage()->Add(fbNode);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberfoxView::UpdateGui()
{
    m_Controls->m_FiberBundleLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_GeometryFrame->setEnabled(true);
    m_Controls->m_GeometryMessage->setVisible(false);
    m_Controls->m_DiffusionPropsMessage->setVisible(false);
    m_Controls->m_FiberGenMessage->setVisible(true);

    m_Controls->m_TransformBundlesButton->setEnabled(false);
    m_Controls->m_CopyBundlesButton->setEnabled(false);
    m_Controls->m_GenerateFibersButton->setEnabled(false);
    m_Controls->m_FlipButton->setEnabled(false);
    m_Controls->m_CircleButton->setEnabled(false);
    m_Controls->m_BvalueBox->setEnabled(true);
    m_Controls->m_NumGradientsBox->setEnabled(true);
    m_Controls->m_JoinBundlesButton->setEnabled(false);

    if (m_SelectedFiducial.IsNotNull())
        m_Controls->m_FlipButton->setEnabled(true);

    if (m_SelectedImage.IsNotNull() || m_SelectedBundle.IsNotNull())
    {
        m_Controls->m_CircleButton->setEnabled(true);
        m_Controls->m_FiberGenMessage->setVisible(false);
    }

    if (m_TissueMask.IsNotNull())
    {
        m_Controls->m_GeometryMessage->setVisible(true);
        m_Controls->m_GeometryFrame->setEnabled(false);
    }

    if (m_SelectedDWI.IsNotNull())
    {
        m_Controls->m_DiffusionPropsMessage->setVisible(true);
        m_Controls->m_BvalueBox->setEnabled(false);
        m_Controls->m_NumGradientsBox->setEnabled(false);
        m_Controls->m_GeometryMessage->setVisible(true);
        m_Controls->m_GeometryFrame->setEnabled(false);
    }

    if (m_SelectedBundle.IsNotNull())
    {
        m_Controls->m_TransformBundlesButton->setEnabled(true);
        m_Controls->m_CopyBundlesButton->setEnabled(true);
        m_Controls->m_GenerateFibersButton->setEnabled(true);
        m_Controls->m_FiberBundleLabel->setText(m_SelectedBundle->GetName().c_str());

        if (m_SelectedBundles.size()>1)
            m_Controls->m_JoinBundlesButton->setEnabled(true);
    }
}

void QmitkFiberfoxView::OnSelectionChanged( berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes )
{
    m_SelectedFiducial = NULL;
    m_TissueMask = NULL;
    m_SelectedBundles.clear();
    m_SelectedBundle = NULL;
    m_SelectedImage = NULL;
    m_SelectedDWI = NULL;
    m_Controls->m_TissueMaskLabel->setText("<font color='grey'>optional</font>");

    // iterate all selected objects, adjust warning visibility
    for( int i=0; i<nodes.size(); i++)
    {
        mitk::DataNode::Pointer node = nodes.at(i);

        if ( node.IsNotNull() && dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData()) )
        {
            m_SelectedDWI = node;
            m_SelectedImage = node;
        }
        else if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            m_SelectedImage = node;
            bool isBinary = false;
            node->GetPropertyValue<bool>("binary", isBinary);
            if (isBinary)
            {
                m_TissueMask = dynamic_cast<mitk::Image*>(node->GetData());
                m_Controls->m_TissueMaskLabel->setText(node->GetName().c_str());
            }
        }
        else if ( node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
        {
            if (m_Controls->m_RealTimeFibers->isChecked() && node!=m_SelectedBundle)
            {
                m_SelectedBundle = node;
                m_SelectedBundles.push_back(node);
                mitk::FiberBundleX::Pointer newFib = dynamic_cast<mitk::FiberBundleX*>(node->GetData());
                if (newFib->GetNumFibers()!=m_Controls->m_FiberDensityBox->value())
                    GenerateFibers();
            }
            else
            {
                m_SelectedBundle = node;
                m_SelectedBundles.push_back(node);
            }
        }
        else if ( node.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(node->GetData()) )
        {
            m_SelectedFiducial = node;
            m_SelectedBundles.clear();
            mitk::DataStorage::SetOfObjects::ConstPointer parents = GetDataStorage()->GetSources(node);
            for( mitk::DataStorage::SetOfObjects::const_iterator it = parents->begin(); it != parents->end(); ++it )
            {
                mitk::DataNode::Pointer pNode = *it;
                if ( pNode.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(pNode->GetData()) )
                {
                    m_SelectedBundle = pNode;
                    m_SelectedBundles.push_back(pNode);
                }
            }
        }
    }
    UpdateGui();
}


void QmitkFiberfoxView::EnableCrosshairNavigation()
{
    MITK_DEBUG << "EnableCrosshairNavigation";

    // enable the crosshair navigation
    if (mitk::ILinkedRenderWindowPart* linkedRenderWindow =
            dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart()))
    {
        MITK_DEBUG << "enabling linked navigation";
        linkedRenderWindow->EnableLinkedNavigation(true);
        //        linkedRenderWindow->EnableSlicingPlanes(true);
    }

    if (m_Controls->m_RealTimeFibers->isChecked())
        GenerateFibers();
}

void QmitkFiberfoxView::DisableCrosshairNavigation()
{
    MITK_DEBUG << "DisableCrosshairNavigation";

    // disable the crosshair navigation during the drawing
    if (mitk::ILinkedRenderWindowPart* linkedRenderWindow =
            dynamic_cast<mitk::ILinkedRenderWindowPart*>(this->GetRenderWindowPart()))
    {
        MITK_DEBUG << "disabling linked navigation";
        linkedRenderWindow->EnableLinkedNavigation(false);
        //        linkedRenderWindow->EnableSlicingPlanes(false);
    }
}

void QmitkFiberfoxView::NodeRemoved(const mitk::DataNode* node)
{
    if (node == m_SelectedImage)
        m_SelectedImage = NULL;
    if (node == m_SelectedBundle)
        m_SelectedBundle = NULL;

    mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
    std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it = m_DataNodeToPlanarFigureData.find(nonConstNode);

    if( it != m_DataNodeToPlanarFigureData.end() )
    {
        QmitkPlanarFigureData& data = it->second;

        // remove observers
        data.m_Figure->RemoveObserver( data.m_EndPlacementObserverTag );
        data.m_Figure->RemoveObserver( data.m_SelectObserverTag );
        data.m_Figure->RemoveObserver( data.m_StartInteractionObserverTag );
        data.m_Figure->RemoveObserver( data.m_EndInteractionObserverTag );

        m_DataNodeToPlanarFigureData.erase( it );
    }
}

void QmitkFiberfoxView::NodeAdded( const mitk::DataNode* node )
{
    // add observer for selection in renderwindow
    mitk::PlanarFigure* figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
    bool isPositionMarker (false);
    node->GetBoolProperty("isContourMarker", isPositionMarker);
    if( figure && !isPositionMarker )
    {
        MITK_DEBUG << "figure added. will add interactor if needed.";
        mitk::PlanarFigureInteractor::Pointer figureInteractor
                = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetInteractor());

        mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>( node );
        if(figureInteractor.IsNull())
        {
            figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", nonConstNode);
        }
        else
        {
            // just to be sure that the interactor is not added twice
            mitk::GlobalInteraction::GetInstance()->RemoveInteractor(figureInteractor);
        }

        MITK_DEBUG << "adding interactor to globalinteraction";
        mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);

        MITK_DEBUG << "will now add observers for planarfigure";
        QmitkPlanarFigureData data;
        data.m_Figure = figure;

        //        // add observer for event when figure has been placed
        typedef itk::SimpleMemberCommand< QmitkFiberfoxView > SimpleCommandType;
        //        SimpleCommandType::Pointer initializationCommand = SimpleCommandType::New();
        //        initializationCommand->SetCallbackFunction( this, &QmitkFiberfoxView::PlanarFigureInitialized );
        //        data.m_EndPlacementObserverTag = figure->AddObserver( mitk::EndPlacementPlanarFigureEvent(), initializationCommand );

        // add observer for event when figure is picked (selected)
        typedef itk::MemberCommand< QmitkFiberfoxView > MemberCommandType;
        MemberCommandType::Pointer selectCommand = MemberCommandType::New();
        selectCommand->SetCallbackFunction( this, &QmitkFiberfoxView::PlanarFigureSelected );
        data.m_SelectObserverTag = figure->AddObserver( mitk::SelectPlanarFigureEvent(), selectCommand );

        // add observer for event when interaction with figure starts
        SimpleCommandType::Pointer startInteractionCommand = SimpleCommandType::New();
        startInteractionCommand->SetCallbackFunction( this, &QmitkFiberfoxView::DisableCrosshairNavigation);
        data.m_StartInteractionObserverTag = figure->AddObserver( mitk::StartInteractionPlanarFigureEvent(), startInteractionCommand );

        // add observer for event when interaction with figure starts
        SimpleCommandType::Pointer endInteractionCommand = SimpleCommandType::New();
        endInteractionCommand->SetCallbackFunction( this, &QmitkFiberfoxView::EnableCrosshairNavigation);
        data.m_EndInteractionObserverTag = figure->AddObserver( mitk::EndInteractionPlanarFigureEvent(), endInteractionCommand );

        m_DataNodeToPlanarFigureData[nonConstNode] = data;
    }
}

void QmitkFiberfoxView::PlanarFigureSelected( itk::Object* object, const itk::EventObject& )
{
    mitk::TNodePredicateDataType<mitk::PlanarFigure>::Pointer isPf = mitk::TNodePredicateDataType<mitk::PlanarFigure>::New();

    mitk::DataStorage::SetOfObjects::ConstPointer allPfs = this->GetDataStorage()->GetSubset( isPf );
    for ( mitk::DataStorage::SetOfObjects::const_iterator it = allPfs->begin(); it!=allPfs->end(); ++it)
    {
        mitk::DataNode* node = *it;

        if( node->GetData() == object )
        {
            node->SetSelected(true);
            m_SelectedFiducial = node;
        }
        else
            node->SetSelected(false);
    }
    UpdateGui();
    this->RequestRenderWindowUpdate();
}

void QmitkFiberfoxView::SetFocus()
{
    m_Controls->m_CircleButton->setFocus();
}
