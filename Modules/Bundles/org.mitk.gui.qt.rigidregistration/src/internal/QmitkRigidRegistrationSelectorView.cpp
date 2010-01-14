/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: -1 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkImageTimeSelector.h"
#include <QValidator>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include "mitkMatrixConvert.h"
#include <qinputdialog.h>
#include <qmessagebox.h>
#include "QmitkLoadPresetDialog.h"
#include <itkArray.h>
#include "mitkRigidRegistrationPreset.h"
#include "mitkRigidRegistrationTestPreset.h"
#include "mitkProgressBar.h"

#include "QmitkRigidRegistrationSelectorView.h"

#include "QmitkTranslationTransformView.h"
#include "QmitkScaleTransformView.h"
#include "QmitkScaleLogarithmicTransformView.h"
#include "QmitkAffineTransformView.h"
#include "QmitkFixedCenterOfRotationAffineTransformView.h"
#include "QmitkRigid3DTransformView.h"
#include "QmitkEuler3DTransformView.h"
#include "QmitkCenteredEuler3DTransformView.h"
#include "QmitkQuaternionRigidTransformView.h"
#include "QmitkVersorTransformView.h"
#include "QmitkVersorRigid3DTransformView.h"
#include "QmitkScaleSkewVersor3DTransformView.h"
#include "QmitkSimilarity3DTransformView.h"
#include "QmitkRigid2DTransformView.h"
#include "QmitkCenteredRigid2DTransformView.h"
#include "QmitkEuler2DTransformView.h"
#include "QmitkSimilarity2DTransformView.h"
#include "QmitkCenteredSimilarity2DTransformView.h"

#include "QmitkMeanSquaresMetricView.h"
#include "QmitkNormalizedCorrelationMetricView.h"
#include "QmitkGradientDifferenceMetricView.h"
#include "QmitkKullbackLeiblerCompareHistogramMetricView.h"
#include "QmitkCorrelationCoefficientHistogramMetricView.h"
#include "QmitkMeanSquaresHistogramMetricView.h"
#include "QmitkMutualInformationHistogramMetricView.h"
#include "QmitkNormalizedMutualInformationHistogramMetricView.h"
#include "QmitkMattesMutualInformationMetricView.h"
#include "QmitkMeanReciprocalSquareDifferenceMetricView.h"
#include "QmitkMutualInformationMetricView.h"
#include "QmitkMatchCardinalityMetricView.h"
#include "QmitkKappaStatisticMetricView.h"

#include "QmitkExhaustiveOptimizerView.h"
#include "QmitkGradientDescentOptimizerView.h"
#include "QmitkQuaternionRigidTransformGradientDescentOptimizerView.h"
#include "QmitkLBFGSBOptimizerView.h"
#include "QmitkOnePlusOneEvolutionaryOptimizerView.h"
#include "QmitkPowellOptimizerView.h"
#include "QmitkFRPROptimizerView.h"
#include "QmitkRegularStepGradientDescentOptimizerView.h"
#include "QmitkVersorTransformOptimizerView.h"
#include "QmitkAmoebaOptimizerView.h"
#include "QmitkConjugateGradientOptimizerView.h"
#include "QmitkLBFGSOptimizerView.h"
#include "QmitkSPSAOptimizerView.h"
#include "QmitkVersorRigid3DTransformOptimizerView.h"

