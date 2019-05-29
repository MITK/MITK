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

// Qmitk
#include "QmitkFiberGenerationView.h"

// MITK
#include <mitkImage.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkDataStorage.h>
#include <itkFibersFromPlanarFiguresFilter.h>
#include <itkTractsToDWIImageFilter.h>
#include <mitkTensorImage.h>
#include <mitkILinkedRenderWindowPart.h>
#include <mitkImageToItk.h>
#include <mitkImageCast.h>
#include <mitkImageGenerator.h>
#include <mitkNodePredicateDataType.h>
#include <itkScalableAffineTransform.h>
#include <mitkLevelWindowProperty.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateIsDWI.h>
#include <boost/property_tree/ptree.hpp>
#define RAPIDXML_NO_EXCEPTIONS
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <QFileDialog>
#include <QMessageBox>
#include "usModuleRegistry.h"
#include <mitkChiSquareNoiseModel.h>
#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>
#include <QScrollBar>
#include <itkInvertIntensityImageFilter.h>
#include <QDialogButtonBox>
#include <itkAdcImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <mitkITKImageImport.h>
#include <itkRandomPhantomFilter.h>

#include "mitkNodePredicateDataType.h"
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>

#define _USE_MATH_DEFINES
#include <math.h>


const std::string QmitkFiberGenerationView::VIEW_ID = "org.mitk.views.fibergenerationview";

QmitkFiberGenerationView::QmitkFiberGenerationView()
  : QmitkAbstractView()
  , m_Controls( 0 )
  , m_SelectedImageNode( nullptr )
{

}

// Destructor
QmitkFiberGenerationView::~QmitkFiberGenerationView()
{
}

void QmitkFiberGenerationView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkFiberGenerationViewControls;
    m_Controls->setupUi( parent );

    m_ParameterFile = QDir::currentPath()+"/param.ffp";

    connect(static_cast<QObject*>(m_Controls->m_RandomPhantomButton), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(RandomPhantom()));
    connect(static_cast<QObject*>(m_Controls->m_GenerateFibersButton), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(GenerateFibers()));
    connect(static_cast<QObject*>(m_Controls->m_CircleButton), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(OnDrawROI()));
    connect(static_cast<QObject*>(m_Controls->m_FlipButton), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(OnFlipButton()));
    connect(static_cast<QObject*>(m_Controls->m_JoinBundlesButton), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(JoinBundles()));
    connect(static_cast<QObject*>(m_Controls->m_VarianceBox), SIGNAL(valueChanged(double)), static_cast<QObject*>(this), SLOT(OnVarianceChanged(double)));
    connect(static_cast<QObject*>(m_Controls->m_DistributionBox), SIGNAL(currentIndexChanged(int)), static_cast<QObject*>(this), SLOT(OnDistributionChanged(int)));
    connect(static_cast<QObject*>(m_Controls->m_FiberDensityBox), SIGNAL(valueChanged(int)), static_cast<QObject*>(this), SLOT(OnFiberDensityChanged(int)));
    connect(static_cast<QObject*>(m_Controls->m_FiberSamplingBox), SIGNAL(valueChanged(double)), static_cast<QObject*>(this), SLOT(OnFiberSamplingChanged(double)));
    connect(static_cast<QObject*>(m_Controls->m_TensionBox), SIGNAL(valueChanged(double)), static_cast<QObject*>(this), SLOT(OnTensionChanged(double)));
    connect(static_cast<QObject*>(m_Controls->m_ContinuityBox), SIGNAL(valueChanged(double)), static_cast<QObject*>(this), SLOT(OnContinuityChanged(double)));
    connect(static_cast<QObject*>(m_Controls->m_BiasBox), SIGNAL(valueChanged(double)), static_cast<QObject*>(this), SLOT(OnBiasChanged(double)));

    connect(static_cast<QObject*>(m_Controls->m_ConstantRadiusBox), SIGNAL(stateChanged(int)), static_cast<QObject*>(this), SLOT(OnConstantRadius(int)));
    connect(static_cast<QObject*>(m_Controls->m_CopyBundlesButton), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(CopyBundles()));
    connect(static_cast<QObject*>(m_Controls->m_TransformBundlesButton), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(ApplyTransform()));
    connect(static_cast<QObject*>(m_Controls->m_AlignOnGrid), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(AlignOnGrid()));
    connect(static_cast<QObject*>(m_Controls->m_FidAxis1), SIGNAL(editingFinished()), static_cast<QObject*>(this), SLOT(UpdateFiducialPosition()));
    connect(static_cast<QObject*>(m_Controls->m_FidAxis2), SIGNAL(editingFinished()), static_cast<QObject*>(this), SLOT(UpdateFiducialPosition()));
    connect(static_cast<QObject*>(m_Controls->m_FidPosX), SIGNAL(editingFinished()), static_cast<QObject*>(this), SLOT(UpdateFiducialPosition()));
    connect(static_cast<QObject*>(m_Controls->m_FidPosY), SIGNAL(editingFinished()), static_cast<QObject*>(this), SLOT(UpdateFiducialPosition()));
    connect(static_cast<QObject*>(m_Controls->m_FidPosZ), SIGNAL(editingFinished()), static_cast<QObject*>(this), SLOT(UpdateFiducialPosition()));
    connect(static_cast<QObject*>(m_Controls->m_FidTwist), SIGNAL(editingFinished()), static_cast<QObject*>(this), SLOT(UpdateFiducialPosition()));

    connect(static_cast<QObject*>(m_Controls->m_AdvancedOptionsBox), SIGNAL( stateChanged(int)), static_cast<QObject*>(this), SLOT(ShowAdvancedOptions(int)));

    connect(static_cast<QObject*>(m_Controls->m_SaveParametersButton), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(SaveParameters()));
    connect(static_cast<QObject*>(m_Controls->m_LoadParametersButton), SIGNAL(clicked()), static_cast<QObject*>(this), SLOT(LoadParameters()));
    OnDistributionChanged(0);
  }
  UpdateGui();
}

void QmitkFiberGenerationView::RandomPhantom()
{
  itk::RandomPhantomFilter::Pointer filter = itk::RandomPhantomFilter::New();
  filter->SetNumTracts(m_Controls->m_NumBundlesBox->value());
  filter->SetMinStreamlineDensity(m_Controls->m_MinDensityBox->value());
  filter->SetMaxStreamlineDensity(m_Controls->m_MaxDensityBox->value());
  mitk::Vector3D vol;
  vol[0] = m_Controls->m_VolumeSizeX->value();
  vol[1] = m_Controls->m_VolumeSizeY->value();
  vol[2] = m_Controls->m_VolumeSizeZ->value();
  filter->SetVolumeSize(vol);
  filter->SetStepSizeMin(m_Controls->m_StepSizeMinBox->value());
  filter->SetStepSizeMax(m_Controls->m_StepSizeMaxBox->value());
  filter->SetCurvynessMin(m_Controls->m_CurvyMinBox->value());
  filter->SetCurvynessMax(m_Controls->m_CurvyMaxBox->value());
  filter->SetStartRadiusMin(m_Controls->m_SizeMinBox->value());
  filter->SetStartRadiusMax(m_Controls->m_SizeMaxBox->value());
  filter->SetMinTwist(m_Controls->m_MinTwistBox->value());
  filter->SetMaxTwist(m_Controls->m_MaxTwistBox->value());
  filter->Update();
  auto fibs = filter->GetFiberBundles();

  std::vector< mitk::DataNode::Pointer > fiber_nodes;
  int c = 1;
  for (auto fib : fibs)
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( fib );
    node->SetName("Bundle_" + boost::lexical_cast<std::string>(c));
    GetDataStorage()->Add(node);
    fiber_nodes.push_back(node);
    ++c;
  }

//  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  mitk::RenderingManager::GetInstance()->InitializeViews(GetDataStorage()->ComputeVisibleBoundingGeometry3D());
}

void QmitkFiberGenerationView::UpdateParametersFromGui()
{
  m_Parameters.ClearFiberParameters();
  m_Parameters.m_Misc.m_CheckAdvancedFiberOptionsBox = m_Controls->m_AdvancedOptionsBox->isChecked();

  switch(m_Controls->m_DistributionBox->currentIndex())
  {
  case 0:
    m_Parameters.m_FiberGen.m_Distribution = FiberGenerationParameters::DISTRIBUTE_UNIFORM;
    break;
  case 1:
    m_Parameters.m_FiberGen.m_Distribution = FiberGenerationParameters::DISTRIBUTE_GAUSSIAN;
    break;
  default:
    m_Parameters.m_FiberGen.m_Distribution = FiberGenerationParameters::DISTRIBUTE_UNIFORM;
  }
  m_Parameters.m_FiberGen.m_Variance = m_Controls->m_VarianceBox->value();
  m_Parameters.m_FiberGen.m_Density = m_Controls->m_FiberDensityBox->value();
  m_Parameters.m_FiberGen.m_Sampling = m_Controls->m_FiberSamplingBox->value();
  m_Parameters.m_FiberGen.m_Tension = m_Controls->m_TensionBox->value();
  m_Parameters.m_FiberGen.m_Continuity = m_Controls->m_ContinuityBox->value();
  m_Parameters.m_FiberGen.m_Bias = m_Controls->m_BiasBox->value();
  m_Parameters.m_FiberGen.m_Rotation[0] = m_Controls->m_XrotBox->value();
  m_Parameters.m_FiberGen.m_Rotation[1] = m_Controls->m_YrotBox->value();
  m_Parameters.m_FiberGen.m_Rotation[2] = m_Controls->m_ZrotBox->value();
  m_Parameters.m_FiberGen.m_Translation[0] = m_Controls->m_XtransBox->value();
  m_Parameters.m_FiberGen.m_Translation[1] = m_Controls->m_YtransBox->value();
  m_Parameters.m_FiberGen.m_Translation[2] = m_Controls->m_ZtransBox->value();
  m_Parameters.m_FiberGen.m_Scale[0] = m_Controls->m_XscaleBox->value();
  m_Parameters.m_FiberGen.m_Scale[1] = m_Controls->m_YscaleBox->value();
  m_Parameters.m_FiberGen.m_Scale[2] = m_Controls->m_ZscaleBox->value();

  m_Parameters.m_Misc.m_CheckRealTimeFibersBox = m_Controls->m_RealTimeFibers->isChecked();
  m_Parameters.m_Misc.m_CheckAdvancedFiberOptionsBox = m_Controls->m_AdvancedOptionsBox->isChecked();
  m_Parameters.m_Misc.m_CheckIncludeFiducialsBox = m_Controls->m_IncludeFiducials->isChecked();
  m_Parameters.m_Misc.m_CheckConstantRadiusBox = m_Controls->m_ConstantRadiusBox->isChecked();
}

void QmitkFiberGenerationView::SaveParameters(QString filename)
{
  UpdateParametersFromGui();
  m_Parameters.SaveParameters(filename.toStdString());
  m_ParameterFile = filename;
}

void QmitkFiberGenerationView::SaveParameters()
{
  QString filename = QFileDialog::getSaveFileName(
        0,
        tr("Save Parameters"),
        m_ParameterFile,
        tr("Fiberfox Parameters (*.ffp)") );

  SaveParameters(filename);
}