QmitkRigidRegistrationSelectorView::QmitkRigidRegistrationSelectorView(QWidget* parent, Qt::WindowFlags f ) : QWidget( parent, f ),
m_FixedNode(NULL), m_FixedMaskNode(NULL), m_MovingNode(NULL), m_MovingMaskNode(NULL), m_FixedDimension(0), m_MovingDimension(0), 
m_StopOptimization(false), m_GeometryItkPhysicalToWorldTransform(NULL), m_GeometryWorldToItkPhysicalTransform(NULL), 
m_MovingGeometry(NULL), m_ImageGeometry(NULL)
{
  m_Controls.setupUi(parent);

  this->AddTransform(new QmitkTranslationTransformView(this, f));
  this->AddTransform(new QmitkScaleTransformView(this, f));
  this->AddTransform(new QmitkScaleLogarithmicTransformView(this, f));
  this->AddTransform(new QmitkAffineTransformView(this, f));
  this->AddTransform(new QmitkFixedCenterOfRotationAffineTransformView(this, f));
  this->AddTransform(new QmitkRigid3DTransformView(this, f));
  this->AddTransform(new QmitkEuler3DTransformView(this, f));
  this->AddTransform(new QmitkCenteredEuler3DTransformView(this, f));
  this->AddTransform(new QmitkQuaternionRigidTransformView(this, f));
  this->AddTransform(new QmitkVersorTransformView(this, f));
  this->AddTransform(new QmitkVersorRigid3DTransformView(this, f));
  this->AddTransform(new QmitkScaleSkewVersor3DTransformView(this, f));
  this->AddTransform(new QmitkSimilarity3DTransformView(this, f));
  this->AddTransform(new QmitkRigid2DTransformView(this, f));
  this->AddTransform(new QmitkCenteredRigid2DTransformView(this, f));
  this->AddTransform(new QmitkEuler2DTransformView(this, f));
  this->AddTransform(new QmitkSimilarity2DTransformView(this, f));
  this->AddTransform(new QmitkCenteredSimilarity2DTransformView(this, f));

  this->AddMetric(new QmitkMeanSquaresMetricView(this, f));
  this->AddMetric(new QmitkNormalizedCorrelationMetricView(this, f));
  this->AddMetric(new QmitkGradientDifferenceMetricView(this, f));
  this->AddMetric(new QmitkKullbackLeiblerCompareHistogramMetricView(this, f));
  this->AddMetric(new QmitkCorrelationCoefficientHistogramMetricView(this, f));
  this->AddMetric(new QmitkMeanSquaresHistogramMetricView(this, f));
  this->AddMetric(new QmitkMutualInformationHistogramMetricView(this, f));
  this->AddMetric(new QmitkNormalizedMutualInformationHistogramMetricView(this, f));
  this->AddMetric(new QmitkMattesMutualInformationMetricView(this, f));
  this->AddMetric(new QmitkMeanReciprocalSquareDifferenceMetricView(this, f));
  this->AddMetric(new QmitkMutualInformationMetricView(this, f));
  this->AddMetric(new QmitkMatchCardinalityMetricView(this, f));
  this->AddMetric(new QmitkKappaStatisticMetricView(this, f));

  this->AddOptimizer(new QmitkExhaustiveOptimizerView(this, f));
  this->AddOptimizer(new QmitkGradientDescentOptimizerView(this, f));
  this->AddOptimizer(new QmitkQuaternionRigidTransformGradientDescentOptimizerView(this, f));
  this->AddOptimizer(new QmitkLBFGSBOptimizerView(this, f));
  this->AddOptimizer(new QmitkOnePlusOneEvolutionaryOptimizerView(this, f));
  this->AddOptimizer(new QmitkPowellOptimizerView(this, f));
  this->AddOptimizer(new QmitkFRPROptimizerView(this, f));
  this->AddOptimizer(new QmitkRegularStepGradientDescentOptimizerView(this, f));
  this->AddOptimizer(new QmitkVersorTransformOptimizerView(this, f));
  this->AddOptimizer(new QmitkAmoebaOptimizerView(this, f));
  this->AddOptimizer(new QmitkConjugateGradientOptimizerView(this, f));
  this->AddOptimizer(new QmitkLBFGSOptimizerView(this, f));
  this->AddOptimizer(new QmitkSPSAOptimizerView(this, f));
  this->AddOptimizer(new QmitkVersorRigid3DTransformOptimizerView(this, f));

  m_Observer = mitk::RigidRegistrationObserver::New();
  m_Controls.m_TransformFrame->setEnabled(true);
  m_Controls.m_MetricFrame->setEnabled(true);
  m_Controls.m_OptimizerFrame->setEnabled(true);
  m_Controls.m_InterpolatorFrame->setEnabled(true);

  m_Controls.m_TransformFrame->hide();
  m_Controls.m_MetricFrame->hide();
  m_Controls.m_OptimizerFrame->hide();
  m_Controls.m_InterpolatorFrame->hide();
  m_Controls.m_TransformBox->setCurrentIndex(0);
  m_Controls.m_MetricBox->setCurrentIndex(0);
  m_Controls.m_OptimizerBox->setCurrentIndex(0);
  m_Controls.m_TransformWidgetStack->setCurrentIndex(0);
  m_Controls.m_MetricWidgetStack->setCurrentIndex(0);
  m_Controls.m_OptimizerWidgetStack->setCurrentIndex(0);
  /// and show the selected views
  this->TransformSelected(m_Controls.m_TransformBox->currentIndex());
  this->MetricSelected(m_Controls.m_MetricBox->currentIndex());
  this->OptimizerSelected(m_Controls.m_OptimizerBox->currentIndex());
  

  //// create connections
  connect( m_Controls.m_TransformGroup, SIGNAL(clicked(bool)), m_Controls.m_TransformFrame, SLOT(setVisible(bool)));
  connect( m_Controls.m_TransformBox, SIGNAL(activated(int)), m_Controls.m_TransformWidgetStack, SLOT(setCurrentIndex(int)));
  connect( m_Controls.m_TransformBox, SIGNAL(activated(int)), this, SLOT(TransformSelected(int)));
  connect( m_Controls.m_MetricBox, SIGNAL(activated(int)), this, SLOT(MetricSelected(int)));
  connect( m_Controls.m_OptimizerBox, SIGNAL(activated(int)), this, SLOT(OptimizerSelected(int)));
  connect( m_Controls.m_MetricGroup, SIGNAL(clicked(bool)), m_Controls.m_MetricFrame, SLOT(setVisible(bool)));
  connect( m_Controls.m_MetricBox, SIGNAL(activated(int)), m_Controls.m_MetricWidgetStack, SLOT(setCurrentIndex(int)));
  connect( m_Controls.m_OptimizerGroup, SIGNAL(clicked(bool)), m_Controls.m_OptimizerFrame, SLOT(setVisible(bool)));
  connect( m_Controls.m_OptimizerBox, SIGNAL(activated(int)), m_Controls.m_OptimizerWidgetStack, SLOT(setCurrentIndex(int)));
  connect( m_Controls.m_InterpolatorGroup, SIGNAL(toggled(bool)), m_Controls.m_InterpolatorFrame, SLOT(setVisible(bool)));

  m_Preset = new mitk::RigidRegistrationPreset();
  m_Preset->LoadPreset();

  m_TestPreset = new mitk::RigidRegistrationTestPreset();
  m_TestPreset->LoadPreset();

  this->DoLoadRigidRegistrationPreset("AffineMutualInformationGradientDescent", false);
}

QmitkRigidRegistrationSelectorView::~QmitkRigidRegistrationSelectorView()
{
}

/// this method starts the registration process
void QmitkRigidRegistrationSelectorView::CalculateTransformation(unsigned int timestep)
{
  if (m_FixedNode.IsNotNull() && m_MovingNode.IsNotNull())
  {
    emit AddNewTransformationToUndoList();

    mitk::Image::Pointer fimage = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
    mitk::Image::Pointer mimage = dynamic_cast<mitk::Image*>(m_MovingNode->GetData());
    mitk::Image::Pointer mmimage = NULL;
    mitk::Image::Pointer fmimage = NULL;
    if (m_MovingMaskNode.IsNotNull())
    {
      mmimage = dynamic_cast<mitk::Image*>(m_MovingMaskNode->GetData());
    }
    if (m_FixedMaskNode.IsNotNull())
    {
      fmimage = dynamic_cast<mitk::Image*>(m_FixedMaskNode->GetData());
    }

    mitk::ImageTimeSelector::Pointer its = mitk::ImageTimeSelector::New();

    if(fimage->GetDimension()>3)
    {
      its->SetInput(fimage);
      its->SetTimeNr(timestep);
      its->Update();
      fimage = its->GetOutput();
    }

    if(mimage->GetDimension()>3)
    {
      its->SetInput(mimage);
      its->SetTimeNr(timestep);
      its->Update();
      mimage = its->GetOutput();
    }

    // Initial moving image geometry
    m_ImageGeometry = m_MovingNode->GetData()->GetGeometry()->Clone();
    std::cout << "Moving Image Geometry (IndexToWorldTransform)"  << std::endl;
    std::cout << m_ImageGeometry->GetIndexToWorldTransform()->GetMatrix();
    mitk::Geometry3D::TransformType::InputPointType center = m_ImageGeometry->GetIndexToWorldTransform()->GetCenter();
    std::cout << "center " << center[0] << " " << center[1] << " " << center[2]  << std::endl;
    mitk::Geometry3D::TransformType::OutputVectorType offset = m_ImageGeometry->GetIndexToWorldTransform()->GetOffset();
    std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl;
    std::cout << std::endl;

    // Fixed image geometry
    //     mitk::AffineGeometryFrame3D::Pointer m_FixedGeometryCopy = m_FixedNode->GetData()->GetGeometry()->Clone();
    //     std::cout << "Fixed Image Geometry (IndexToWorldTransform)"  << std::endl;
    //     std::cout << m_FixedGeometryCopy->GetIndexToWorldTransform()->GetMatrix();
    //     center = m_FixedGeometryCopy->GetIndexToWorldTransform()->GetCenter();
    //     std::cout << "center " << center[0] << " " << center[1] << " " << center[2]  << std::endl;
    //     offset = m_FixedGeometryCopy->GetIndexToWorldTransform()->GetOffset();
    //     std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl;
    //     std::cout << std::endl;

    // Calculate the World to ITK-Physical transform for the moving image
    m_MovingGeometry = m_MovingNode->GetData()->GetGeometry();

    unsigned long size;
    size = m_MovingNodeChildren->Size();
    mitk::DataTreeNode::Pointer childNode;
    for (unsigned long i = 0; i < size; ++i)
    {
      m_ChildNodes.insert(std::pair<mitk::DataTreeNode::Pointer, mitk::Geometry3D*>(m_MovingNodeChildren->GetElement(i), m_MovingNodeChildren->GetElement(i)->GetData()->GetGeometry()));
      m_ChildNodes2.insert(std::pair<mitk::DataTreeNode::Pointer, mitk::AffineGeometryFrame3D::Pointer>(m_MovingNodeChildren->GetElement(i), m_MovingNodeChildren->GetElement(i)->GetData()->GetGeometry()->Clone()));
    }

    m_GeometryWorldToItkPhysicalTransform = mitk::Geometry3D::TransformType::New();
    GetWorldToItkPhysicalTransform(m_MovingGeometry, m_GeometryWorldToItkPhysicalTransform.GetPointer());

    //     std::cout << "Moving Image: World to ITK-physical transform" << std::endl;
    //     std::cout << m_GeometryWorldToItkPhysicalTransform->GetMatrix();
    //     center = m_GeometryWorldToItkPhysicalTransform->GetCenter();
    //     std::cout << "center " << center[0] << " " << center[1] << " " << center[2]  << std::endl;
    //     offset = m_GeometryWorldToItkPhysicalTransform->GetOffset();
    //     std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl;
    //     std::cout << std::endl;

    // Calculate the ITK-Physical to World transform for the fixed image
    m_GeometryItkPhysicalToWorldTransform = mitk::Geometry3D::TransformType::New();
    mitk::Geometry3D::TransformType::Pointer fixedWorld2Phys = mitk::Geometry3D::TransformType::New();
    GetWorldToItkPhysicalTransform(m_FixedNode->GetData()->GetGeometry(), fixedWorld2Phys.GetPointer());
    fixedWorld2Phys->GetInverse(m_GeometryItkPhysicalToWorldTransform);

    //     std::cout << "Fixed Image: ITK-physical to World transform" << std::endl;
    //     std::cout << m_GeometryItkPhysicalToWorldTransform->GetMatrix();
    //     center = m_GeometryItkPhysicalToWorldTransform->GetCenter();
    //     std::cout << "center " << center[0] << " " << center[1] << " " << center[2]  << std::endl;
    //     offset = m_GeometryItkPhysicalToWorldTransform->GetOffset();
    //     std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl;
    //     std::cout << std::endl;

    // init callback
    itk::ReceptorMemberCommand<QmitkRigidRegistrationSelectorView>::Pointer command = itk::ReceptorMemberCommand<QmitkRigidRegistrationSelectorView>::New();
    command->SetCallbackFunction(this, &QmitkRigidRegistrationSelectorView::SetOptimizerValue);
    int observer = m_Observer->AddObserver( itk::AnyEvent(), command );

    std::vector<std::string> presets;
    // init registration method
    mitk::ImageRegistrationMethod::Pointer registration = mitk::ImageRegistrationMethod::New(); 

    registration->SetObserver(m_Observer);
    registration->SetInterpolator(m_Controls.m_InterpolatorBox->currentIndex());
    registration->SetReferenceImage(fimage);
    registration->SetInput(mimage);    
    if (mmimage.IsNotNull())
    {
      registration->SetMovingMask(mmimage);
    }
    if (fmimage.IsNotNull())
    {
      registration->SetFixedMask(fmimage);
    }
    
    dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->currentWidget())->SetFixedImage(dynamic_cast<mitk::Image*>(m_FixedNode->GetData()));
    dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->currentWidget())->SetMovingImage(dynamic_cast<mitk::Image*>(m_MovingNode->GetData()));
    registration->SetOptimizerScales(dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->currentWidget())->GetScales());
    registration->SetTransform(dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->currentWidget())->GetTransform());
    
    dynamic_cast<QmitkRigidRegistrationMetricsGUIBase*>(m_Controls.m_MetricWidgetStack->currentWidget())->SetMovingImage(dynamic_cast<mitk::Image*>(m_MovingNode->GetData()));
    registration->SetMetric(dynamic_cast<QmitkRigidRegistrationMetricsGUIBase*>(m_Controls.m_MetricWidgetStack->currentWidget())->GetMetric());
    
    registration->SetOptimizer(dynamic_cast<QmitkRigidRegistrationOptimizerGUIBase*>(m_Controls.m_OptimizerWidgetStack->currentWidget())->GetOptimizer());

    double time(0.0);
    double tstart(0.0);
    tstart = clock();

    try
    {
      registration->Update();
    }
    catch (itk::ExceptionObject e)
    {
      MITK_INFO << "Caught exception: "<<e.GetDescription();
      QMessageBox::information( this, "Registration exception", e.GetDescription());
      mitk::ProgressBar::GetInstance()->Progress(20);
    }

    time += clock() - tstart;
    time = time / CLOCKS_PER_SEC;

    //printOut of the Time
    MITK_INFO << "Registration Time: " << time;

    m_Observer->RemoveObserver(observer);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }  
}