void QmitkFiberGenerationView::LoadParameters()
{
  QString filename = QFileDialog::getOpenFileName(0, tr("Load Parameters"), QString(itksys::SystemTools::GetFilenamePath(m_ParameterFile.toStdString()).c_str()), tr("Fiberfox Parameters (*.ffp)") );
  if(filename.isEmpty() || filename.isNull())
    return;

  m_ParameterFile = filename;
  m_Parameters.LoadParameters(filename.toStdString());

  if (m_Parameters.m_MissingTags.size()>0)
  {
    QString missing("Parameter file might be corrupted. The following parameters could not be read: ");
    missing += QString(m_Parameters.m_MissingTags.c_str());
    missing += "\nDefault values have been assigned to the missing parameters.";
    QMessageBox::information( nullptr, "Warning!", missing);
  }

  m_Controls->m_RealTimeFibers->setChecked(m_Parameters.m_Misc.m_CheckRealTimeFibersBox);
  m_Controls->m_AdvancedOptionsBox->setChecked(m_Parameters.m_Misc.m_CheckAdvancedFiberOptionsBox);
  m_Controls->m_IncludeFiducials->setChecked(m_Parameters.m_Misc.m_CheckIncludeFiducialsBox);
  m_Controls->m_ConstantRadiusBox->setChecked(m_Parameters.m_Misc.m_CheckConstantRadiusBox);

  m_Controls->m_DistributionBox->setCurrentIndex(m_Parameters.m_FiberGen.m_Distribution);
  m_Controls->m_VarianceBox->setValue(m_Parameters.m_FiberGen.m_Variance);
  m_Controls->m_FiberDensityBox->setValue(m_Parameters.m_FiberGen.m_Density);
  m_Controls->m_FiberSamplingBox->setValue(m_Parameters.m_FiberGen.m_Sampling);
  m_Controls->m_TensionBox->setValue(m_Parameters.m_FiberGen.m_Tension);
  m_Controls->m_ContinuityBox->setValue(m_Parameters.m_FiberGen.m_Continuity);
  m_Controls->m_BiasBox->setValue(m_Parameters.m_FiberGen.m_Bias);
  m_Controls->m_XrotBox->setValue(m_Parameters.m_FiberGen.m_Rotation[0]);
  m_Controls->m_YrotBox->setValue(m_Parameters.m_FiberGen.m_Rotation[1]);
  m_Controls->m_ZrotBox->setValue(m_Parameters.m_FiberGen.m_Rotation[2]);
  m_Controls->m_XtransBox->setValue(m_Parameters.m_FiberGen.m_Translation[0]);
  m_Controls->m_YtransBox->setValue(m_Parameters.m_FiberGen.m_Translation[1]);
  m_Controls->m_ZtransBox->setValue(m_Parameters.m_FiberGen.m_Translation[2]);
  m_Controls->m_XscaleBox->setValue(m_Parameters.m_FiberGen.m_Scale[0]);
  m_Controls->m_YscaleBox->setValue(m_Parameters.m_FiberGen.m_Scale[1]);
  m_Controls->m_ZscaleBox->setValue(m_Parameters.m_FiberGen.m_Scale[2]);
}

void QmitkFiberGenerationView::ShowAdvancedOptions(int state)
{
  if (state)
  {
    m_Controls->m_AdvancedFiberOptionsFrame->setVisible(true);
    m_Controls->m_AdvancedOptionsBox->setChecked(true);
  }
  else
  {
    m_Controls->m_AdvancedFiberOptionsFrame->setVisible(false);
    m_Controls->m_AdvancedOptionsBox->setChecked(false);
  }
}