void QmitkRigidRegistrationSelectorView::SetFixedNode( mitk::DataTreeNode * fixedNode )
{
  m_FixedNode = fixedNode;
  m_Controls.m_TransformBox->setCurrentIndex(m_Controls.m_TransformBox->currentIndex());
}

void QmitkRigidRegistrationSelectorView::SetFixedDimension( int dimension )
{
  m_FixedDimension = dimension;
}

void QmitkRigidRegistrationSelectorView::SetMovingNode( mitk::DataTreeNode * movingNode )
{
  m_MovingNode = movingNode;
  this->TransformSelected(m_Controls.m_TransformBox->currentIndex());
}

void QmitkRigidRegistrationSelectorView::SetMovingDimension(int dimension )
{
  m_MovingDimension = dimension;
}

// this is a callback function that retrieves the current transformation
// parameters after every step of progress in the optimizer.
// depending on the choosen transformation, we construct a vtktransform
// that will be applied to the geometry of the moving image.
// the values are delivered by mitkRigidRgistrationObserver.cpp
void QmitkRigidRegistrationSelectorView::SetOptimizerValue( const itk::EventObject & )
{
  if (m_StopOptimization)
  {
    m_Observer->SetStopOptimization(true);
    m_StopOptimization = false;
  }

  // retreive optimizer value for the current transformation
  double value = m_Observer->GetCurrentOptimizerValue();

  // retreive current parameterset of the transformation
  itk::Array<double> transformParams = m_Observer->GetCurrentTranslation();

  // init an empty affine transformation that will be filled with
  // the corresponding transformation parameters in the following
  vtkMatrix4x4* vtkmatrix = vtkMatrix4x4::New();
  vtkmatrix->Identity();

  // init a transform that will be initialized with the vtkmatrix later
  vtkTransform* vtktransform = vtkTransform::New();

  if (m_MovingNode.IsNotNull())
  {
    vtktransform = dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->currentWidget())->Transform(vtkmatrix, vtktransform, transformParams);

    // the retrieved transform goes from fixed to moving space.
    // invert the transform in order to go from moving to fixed space.
    vtkMatrix4x4* vtkmatrix_inv = vtkMatrix4x4::New();
    vtktransform->GetInverse(vtkmatrix_inv);

    // now adapt the moving geometry accordingly
    m_MovingGeometry->GetIndexToWorldTransform()->SetIdentity();

    // the next view lines: Phi(Phys2World)*Phi(Result)*Phi(World2Phy)*Phi(Initial)

    // set moving image geometry to registration result
    m_MovingGeometry->SetIndexToWorldTransformByVtkMatrix(vtkmatrix_inv);

    /*std::cout << std::endl;
    std::cout << m_MovingGeometry->GetIndexToWorldTransform()->GetMatrix();
    mitk::Geometry3D::TransformType::OutputVectorType offset = m_MovingGeometry->GetIndexToWorldTransform()->GetOffset();
    std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl;*/


#if !defined(ITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE)
    // the next few lines: Phi(Phys2World)*Phi(Result)*Phi(World2Phy)*Phi(Initial)
    // go to itk physical space before applying the registration result
    m_MovingGeometry->Compose(m_GeometryWorldToItkPhysicalTransform, 1);

    // right in the beginning, transform by initial moving image geometry
    m_MovingGeometry->Compose(m_ImageGeometry->GetIndexToWorldTransform(), 1);

    // in the end, go back to world space
    m_MovingGeometry->Compose(m_GeometryItkPhysicalToWorldTransform, 0);  

#else     
    m_MovingGeometry->Compose(m_ImageGeometry->GetIndexToWorldTransform(), 1);
#endif

    /*std::cout << std::endl << m_MovingGeometry->GetIndexToWorldTransform()->GetMatrix();
    offset = m_MovingGeometry->GetIndexToWorldTransform()->GetOffset();
    std::cout << "offset " << offset[0] << " " << offset[1] << " " << offset[2]  << std::endl << std::endl;*/

    // now adapt all children geometries accordingly if children exist
    std::map<mitk::DataTreeNode::Pointer, mitk::Geometry3D*>::iterator iter;
    std::map<mitk::DataTreeNode::Pointer, mitk::AffineGeometryFrame3D::Pointer>::iterator iter2;
    mitk::DataTreeNode::Pointer childNode;
    for( iter = m_ChildNodes.begin(); iter != m_ChildNodes.end(); iter++ ) 
    {
      childNode = (*iter).first;
      if (childNode.IsNotNull())
      {
        mitk::Geometry3D* childGeometry;
        mitk::AffineGeometryFrame3D::Pointer childImageGeometry;
        // Calculate the World to ITK-Physical transform for the moving mask
        childGeometry = (*iter).second;
        iter2 = m_ChildNodes2.find(childNode);
        childImageGeometry = (*iter2).second;

        childGeometry->GetIndexToWorldTransform()->SetIdentity();

        // the next view lines: Phi(Phys2World)*Phi(Result)*Phi(World2Phy)*Phi(Initial)

        // set moving mask geometry to registration result
        childGeometry->SetIndexToWorldTransformByVtkMatrix(vtkmatrix_inv);

#if !defined(ITK_IMAGE_BEHAVES_AS_ORIENTED_IMAGE)
        // the next few lines: Phi(Phys2World)*Phi(Result)*Phi(World2Phy)*Phi(Initial)
        // go to itk physical space before applying the registration result
        childGeometry->Compose(m_GeometryWorldToItkPhysicalTransform, 1);

        // right in the beginning, transform by initial moving image geometry
        childGeometry->Compose(childImageGeometry->GetIndexToWorldTransform(), 1);

        // in the end, go back to world space
        childGeometry->Compose(m_GeometryItkPhysicalToWorldTransform, 0);  

#else     
        childGeometry->Compose(childImageGeometry->GetIndexToWorldTransform(), 1);
#endif
      }
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  emit OptimizerChanged(value);
}

/// this method is called whenever the combobox with the selectable transforms changes
/// responsible for showing the selected transform parameters
void QmitkRigidRegistrationSelectorView::TransformSelected( int transform )
{
  if (m_FixedNode.IsNotNull())
  {
    dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->widget(transform))->SetFixedImage(dynamic_cast<mitk::Image*>(m_FixedNode->GetData()));
  }
  if (m_MovingNode.IsNotNull())
  {
    dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->widget(transform))->SetMovingImage(dynamic_cast<mitk::Image*>(m_MovingNode->GetData()));
  }
  int numberOfTransformParameters = dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->widget(transform))->GetNumberOfTransformParameters();
  dynamic_cast<QmitkRigidRegistrationOptimizerGUIBase*>(m_Controls.m_OptimizerWidgetStack->currentWidget())->SetNumberOfTransformParameters(numberOfTransformParameters);

  //set fixed height
  m_Controls.m_TransformWidgetStack->setFixedHeight( dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->widget(transform))->minimumSizeHint().height() );
  this->OptimizerSelected(m_Controls.m_OptimizerWidgetStack->currentIndex());
}

/// this method is called whenever the combobox with the selectable metrics changes
/// responsible for showing the selected metric parameters
void QmitkRigidRegistrationSelectorView::MetricSelected( int metric )
{
  if (m_FixedNode.IsNotNull())
  {
    dynamic_cast<QmitkRigidRegistrationMetricsGUIBase*>(m_Controls.m_MetricWidgetStack->widget(metric))->SetMovingImage(dynamic_cast<mitk::Image*>(m_MovingNode->GetData()));
  }

  //set fixed height
  m_Controls.m_MetricWidgetStack->setFixedHeight( dynamic_cast<QmitkRigidRegistrationMetricsGUIBase*>(m_Controls.m_MetricWidgetStack->widget(metric))->minimumSizeHint().height() );
}

/// this method is called whenever the combobox with the selectable optimizers changes
/// responsible for showing the selected optimizer parameters
void QmitkRigidRegistrationSelectorView::OptimizerSelected( int optimizer )
{
  int numberOfTransformParameters = dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->currentWidget())->GetNumberOfTransformParameters();
  dynamic_cast<QmitkRigidRegistrationOptimizerGUIBase*>(m_Controls.m_OptimizerWidgetStack->widget(optimizer))->SetNumberOfTransformParameters(numberOfTransformParameters);

  //set fixed height
  m_Controls.m_OptimizerWidgetStack->setFixedHeight( dynamic_cast<QmitkRigidRegistrationOptimizerGUIBase*>(m_Controls.m_OptimizerWidgetStack->widget(optimizer))->minimumSizeHint().height() );
}