void QmitkFiberGenerationView::OnConstantRadius(int value)
{
  if (value>0 && m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();
}

void QmitkFiberGenerationView::OnDistributionChanged(int value)
{
  if (value==1)
    m_Controls->m_VarianceBox->setVisible(true);
  else
    m_Controls->m_VarianceBox->setVisible(false);

  if (m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();
}

void QmitkFiberGenerationView::OnVarianceChanged(double)
{
  if (m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();
}

void QmitkFiberGenerationView::OnFiberDensityChanged(int)
{
  if (m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();
}

void QmitkFiberGenerationView::OnFiberSamplingChanged(double)
{
  if (m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();
}

void QmitkFiberGenerationView::OnTensionChanged(double)
{
  if (m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();
}

void QmitkFiberGenerationView::OnContinuityChanged(double)
{
  if (m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();
}

void QmitkFiberGenerationView::OnBiasChanged(double)
{
  if (m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();
}

void QmitkFiberGenerationView::UpdateFiducialPosition()
{
  if (m_SelectedFiducials.size()!=1)
    return;

  mitk::PlanarEllipse* pe = dynamic_cast<mitk::PlanarEllipse*>(m_SelectedFiducials.at(0)->GetData());
  mitk::BaseGeometry* geom = pe->GetGeometry();

  // translate
  mitk::Point3D origin;
  origin[0] = m_Controls->m_FidPosX->value();
  origin[1] = m_Controls->m_FidPosY->value();
  origin[2] = m_Controls->m_FidPosZ->value();

  // transform control point coordinate into geometry translation
  auto p0 = pe->GetControlPoint(0);
  auto p1 = pe->GetControlPoint(1);
  auto p2 = pe->GetControlPoint(2);
  auto p3 = pe->GetControlPoint(3);

  auto v1 = p1 - p0;
  auto v2 = p2 - p0;
  auto angle_deg = m_Controls->m_FidTwist->value();
  auto dot = std::cos(itk::Math::pi*angle_deg/180.0);

  vnl_matrix_fixed<double, 2, 2> tRot;
  tRot[0][0] = dot;
  tRot[1][1] = tRot[0][0];
  tRot[1][0] = sin(acos(tRot[0][0]));
  tRot[0][1] = -tRot[1][0];
  if (angle_deg<0)
    tRot = tRot.transpose();
  vnl_vector_fixed<double, 2> vt; vt[0]=1; vt[1]=0;
  vnl_vector_fixed<double, 2> v3 = tRot*vt;

  if (v1.GetNorm()<0.0001 || v2.GetNorm()<0.0001)
  {
    QMessageBox::information( nullptr, "", "Please select exactly one fiducial.");
    return;
  }

  v1.Normalize();
  v2.Normalize();

  p1 = p0 + v1*m_Controls->m_FidAxis1->value();
  p2 = p0 + v2*m_Controls->m_FidAxis2->value();
  p3 = p0 + mitk::Vector2D(v3);
  pe->SetControlPoint(1, p1);
  pe->SetControlPoint(2, p2);
  pe->SetControlPoint(3, p3);
  geom->SetOrigin(origin);

  pe->Modified();

  if (m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();

  UpdateGui();
}

void QmitkFiberGenerationView::AlignOnGrid()
{
  for (unsigned int i=0; i<m_SelectedFiducials.size(); i++)
  {
    mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(m_SelectedFiducials.at(i)->GetData());
    mitk::Point3D wc0 = pe->GetWorldControlPoint(0);

    mitk::DataStorage::SetOfObjects::ConstPointer parentFibs = GetDataStorage()->GetSources(m_SelectedFiducials.at(i));
    for( mitk::DataStorage::SetOfObjects::const_iterator it = parentFibs->begin(); it != parentFibs->end(); ++it )
    {
      mitk::DataNode::Pointer pFibNode = *it;
      if ( pFibNode.IsNotNull() && dynamic_cast<mitk::FiberBundle*>(pFibNode->GetData()) )
      {
        mitk::DataStorage::SetOfObjects::ConstPointer parentImgs = GetDataStorage()->GetSources(pFibNode);
        for( mitk::DataStorage::SetOfObjects::const_iterator it2 = parentImgs->begin(); it2 != parentImgs->end(); ++it2 )
        {
          mitk::DataNode::Pointer pImgNode = *it2;
          if ( pImgNode.IsNotNull() && dynamic_cast<mitk::Image*>(pImgNode->GetData()) )
          {
            mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(pImgNode->GetData());
            mitk::BaseGeometry::Pointer geom = img->GetGeometry();
            itk::Index<3> idx;
            geom->WorldToIndex(wc0, idx);

            mitk::Point3D cIdx; cIdx[0]=idx[0]; cIdx[1]=idx[1]; cIdx[2]=idx[2];
            mitk::Point3D world;
            geom->IndexToWorld(cIdx,world);

            mitk::Vector3D trans = world - wc0;
            pe->GetGeometry()->Translate(trans);

            break;
          }
        }
        break;
      }
    }
  }

  for(unsigned int i=0; i<m_SelectedBundles2.size(); i++ )
  {
    mitk::DataNode::Pointer fibNode = m_SelectedBundles2.at(i);

    mitk::DataStorage::SetOfObjects::ConstPointer sources = GetDataStorage()->GetSources(fibNode);
    for( mitk::DataStorage::SetOfObjects::const_iterator it = sources->begin(); it != sources->end(); ++it )
    {
      mitk::DataNode::Pointer imgNode = *it;
      if ( imgNode.IsNotNull() && dynamic_cast<mitk::Image*>(imgNode->GetData()) )
      {
        mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(fibNode);
        for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations->begin(); it2 != derivations->end(); ++it2 )
        {
          mitk::DataNode::Pointer fiducialNode = *it2;
          if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
          {
            mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData());
            mitk::Point3D wc0 = pe->GetWorldControlPoint(0);

            mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(imgNode->GetData());
            mitk::BaseGeometry::Pointer geom = img->GetGeometry();
            itk::Index<3> idx;
            geom->WorldToIndex(wc0, idx);
            mitk::Point3D cIdx; cIdx[0]=idx[0]; cIdx[1]=idx[1]; cIdx[2]=idx[2];
            mitk::Point3D world;
            geom->IndexToWorld(cIdx,world);

            mitk::Vector3D trans = world - wc0;
            pe->GetGeometry()->Translate(trans);
          }
        }

        break;
      }
    }
  }

  for(unsigned int i=0; i<m_SelectedImages.size(); i++ )
  {
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_SelectedImages.at(i)->GetData());

    mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(m_SelectedImages.at(i));
    for( mitk::DataStorage::SetOfObjects::const_iterator it = derivations->begin(); it != derivations->end(); ++it )
    {
      mitk::DataNode::Pointer fibNode = *it;
      if ( fibNode.IsNotNull() && dynamic_cast<mitk::FiberBundle*>(fibNode->GetData()) )
      {
        mitk::DataStorage::SetOfObjects::ConstPointer derivations2 = GetDataStorage()->GetDerivations(fibNode);
        for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations2->begin(); it2 != derivations2->end(); ++it2 )
        {
          mitk::DataNode::Pointer fiducialNode = *it2;
          if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
          {
            mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData());
            mitk::Point3D wc0 = pe->GetWorldControlPoint(0);

            mitk::BaseGeometry::Pointer geom = img->GetGeometry();
            itk::Index<3> idx;
            geom->WorldToIndex(wc0, idx);
            mitk::Point3D cIdx; cIdx[0]=idx[0]; cIdx[1]=idx[1]; cIdx[2]=idx[2];
            mitk::Point3D world;
            geom->IndexToWorld(cIdx,world);

            mitk::Vector3D trans = world - wc0;
            pe->GetGeometry()->Translate(trans);
          }
        }
      }
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  if (m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();
}

void QmitkFiberGenerationView::OnFlipButton()
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

void QmitkFiberGenerationView::OnAddBundle()
{
  if (m_SelectedImageNode.IsNull())
    return;

  mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedImageNode);

  mitk::FiberBundle::Pointer bundle = mitk::FiberBundle::New();
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData( bundle );
  QString name = QString("Bundle_%1").arg(children->size());
  node->SetName(name.toStdString());
  m_SelectedBundles.push_back(node);
  UpdateGui();

  GetDataStorage()->Add(node, m_SelectedImageNode);
}

void QmitkFiberGenerationView::OnDrawROI()
{
  if (m_SelectedBundles.empty())
    OnAddBundle();
  if (m_SelectedBundles.empty())
    return;

  mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedBundles.at(0));

  mitk::PlanarEllipse::Pointer figure = mitk::PlanarEllipse::New();

  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetData( figure );
  node->SetBoolProperty("planarfigure.3drendering", true);
  node->SetBoolProperty("planarfigure.3drendering.fill", true);

  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  for( int i=0; i<nodes.size(); i++)
    nodes.at(i)->SetSelected(false);

  m_SelectedFiducial = node;

  QString name = QString("Fiducial_%1").arg(children->size());
  node->SetName(name.toStdString());
  node->SetSelected(true);

  this->DisableCrosshairNavigation();

  mitk::PlanarFigureInteractor::Pointer figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetDataInteractor().GetPointer());
  if(figureInteractor.IsNull())
  {
    figureInteractor = mitk::PlanarFigureInteractor::New();
    us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "MitkPlanarFigure" );
    figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
    figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
    figureInteractor->SetDataNode( node );
  }

  UpdateGui();
  GetDataStorage()->Add(node, m_SelectedBundles.at(0));
}

bool QmitkFiberGenerationView::CompareLayer(mitk::DataNode::Pointer i,mitk::DataNode::Pointer j)
{
  int li = -1;
  i->GetPropertyValue("layer", li);
  int lj = -1;
  j->GetPropertyValue("layer", lj);
  return li<lj;
}

void QmitkFiberGenerationView::GenerateFibers()
{
  UpdateGui();
  if (m_SelectedBundles.empty())
  {
    if (m_SelectedFiducial.IsNull())
      return;

    mitk::DataStorage::SetOfObjects::ConstPointer parents = GetDataStorage()->GetSources(m_SelectedFiducial);
    for( mitk::DataStorage::SetOfObjects::const_iterator it = parents->begin(); it != parents->end(); ++it )
      if(dynamic_cast<mitk::FiberBundle*>((*it)->GetData()))
        m_SelectedBundles.push_back(*it);

    if (m_SelectedBundles.empty())
      return;
  }

  UpdateParametersFromGui();

  for (unsigned int i=0; i<m_SelectedBundles.size(); i++)
  {
    mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_SelectedBundles.at(i));
    std::vector< mitk::DataNode::Pointer > childVector;
    for( mitk::DataStorage::SetOfObjects::const_iterator it = children->begin(); it != children->end(); ++it )
      childVector.push_back(*it);
    std::sort(childVector.begin(), childVector.end(), CompareLayer);

    std::vector< mitk::PlanarEllipse::Pointer > fib;
    std::vector< unsigned int > flip;
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
          {
            radius = v.GetVnlVector().magnitude();
            ellipse->SetControlPoint(1, p);
            ellipse->Modified();
          }
          else
          {
            v.Normalize();
            v *= radius;
            ellipse->SetControlPoint(1, c+v);
            ellipse->Modified();
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
      m_Parameters.m_FiberGen.m_Fiducials.push_back(fib);
      m_Parameters.m_FiberGen.m_FlipList.push_back(flip);
    }
    else if (fib.size()>0)
      m_SelectedBundles.at(i)->SetData( mitk::FiberBundle::New() );

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

  itk::FibersFromPlanarFiguresFilter::Pointer filter = itk::FibersFromPlanarFiguresFilter::New();
  filter->SetParameters(m_Parameters.m_FiberGen);
  filter->Update();
  std::vector< mitk::FiberBundle::Pointer > fiberBundles = filter->GetFiberBundles();

  for (unsigned int i=0; i<fiberBundles.size(); i++)
  {
    m_SelectedBundles.at(i)->SetData( fiberBundles.at(i) );
    if (fiberBundles.at(i)->GetNumFibers()>50000)
      m_SelectedBundles.at(i)->SetVisibility(false);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberGenerationView::ApplyTransform()
{
  std::vector< mitk::DataNode::Pointer > selectedBundles;
  for(unsigned int i=0; i<m_SelectedImages.size(); i++ )
  {
    mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(m_SelectedImages.at(i));
    for( mitk::DataStorage::SetOfObjects::const_iterator it = derivations->begin(); it != derivations->end(); ++it )
    {
      mitk::DataNode::Pointer fibNode = *it;
      if ( fibNode.IsNotNull() && dynamic_cast<mitk::FiberBundle*>(fibNode->GetData()) )
        selectedBundles.push_back(fibNode);
    }
  }
  if (selectedBundles.empty())
    selectedBundles = m_SelectedBundles2;

  if (!selectedBundles.empty())
  {
    for (std::vector<mitk::DataNode::Pointer>::const_iterator it = selectedBundles.begin(); it!=selectedBundles.end(); ++it)
    {
      mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>((*it)->GetData());
      fib->RotateAroundAxis(m_Controls->m_XrotBox->value(), m_Controls->m_YrotBox->value(), m_Controls->m_ZrotBox->value());
      fib->TranslateFibers(m_Controls->m_XtransBox->value(), m_Controls->m_YtransBox->value(), m_Controls->m_ZtransBox->value());
      fib->ScaleFibers(m_Controls->m_XscaleBox->value(), m_Controls->m_YscaleBox->value(), m_Controls->m_ZscaleBox->value());

      // handle child fiducials
      if (m_Controls->m_IncludeFiducials->isChecked())
      {
        mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(*it);
        for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations->begin(); it2 != derivations->end(); ++it2 )
        {
          mitk::DataNode::Pointer fiducialNode = *it2;
          if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
          {
            mitk::PlanarEllipse* pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData());
            mitk::BaseGeometry* geom = pe->GetGeometry();

            // translate
            mitk::Vector3D world;
            world[0] = m_Controls->m_XtransBox->value();
            world[1] = m_Controls->m_YtransBox->value();
            world[2] = m_Controls->m_ZtransBox->value();
            geom->Translate(world);

            // calculate rotation matrix
            double x = m_Controls->m_XrotBox->value()*itk::Math::pi/180;
            double y = m_Controls->m_YrotBox->value()*itk::Math::pi/180;
            double z = m_Controls->m_ZrotBox->value()*itk::Math::pi/180;

            itk::Matrix< double, 3, 3 > rotX; rotX.SetIdentity();
            rotX[1][1] = cos(x);
            rotX[2][2] = rotX[1][1];
            rotX[1][2] = -sin(x);
            rotX[2][1] = -rotX[1][2];

            itk::Matrix< double, 3, 3 > rotY; rotY.SetIdentity();
            rotY[0][0] = cos(y);
            rotY[2][2] = rotY[0][0];
            rotY[0][2] = sin(y);
            rotY[2][0] = -rotY[0][2];

            itk::Matrix< double, 3, 3 > rotZ; rotZ.SetIdentity();
            rotZ[0][0] = cos(z);
            rotZ[1][1] = rotZ[0][0];
            rotZ[0][1] = -sin(z);
            rotZ[1][0] = -rotZ[0][1];

            itk::Matrix< double, 3, 3 > rot = rotZ*rotY*rotX;

            // transform control point coordinate into geometry translation
            geom->SetOrigin(pe->GetWorldControlPoint(0));
            mitk::Point2D cp; cp.Fill(0.0);
            pe->SetControlPoint(0, cp);

            // rotate fiducial
            geom->GetIndexToWorldTransform()->SetMatrix(rot*geom->GetIndexToWorldTransform()->GetMatrix());

            // implicit translation
            mitk::Vector3D trans;
            trans[0] = geom->GetOrigin()[0]-fib->GetGeometry()->GetCenter()[0];
            trans[1] = geom->GetOrigin()[1]-fib->GetGeometry()->GetCenter()[1];
            trans[2] = geom->GetOrigin()[2]-fib->GetGeometry()->GetCenter()[2];
            mitk::Vector3D newWc = rot*trans;
            newWc = newWc-trans;
            geom->Translate(newWc);

            pe->Modified();
          }
        }
      }
    }
  }
  else
  {
    for (unsigned int i=0; i<m_SelectedFiducials.size(); i++)
    {
      mitk::PlanarEllipse* pe = dynamic_cast<mitk::PlanarEllipse*>(m_SelectedFiducials.at(i)->GetData());
      mitk::BaseGeometry* geom = pe->GetGeometry();

      // translate
      mitk::Vector3D world;
      world[0] = m_Controls->m_XtransBox->value();
      world[1] = m_Controls->m_YtransBox->value();
      world[2] = m_Controls->m_ZtransBox->value();
      geom->Translate(world);

      // calculate rotation matrix
      double x = m_Controls->m_XrotBox->value()*itk::Math::pi/180;
      double y = m_Controls->m_YrotBox->value()*itk::Math::pi/180;
      double z = m_Controls->m_ZrotBox->value()*itk::Math::pi/180;
      itk::Matrix< double, 3, 3 > rotX; rotX.SetIdentity();
      rotX[1][1] = cos(x);
      rotX[2][2] = rotX[1][1];
      rotX[1][2] = -sin(x);
      rotX[2][1] = -rotX[1][2];
      itk::Matrix< double, 3, 3 > rotY; rotY.SetIdentity();
      rotY[0][0] = cos(y);
      rotY[2][2] = rotY[0][0];
      rotY[0][2] = sin(y);
      rotY[2][0] = -rotY[0][2];
      itk::Matrix< double, 3, 3 > rotZ; rotZ.SetIdentity();
      rotZ[0][0] = cos(z);
      rotZ[1][1] = rotZ[0][0];
      rotZ[0][1] = -sin(z);
      rotZ[1][0] = -rotZ[0][1];
      itk::Matrix< double, 3, 3 > rot = rotZ*rotY*rotX;

      // transform control point coordinate into geometry translation
      geom->SetOrigin(pe->GetWorldControlPoint(0));
      mitk::Point2D cp; cp.Fill(0.0);
      pe->SetControlPoint(0, cp);

      // rotate fiducial
      geom->GetIndexToWorldTransform()->SetMatrix(rot*geom->GetIndexToWorldTransform()->GetMatrix());
      pe->Modified();
    }
    if (m_Controls->m_RealTimeFibers->isChecked())
      GenerateFibers();
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberGenerationView::CopyBundles()
{
  if ( m_SelectedBundles.size()<1 ){
    QMessageBox::information( nullptr, "Warning", "Select at least one fiber bundle!");
    MITK_WARN("QmitkFiberGenerationView") << "Select at least one fiber bundle!";
    return;
  }

  for (std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedBundles.begin(); it!=m_SelectedBundles.end(); ++it)
  {
    // find parent image
    mitk::DataNode::Pointer parentNode;
    mitk::DataStorage::SetOfObjects::ConstPointer parentImgs = GetDataStorage()->GetSources(*it);
    for( mitk::DataStorage::SetOfObjects::const_iterator it2 = parentImgs->begin(); it2 != parentImgs->end(); ++it2 )
    {
      mitk::DataNode::Pointer pImgNode = *it2;
      if ( pImgNode.IsNotNull() && dynamic_cast<mitk::Image*>(pImgNode->GetData()) )
      {
        parentNode = pImgNode;
        break;
      }
    }

    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>((*it)->GetData());
    mitk::FiberBundle::Pointer newBundle = fib->GetDeepCopy();
    QString name((*it)->GetName().c_str());
    name += "_copy";

    mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
    fbNode->SetData(newBundle);
    fbNode->SetName(name.toStdString());
    fbNode->SetVisibility(true);
    if (parentNode.IsNotNull())
      GetDataStorage()->Add(fbNode, parentNode);
    else
      GetDataStorage()->Add(fbNode);

    // copy child fiducials
    if (m_Controls->m_IncludeFiducials->isChecked())
    {
      mitk::DataStorage::SetOfObjects::ConstPointer derivations = GetDataStorage()->GetDerivations(*it);
      for( mitk::DataStorage::SetOfObjects::const_iterator it2 = derivations->begin(); it2 != derivations->end(); ++it2 )
      {
        mitk::DataNode::Pointer fiducialNode = *it2;
        if ( fiducialNode.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData()) )
        {
          mitk::PlanarEllipse::Pointer pe = dynamic_cast<mitk::PlanarEllipse*>(fiducialNode->GetData())->Clone();
          mitk::DataNode::Pointer newNode = mitk::DataNode::New();
          newNode->SetData(pe);
          newNode->SetName(fiducialNode->GetName());
          newNode->SetBoolProperty("planarfigure.3drendering", true);
          GetDataStorage()->Add(newNode, fbNode);

        }
      }
    }
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberGenerationView::JoinBundles()
{
  if ( m_SelectedBundles.size()<2 ){
    QMessageBox::information( nullptr, "Warning", "Select at least two fiber bundles!");
    MITK_WARN("QmitkFiberGenerationView") << "Select at least two fiber bundles!";
    return;
  }

  std::vector< mitk::FiberBundle::Pointer > to_add;
  std::vector<mitk::DataNode::Pointer>::const_iterator it = m_SelectedBundles.begin();
  (*it)->SetVisibility(false);
  mitk::FiberBundle::Pointer newBundle = dynamic_cast<mitk::FiberBundle*>((*it)->GetData());
  QString name("");
  name += QString((*it)->GetName().c_str());
  ++it;
  for (; it!=m_SelectedBundles.end(); ++it)
  {
    (*it)->SetVisibility(false);
    to_add.push_back(dynamic_cast<mitk::FiberBundle*>((*it)->GetData()));
  }

  newBundle = newBundle->AddBundles(to_add);
  mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
  fbNode->SetData(newBundle);
  fbNode->SetName("Joined");
  fbNode->SetVisibility(true);
  GetDataStorage()->Add(fbNode);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberGenerationView::UpdateGui()
{
  m_Controls->m_FiberGenMessage->setVisible(true);
  m_Controls->m_TransformBundlesButton->setEnabled(false);
  m_Controls->m_CopyBundlesButton->setEnabled(false);
  m_Controls->m_GenerateFibersButton->setEnabled(false);
  m_Controls->m_FlipButton->setEnabled(false);
  m_Controls->m_CircleButton->setEnabled(false);
  m_Controls->m_JoinBundlesButton->setEnabled(false);
  m_Controls->m_AlignOnGrid->setEnabled(false);
  m_Controls->m_FiducialAttributeBox->setEnabled(false);

  // Fiber generation gui
  if (m_SelectedFiducial.IsNotNull())
  {
    m_Controls->m_TransformBundlesButton->setEnabled(true);
    m_Controls->m_FlipButton->setEnabled(true);
    m_Controls->m_AlignOnGrid->setEnabled(true);
  }

  if (m_SelectedFiducials.size()==1)
  {
    m_Controls->m_FiducialAttributeBox->setEnabled(true);
    mitk::PlanarEllipse* pe = dynamic_cast<mitk::PlanarEllipse*>(m_SelectedFiducials.at(0)->GetData());
    auto origin = pe->GetGeometry()->GetOrigin();
    m_Controls->m_FidPosX->setValue(origin[0]);
    m_Controls->m_FidPosY->setValue(origin[1]);
    m_Controls->m_FidPosZ->setValue(origin[2]);


    auto p0 = pe->GetControlPoint(0);
    auto p1 = pe->GetControlPoint(1);
    auto p2 = pe->GetControlPoint(2);
    auto p3 = pe->GetControlPoint(3);

    auto v1 = p1 - p0;
    auto v2 = p2 - p0;

    m_Controls->m_FidAxis1->setValue(v1.GetNorm());
    m_Controls->m_FidAxis2->setValue(v2.GetNorm());

    vnl_vector_fixed<double, 2> vt; vt[0]=1; vt[1]=0;
    auto v3 = p3-p0; v3.Normalize();
    auto angle = dot_product(vt, v3.GetVnlVector());
    angle = std::acos(angle)*180.0/itk::Math::pi;
    if (v3[1]<0)
      angle *= -1;
    m_Controls->m_FidTwist->setValue(angle);
  }
  else
  {
    m_Controls->m_FidPosX->setValue(0);
    m_Controls->m_FidPosY->setValue(0);
    m_Controls->m_FidPosZ->setValue(0);
    m_Controls->m_FidAxis1->setValue(1);
    m_Controls->m_FidAxis2->setValue(1);
    m_Controls->m_FidTwist->setValue(0);
  }

  if (m_SelectedImageNode.IsNotNull() || !m_SelectedBundles.empty())
  {
    m_Controls->m_CircleButton->setEnabled(true);
    m_Controls->m_FiberGenMessage->setVisible(false);
  }
  if (m_SelectedImageNode.IsNotNull() && !m_SelectedBundles.empty())
    m_Controls->m_AlignOnGrid->setEnabled(true);

  if (!m_SelectedBundles.empty())
  {
    m_Controls->m_TransformBundlesButton->setEnabled(true);
    m_Controls->m_CopyBundlesButton->setEnabled(true);
    m_Controls->m_GenerateFibersButton->setEnabled(true);

    if (m_SelectedBundles.size()>1)
      m_Controls->m_JoinBundlesButton->setEnabled(true);
  }
}

void QmitkFiberGenerationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  m_SelectedBundles2.clear();
  m_SelectedImages.clear();
  m_SelectedFiducials.clear();
  m_SelectedFiducial = nullptr;
  m_SelectedBundles.clear();
  m_SelectedImageNode = nullptr;

  // iterate all selected objects, adjust warning visibility
  for( int i=0; i<nodes.size(); i++)
  {
    mitk::DataNode::Pointer node = nodes.at(i);

    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_SelectedImages.push_back(node);
      m_SelectedImageNode = node;
    }
    else if ( node.IsNotNull() && dynamic_cast<mitk::FiberBundle*>(node->GetData()) )
    {
      m_SelectedBundles2.push_back(node);
      if (m_Controls->m_RealTimeFibers->isChecked())
      {
        m_SelectedBundles.push_back(node);
        mitk::FiberBundle::Pointer newFib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
        if (newFib->GetNumFibers()!=static_cast<unsigned int>(m_Controls->m_FiberDensityBox->value()))
          GenerateFibers();
      }
      else
        m_SelectedBundles.push_back(node);
    }
    else if ( node.IsNotNull() && dynamic_cast<mitk::PlanarEllipse*>(node->GetData()) )
    {
      m_SelectedFiducials.push_back(node);
      m_SelectedFiducial = node;
      m_SelectedBundles.clear();
      mitk::DataStorage::SetOfObjects::ConstPointer parents = GetDataStorage()->GetSources(node);
      for( mitk::DataStorage::SetOfObjects::const_iterator it = parents->begin(); it != parents->end(); ++it )
      {
        mitk::DataNode::Pointer pNode = *it;
        if ( pNode.IsNotNull() && dynamic_cast<mitk::FiberBundle*>(pNode->GetData()) )
          m_SelectedBundles.push_back(pNode);
      }
    }
  }
  UpdateGui();
}


void QmitkFiberGenerationView::EnableCrosshairNavigation()
{
  if (m_Controls->m_RealTimeFibers->isChecked())
    GenerateFibers();
}

void QmitkFiberGenerationView::DisableCrosshairNavigation()
{
}

void QmitkFiberGenerationView::NodeRemoved(const mitk::DataNode* node)
{
  mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>(node);
  std::map<mitk::DataNode*, QmitkPlanarFigureData>::iterator it = m_DataNodeToPlanarFigureData.find(nonConstNode);

  if (dynamic_cast<FiberBundle*>(node->GetData()))
  {
    m_SelectedBundles.clear();
    m_SelectedBundles2.clear();
  }
  else if (dynamic_cast<Image*>(node->GetData()))
    m_SelectedImages.clear();

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

void QmitkFiberGenerationView::NodeAdded( const mitk::DataNode* node )
{
  // add observer for selection in renderwindow
  mitk::PlanarFigure* figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
  bool isPositionMarker (false);
  node->GetBoolProperty("isContourMarker", isPositionMarker);
  if( figure && !isPositionMarker )
  {
    MITK_DEBUG << "figure added. will add interactor if needed.";
    mitk::PlanarFigureInteractor::Pointer figureInteractor
        = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetDataInteractor().GetPointer());

    mitk::DataNode* nonConstNode = const_cast<mitk::DataNode*>( node );
    if(figureInteractor.IsNull())
    {
      figureInteractor = mitk::PlanarFigureInteractor::New();
      us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "MitkPlanarFigure" );
      figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
      figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
      figureInteractor->SetDataNode( nonConstNode );
    }

    MITK_DEBUG << "will now add observers for planarfigure";
    QmitkPlanarFigureData data;
    data.m_Figure = figure;

    //        // add observer for event when figure has been placed
    typedef itk::SimpleMemberCommand< QmitkFiberGenerationView > SimpleCommandType;
    //        SimpleCommandType::Pointer initializationCommand = SimpleCommandType::New();
    //        initializationCommand->SetCallbackFunction( this, &QmitkFiberGenerationView::PlanarFigureInitialized );
    //        data.m_EndPlacementObserverTag = figure->AddObserver( mitk::EndPlacementPlanarFigureEvent(), initializationCommand );

    // add observer for event when figure is picked (selected)
    typedef itk::MemberCommand< QmitkFiberGenerationView > MemberCommandType;
    MemberCommandType::Pointer selectCommand = MemberCommandType::New();
    selectCommand->SetCallbackFunction( this, &QmitkFiberGenerationView::PlanarFigureSelected );
    data.m_SelectObserverTag = figure->AddObserver( mitk::SelectPlanarFigureEvent(), selectCommand );

    // add observer for event when interaction with figure starts
    SimpleCommandType::Pointer startInteractionCommand = SimpleCommandType::New();
    startInteractionCommand->SetCallbackFunction( this, &QmitkFiberGenerationView::DisableCrosshairNavigation);
    data.m_StartInteractionObserverTag = figure->AddObserver( mitk::StartInteractionPlanarFigureEvent(), startInteractionCommand );

    // add observer for event when interaction with figure starts
    SimpleCommandType::Pointer endInteractionCommand = SimpleCommandType::New();
    endInteractionCommand->SetCallbackFunction( this, &QmitkFiberGenerationView::EnableCrosshairNavigation);
    data.m_EndInteractionObserverTag = figure->AddObserver( mitk::EndInteractionPlanarFigureEvent(), endInteractionCommand );

    m_DataNodeToPlanarFigureData[nonConstNode] = data;
  }
}

void QmitkFiberGenerationView::PlanarFigureSelected( itk::Object* object, const itk::EventObject& )
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

void QmitkFiberGenerationView::SetFocus()
{
  m_Controls->m_CircleButton->setFocus();
}