void QmitkRigidRegistrationSelectorView::LoadRigidRegistrationParameter()
{
  this->DoLoadRigidRegistrationParameter(false);
}

void QmitkRigidRegistrationSelectorView::LoadRigidRegistrationTestParameter()
{
  this->DoLoadRigidRegistrationParameter(true);
}

void QmitkRigidRegistrationSelectorView::DoLoadRigidRegistrationParameter(bool testPreset)
{
  std::map<std::string, itk::Array<double> > existingPresets;
  if (testPreset)
  {
    existingPresets = m_TestPreset->getTransformValuesPresets(); 
  }
  else
  {
    existingPresets = m_Preset->getTransformValuesPresets();
  }

  std::map<std::string, itk::Array<double> >::iterator iter;
  std::list<std::string> presets;
  for( iter = existingPresets.begin(); iter != existingPresets.end(); iter++ ) 
  {
    presets.push_back( (*iter).first );
  }
  if (presets.empty())
  {
    QMessageBox::warning( NULL, "RigidRegistrationParameters.xml", "RigidRegistrationParameters.xml is empty/does not exist. There are no presets to select.");
    return;
  }
  presets.sort();
  // ask about the name to load a preset
  QmitkLoadPresetDialog dialog( this, 0, "Load Preset", presets ); // needs a QWidget as parent
  int dialogReturnValue = dialog.exec();
  if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

  this->DoLoadRigidRegistrationPreset(dialog.GetPresetName(), testPreset);
}

void QmitkRigidRegistrationSelectorView::DoLoadRigidRegistrationPreset(std::string presetName, bool testPreset)
{
  itk::Array<double> transformValues;
  if (testPreset)
  {
    transformValues = m_TestPreset->getTransformValues(presetName);
  }
  else
  {
    transformValues = m_Preset->getTransformValues(presetName);
  }
  m_Controls.m_TransformGroup->setChecked(true);
  m_Controls.m_TransformFrame->setVisible(true);
  m_Controls.m_TransformBox->setCurrentIndex((int)transformValues[0]);
  m_Controls.m_TransformWidgetStack->setCurrentIndex((int)transformValues[0]);
  this->TransformSelected((int)transformValues[0]);
  itk::Array<double> transformValuesForGUI;
  transformValuesForGUI.SetSize(transformValues.Size());
  transformValuesForGUI.fill(0);
  for (int i = 1; i < transformValues.Size(); i++)
  {
    transformValuesForGUI[i-1] = transformValues[i];
  }
  dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->currentWidget())->SetTransformParameters(transformValuesForGUI);

  itk::Array<double> metricValues;
  if (testPreset)
  {
    metricValues = m_TestPreset->getMetricValues(presetName);
  }
  else
  {
    metricValues = m_Preset->getMetricValues(presetName);
  }
  m_Controls.m_MetricGroup->setChecked(true);
  m_Controls.m_MetricFrame->setVisible(true);
  m_Controls.m_MetricBox->setCurrentIndex((int)metricValues[0]);
  m_Controls.m_MetricWidgetStack->setCurrentIndex((int)metricValues[0]);
  this->MetricSelected((int)metricValues[0]);
  itk::Array<double> metricValuesForGUI;
  metricValuesForGUI.SetSize(metricValues.Size());
  metricValuesForGUI.fill(0);
  for (int i = 1; i < metricValues.Size(); i++)
  {
    metricValuesForGUI[i-1] = metricValues[i];
  }
  dynamic_cast<QmitkRigidRegistrationMetricsGUIBase*>(m_Controls.m_MetricWidgetStack->currentWidget())->SetMetricParameters(metricValuesForGUI);

  itk::Array<double> optimizerValues;
  if (testPreset)
  {
    optimizerValues = m_TestPreset->getOptimizerValues(presetName);
  }
  else
  {
    optimizerValues = m_Preset->getOptimizerValues(presetName);
  } 
  m_Controls.m_OptimizerGroup->setChecked(true);
  m_Controls.m_OptimizerFrame->setVisible(true);
  m_Controls.m_OptimizerBox->setCurrentIndex((int)optimizerValues[0]);
  m_Controls.m_OptimizerWidgetStack->setCurrentIndex((int)optimizerValues[0]);
  this->OptimizerSelected((int)optimizerValues[0]);
  itk::Array<double> optimizerValuesForGUI;
  optimizerValuesForGUI.SetSize(optimizerValues.Size());
  optimizerValuesForGUI.fill(0);
  for (int i = 1; i < optimizerValues.Size(); i++)
  {
    optimizerValuesForGUI[i-1] = optimizerValues[i];
  }
  dynamic_cast<QmitkRigidRegistrationOptimizerGUIBase*>(m_Controls.m_OptimizerWidgetStack->currentWidget())->SetOptimizerParameters(optimizerValuesForGUI);

  itk::Array<double> interpolatorValues;
  if (testPreset)
  {
    interpolatorValues = m_TestPreset->getInterpolatorValues(presetName);
  }
  else
  {
    interpolatorValues = m_Preset->getInterpolatorValues(presetName);
  }
  m_Controls.m_InterpolatorGroup->setChecked(true);
  m_Controls.m_InterpolatorFrame->setVisible(true);
  m_Controls.m_InterpolatorBox->setCurrentIndex((int)interpolatorValues[0]);
}

void QmitkRigidRegistrationSelectorView::SaveRigidRegistrationParameter()
{
  this->DoSaveRigidRegistrationParameter(false);
}

void QmitkRigidRegistrationSelectorView::SaveRigidRegistrationTestParameter()
{
  this->DoSaveRigidRegistrationParameter(true);
}

void QmitkRigidRegistrationSelectorView::DoSaveRigidRegistrationParameter(bool testPreset)
{
  bool ok;
  QString text = QInputDialog::getText(this, 
    "Save Parameter Preset", "Enter name for preset:", QLineEdit::Normal,
    QString::null, &ok );
  if ( ok )
  {
    std::map<std::string, itk::Array<double> > existingPresets;
    if (testPreset)
    {
      existingPresets = m_TestPreset->getTransformValuesPresets();
    }
    else
    {
      existingPresets = m_Preset->getTransformValuesPresets();
    }
    std::map<std::string, itk::Array<double> >::iterator iter = existingPresets.find(std::string((const char*)text.toLatin1()));
    if (iter != existingPresets.end())
    {
      QMessageBox::critical( this, "Preset definition",
        "Presetname already exists.");
      return;
    }
    if (text.isEmpty())
    {
      QMessageBox::critical( this, "Preset definition",
        "Presetname has to be set.\n"
        "You have to enter a Presetname." );
      return;
    }
    itk::Array<double> transformValues;
    transformValues.SetSize(25);
    transformValues.fill(0);
    transformValues[0] = m_Controls.m_TransformBox->currentIndex();

    itk::Array<double> transformValuesFromGUI = dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->currentWidget())->GetTransformParameters();
    for (int i = 0; i < transformValuesFromGUI.Size(); i++)
    {
      transformValues[i+1] = transformValuesFromGUI[i];
    }

    std::map<std::string, itk::Array<double> > transformMap;
    if (testPreset)
    {
      transformMap = m_TestPreset->getTransformValuesPresets();
    }
    else
    {
      transformMap = m_Preset->getTransformValuesPresets();
    }
    transformMap[std::string((const char*)text.toLatin1())] = transformValues;

    itk::Array<double> metricValues;
    metricValues.SetSize(25);
    metricValues.fill(0);
    metricValues[0] = m_Controls.m_MetricBox->currentIndex();
    itk::Array<double> metricValuesFromGUI = dynamic_cast<QmitkRigidRegistrationMetricsGUIBase*>(m_Controls.m_MetricWidgetStack->currentWidget())->GetMetricParameters();
    for (int i = 0; i < metricValuesFromGUI.Size(); i++)
    {
      metricValues[i+1] = metricValuesFromGUI[i];
    }

    std::map<std::string, itk::Array<double> > metricMap;
    if (testPreset)
    {
      metricMap = m_TestPreset->getMetricValuesPresets();
    }
    else
    {
      metricMap = m_Preset->getMetricValuesPresets();
    }
    metricMap[std::string((const char*)text.toLatin1())] = metricValues;

    itk::Array<double> optimizerValues;
    optimizerValues.SetSize(25);
    optimizerValues.fill(0);
    optimizerValues[0] = m_Controls.m_OptimizerBox->currentIndex();
    itk::Array<double> optimizerValuesFromGUI = dynamic_cast<QmitkRigidRegistrationOptimizerGUIBase*>(m_Controls.m_OptimizerWidgetStack->currentWidget())->GetOptimizerParameters();
    for (int i = 0; i < optimizerValuesFromGUI.Size(); i++)
    {
      optimizerValues[i+1] = optimizerValuesFromGUI[i];
    }

    std::map<std::string, itk::Array<double> > optimizerMap;
    if (testPreset)
    {
      optimizerMap = m_TestPreset->getOptimizerValuesPresets();
    }
    else
    {
      optimizerMap = m_Preset->getOptimizerValuesPresets();
    }
    optimizerMap[std::string((const char*)text.toLatin1())] = optimizerValues;

    itk::Array<double> interpolatorValues;
    interpolatorValues.SetSize(25);
    interpolatorValues.fill(0);
    interpolatorValues[0] = m_Controls.m_InterpolatorBox->currentIndex();

    std::map<std::string, itk::Array<double> > interpolatorMap;
    if (testPreset)
    {
      interpolatorMap = m_TestPreset->getInterpolatorValuesPresets();
    }
    else
    {
      interpolatorMap = m_Preset->getInterpolatorValuesPresets();
    }
    interpolatorMap[std::string((const char*)text.toLatin1())] = interpolatorValues;

    if (testPreset)
    {
      m_TestPreset->newPresets(transformMap, metricMap, optimizerMap, interpolatorMap);
    }
    else
    {
      m_Preset->newPresets(transformMap, metricMap, optimizerMap, interpolatorMap);
    }
  }
  else 
  {
    // user pressed Cancel
  }
}

void QmitkRigidRegistrationSelectorView::StopOptimization(bool stopOptimization)
{
  m_StopOptimization = stopOptimization;
}

int QmitkRigidRegistrationSelectorView::GetSelectedTransform()
{
  return m_Controls.m_TransformBox->currentIndex();
}

void QmitkRigidRegistrationSelectorView::SetFixedMaskNode( mitk::DataTreeNode * fixedMaskNode )
{
  m_FixedMaskNode = fixedMaskNode;
  this->TransformSelected(m_Controls.m_TransformBox->currentIndex());
}

void QmitkRigidRegistrationSelectorView::SetMovingMaskNode( mitk::DataTreeNode * movingMaskNode )
{
  m_MovingMaskNode = movingMaskNode;
  this->TransformSelected(m_Controls.m_TransformBox->currentIndex());
}

void QmitkRigidRegistrationSelectorView::SetMovingNodeChildren(mitk::DataStorage::SetOfObjects::ConstPointer children)
{
  m_MovingNodeChildren = children;
}

void QmitkRigidRegistrationSelectorView::AddTransform(QmitkRigidRegistrationTransformsGUIBase* transform)
{
  m_Controls.m_TransformBox->addItem(transform->GetName());
  int i = 0;
  if (!dynamic_cast<QmitkRigidRegistrationTransformsGUIBase*>(m_Controls.m_TransformWidgetStack->widget(i)))
  {
    m_Controls.m_TransformWidgetStack->addWidget(transform);
    m_Controls.m_TransformWidgetStack->removeWidget(m_Controls.m_TransformWidgetStack->widget(i));
    transform->SetupUI(m_Controls.m_TransformWidgetStack->widget(i));
  }
  else
  {
    i = m_Controls.m_TransformWidgetStack->addWidget(transform);
    transform->SetupUI(m_Controls.m_TransformWidgetStack->widget(i));
  }
}

void QmitkRigidRegistrationSelectorView::AddMetric(QmitkRigidRegistrationMetricsGUIBase* metric)
{
  m_Controls.m_MetricBox->addItem(metric->GetName());
  int i = 0;
  if (!dynamic_cast<QmitkRigidRegistrationMetricsGUIBase*>(m_Controls.m_MetricWidgetStack->widget(i)))
  {
    m_Controls.m_MetricWidgetStack->addWidget(metric);
    m_Controls.m_MetricWidgetStack->removeWidget(m_Controls.m_MetricWidgetStack->widget(i));
    metric->SetupUI(m_Controls.m_MetricWidgetStack->widget(i));
  }
  else
  {
    i = m_Controls.m_MetricWidgetStack->addWidget(metric);
    metric->SetupUI(m_Controls.m_MetricWidgetStack->widget(i));
  }
}

void QmitkRigidRegistrationSelectorView::AddOptimizer(QmitkRigidRegistrationOptimizerGUIBase* optimizer)
{
  m_Controls.m_OptimizerBox->addItem(optimizer->GetName());
  int i = 0;
  if (!dynamic_cast<QmitkRigidRegistrationOptimizerGUIBase*>(m_Controls.m_OptimizerWidgetStack->widget(i)))
  {
    m_Controls.m_OptimizerWidgetStack->addWidget(optimizer);
    m_Controls.m_OptimizerWidgetStack->removeWidget(m_Controls.m_OptimizerWidgetStack->widget(i));
    optimizer->SetupUI(m_Controls.m_OptimizerWidgetStack->widget(i));
  }
  else
  {
    i = m_Controls.m_OptimizerWidgetStack->addWidget(optimizer);
    optimizer->SetupUI(m_Controls.m_OptimizerWidgetStack->widget(i));
  }
}
