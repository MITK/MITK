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

// Qmitk includes
#include "QmitkRigidRegistrationView.h"
#include "QmitkStdMultiWidget.h"

// MITK includes
#include "mitkDataNodeObject.h"
#include <mitkImageCast.h>
#include "mitkManualSegmentationToSurfaceFilter.h"
#include <mitkSegmentationSink.h>
#include <mitkImageStatisticsHolder.h>
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateProperty.h"

// QT includes
#include "qinputdialog.h"
#include "qmessagebox.h"
#include "qcursor.h"
#include "qapplication.h"
#include "qradiobutton.h"
#include "qslider.h"
#include "qtooltip.h"

// VTK includes
#include <vtkTransform.h>

// ITK includes
#include <itkBinaryThresholdImageFilter.h>

// BlueBerry includes
#include "berryIWorkbenchWindow.h"
#include "berryISelectionService.h"

const std::string QmitkRigidRegistrationView::VIEW_ID = "org.mitk.views.rigidregistration";
const char* QmitkRigidRegistrationView::PART_NAME = QT_TRANSLATE_NOOP("Plugin Title", "RigidRegistration");

using namespace berry;

namespace
{
  const mitk::ScalarType SLIDER_SENCITIVITY = 0.1;

  mitk::Vector3D computeRange(mitk::DataNode::Pointer fixedNode, mitk::DataNode::Pointer node)
  {
    auto movingImage = dynamic_cast<mitk::Image*>(fixedNode->GetData());
    auto fixedImage = dynamic_cast<mitk::Image*>(node->GetData());

    mitk::Vector3D range;
    range[0] = movingImage->GetLargestPossibleRegion().GetSize()[0] + fixedImage->GetLargestPossibleRegion().GetSize()[0];
    range[1] = movingImage->GetLargestPossibleRegion().GetSize()[1] + fixedImage->GetLargestPossibleRegion().GetSize()[1];
    range[2] = movingImage->GetLargestPossibleRegion().GetSize()[2] + fixedImage->GetLargestPossibleRegion().GetSize()[2];

    range /= SLIDER_SENCITIVITY;

    return range;
  }
}

struct SelListenerRigidRegistration : ISelectionListener
{
  berryObjectMacro(SelListenerRigidRegistration);

  SelListenerRigidRegistration(QmitkRigidRegistrationView* view)
  {
    m_View = view;
  }

  void DoSelectionChanged(ISelection::ConstPointer selection)
  {
    // save current selection in member variable
    m_View->m_CurrentSelection = selection.Cast<const IStructuredSelection>();

    // do something with the selected items
    if(m_View->m_CurrentSelection)
    {
      if (m_View->m_CurrentSelection->Size() != 2)
      {
        if (m_View->m_FixedNode.IsNull() || m_View->m_MovingNode.IsNull())
        {
          m_View->m_Controls.m_StatusLabel->show();
          m_View->m_Controls.TextLabelFixed->hide();
          m_View->m_Controls.m_FixedLabel->hide();
          m_View->m_Controls.TextLabelMoving->hide();
          m_View->m_Controls.m_MovingLabel->hide();
          m_View->m_Controls.m_UseMaskingCB->hide();
          m_View->m_Controls.m_OpacityLabel->setEnabled(false);
          m_View->m_Controls.m_OpacitySlider->setEnabled(false);
          m_View->m_Controls.label_2->setEnabled(false);
          m_View->m_Controls.m_ShowRedGreenValues->setEnabled(false);
          m_View->m_Controls.m_SwitchImages->hide();
        }
      }
      else
      {
        m_View->m_Controls.m_StatusLabel->hide();
        bool foundFixedImage = false;
        mitk::DataNode::Pointer fixedNode;
        // iterate selection
        for (IStructuredSelection::iterator i = m_View->m_CurrentSelection->Begin();
          i != m_View->m_CurrentSelection->End(); ++i)
        {
          // extract datatree node
          if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
          {
            mitk::DataNode::Pointer node = nodeObj->GetDataNode();
            // only look at interesting types
            QString nodeClass = node->GetData()->GetNameOfClass();
            if ( nodeClass.compare("Image") == 0
              || nodeClass.compare("LabelSetImage") == 0 )
            {
              if (dynamic_cast<mitk::Image*>(node->GetData())->GetDimension() == 4)
              {
                m_View->m_Controls.m_StatusLabel->show();
                QMessageBox::information( NULL, "RigidRegistration", "Only 2D or 3D images can be processed.", QMessageBox::Ok );
                return;
              }
              if (foundFixedImage == false)
              {
                fixedNode = node;
                foundFixedImage = true;
              }
              else
              {
                // m_View->SetImagesVisible(selection);
                m_View->FixedSelected(fixedNode);
                m_View->MovingSelected(node);
                m_View->m_Controls.m_StatusLabel->hide();
                m_View->m_Controls.TextLabelFixed->show();
                m_View->m_Controls.m_FixedLabel->show();
                m_View->m_Controls.TextLabelMoving->show();
                m_View->m_Controls.m_MovingLabel->show();
                m_View->m_Controls.m_UseMaskingCB->show();
                m_View->m_Controls.m_OpacityLabel->setEnabled(true);
                m_View->m_Controls.m_OpacitySlider->setEnabled(true);
                m_View->m_Controls.label_2->setEnabled(true);
                m_View->m_Controls.m_ShowRedGreenValues->setEnabled(true);

                auto range = computeRange(fixedNode, node);
                m_View->m_Controls.m_XTransSlider->setMaximum(range[0]);
                m_View->m_Controls.m_YTransSlider->setMaximum(range[1]);
                m_View->m_Controls.m_ZTransSlider->setMaximum(range[2]);

                m_View->m_Controls.m_XTransSlider->setMinimum(-range[0]);
                m_View->m_Controls.m_YTransSlider->setMinimum(-range[1]);
                m_View->m_Controls.m_ZTransSlider->setMinimum(-range[2]);
              }
            }
            else
            {
              m_View->m_Controls.m_StatusLabel->show();
              return;
            }
          }
        }
      }
    }
    else if (m_View->m_FixedNode.IsNull() || m_View->m_MovingNode.IsNull())
    {
      m_View->m_Controls.m_StatusLabel->show();
    }
  }

  void SelectionChanged(const IWorkbenchPart::Pointer& part,
                        const ISelection::ConstPointer& selection) override
  {
    // check, if selection comes from datamanager
    if (part)
    {
      QString partname = part->GetPartName();
      if(partname == "Data Manager")
      {
        // apply selection
        DoSelectionChanged(selection);
      }
    }
  }

  QmitkRigidRegistrationView* m_View;
};

QmitkRigidRegistrationView::QmitkRigidRegistrationView(QObject * /*parent*/, const char * /*name*/)
: QmitkFunctionality(),
  m_MultiWidget(NULL),
  m_MovingNode(NULL),
  m_MovingMaskNode(NULL),
  m_FixedNode(NULL),
  m_FixedMaskNode(NULL),
  m_ShowRedGreen(false),
  m_Opacity(0.5),
  m_OriginalOpacity(1.0),
  m_Deactivated(false),
  m_FixedDimension(0),
  m_MovingDimension(0),
  m_ContourHelperNode(nullptr)
{
  m_TranslateSliderPos[0] = 0;
  m_TranslateSliderPos[1] = 0;
  m_TranslateSliderPos[2] = 0;

  m_RotateSliderPos[0] = 0;
  m_RotateSliderPos[1] = 0;
  m_RotateSliderPos[2] = 0;

  m_ScaleSliderPos[0] = 0;
  m_ScaleSliderPos[1] = 0;
  m_ScaleSliderPos[2] = 0;

  translationParams = new int[3];
  rotationParams = new int[3];
  scalingParams = new int[3];

  m_TimeStepperAdapter = NULL;

  this->GetDataStorage()->RemoveNodeEvent.AddListener(mitk::MessageDelegate1<QmitkRigidRegistrationView,
    const mitk::DataNode*> ( this, &QmitkRigidRegistrationView::DataNodeHasBeenRemoved ));

  m_Timer = new QTimer(this);
  m_Timer->setInterval(1000);
  m_Timer->setSingleShot(true);
}

QmitkRigidRegistrationView::~QmitkRigidRegistrationView()
{
  if(m_SelListener)
  {
    berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
    if(s) s->RemovePostSelectionListener(m_SelListener.data());
  }

  this->GetDataStorage()->RemoveNodeEvent.RemoveListener(mitk::MessageDelegate1<QmitkRigidRegistrationView,
    const mitk::DataNode*> ( this, &QmitkRigidRegistrationView::DataNodeHasBeenRemoved ));

  if (m_ContourHelperNode) {
    this->GetDataStorage()->Remove(m_ContourHelperNode);
    m_ContourHelperNode = nullptr;
  }

  if (m_Timer) {
    if (m_Timer->isActive()) {
      m_Timer->stop();
    }
    delete m_Timer;
    m_Timer = nullptr;
  }
}

void QmitkRigidRegistrationView::CreateQtPartControl(QWidget* parent)
{
  SetPartName(QApplication::translate("Plugin Title", PART_NAME));

  m_Controls.setupUi(parent);
  m_Controls.m_ManualFrame->hide();
  m_Controls.timeSlider->hide();
  m_Controls.TextLabelFixed->hide();
  m_Controls.m_FixedLabel->hide();
  m_Controls.TextLabelMoving->hide();
  m_Controls.m_MovingLabel->hide();
  m_Controls.m_UseMaskingCB->hide();
  m_Controls.m_OpacityLabel->setEnabled(false);
  m_Controls.m_OpacitySlider->setEnabled(false);
  m_Controls.label_2->setEnabled(false);
  m_Controls.m_ShowRedGreenValues->setEnabled(false);
  m_Controls.m_SwitchImages->hide();
  if (m_Controls.m_RigidTransform->currentIndex() == 1)
  {
    m_Controls.frame->show();
  }
  else
  {
    m_Controls.frame->hide();
  }
  m_Controls.m_ManualFrame->setEnabled(false);
  m_Parent->setEnabled(false);

  this->m_Controls.m_RigidTransform->removeTab(1);

  mitk::NodePredicateAnd::Pointer andPred =                       // we want binary images in the selectors
    mitk::NodePredicateAnd::New(mitk::NodePredicateDataType::New("Image"),
    mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true)));

  m_Controls.m_FixedImageCB->SetPredicate(andPred);
  m_Controls.m_FixedImageCB->SetDataStorage(this->GetDataStorage());
  m_Controls.m_FixedImageCB->hide();
  m_Controls.m_FixedMaskLB->hide();

  m_Controls.m_MovingImageCB->SetPredicate(andPred);
  m_Controls.m_MovingImageCB->SetDataStorage(this->GetDataStorage());
  m_Controls.m_MovingImageCB->hide();
  m_Controls.m_MovingMaskLB->hide();

  this->CreateConnections();
  this->CheckCalculateEnabled();

  ShowManualRegistrationFrame();
  mitk::RigidRegistrationPreset* preset = new mitk::RigidRegistrationPreset();
  preset->LoadPreset();
}

void QmitkRigidRegistrationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_Parent->setEnabled(true);
  m_MultiWidget = &stdMultiWidget;
  m_MultiWidget->SetWidgetPlanesVisibility(true);
}

void QmitkRigidRegistrationView::StdMultiWidgetNotAvailable()
{
  m_Parent->setEnabled(false);
  m_MultiWidget = NULL;
}

void QmitkRigidRegistrationView::CreateConnections()
{
  connect((QObject*)(m_Controls.m_SwitchImages),SIGNAL(clicked()),this,SLOT(SwitchImages()));
  connect(m_Controls.m_ShowRedGreenValues, SIGNAL(toggled(bool)), this, SLOT(ShowRedGreen(bool)));
  connect(m_Controls.m_ShowContour, SIGNAL(toggled(bool)), this, SLOT(EnableContour(bool)));
  //connect(m_Controls.m_UseFixedImageMask, SIGNAL(toggled(bool)), this, SLOT(UseFixedMaskImageChecked(bool)));
  //connect(m_Controls.m_UseMovingImageMask, SIGNAL(toggled(bool)), this, SLOT(UseMovingMaskImageChecked(bool)));
  connect(m_Controls.m_RigidTransform, SIGNAL(currentChanged(int)), this, SLOT(TabChanged(int)));
  connect(m_Controls.m_OpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(OpacityUpdate(int)));

  connect(m_Timer, &QTimer::timeout, [this] { ShowContour(); });
  connect(m_Controls.m_ContourSlider, &QSlider::valueChanged, this, [this] (int threshold) {
    m_Controls.m_ThresholdLabel->setText(QString::number(threshold));
    m_Timer->start();
  });

  connect(m_Controls.m_AutomaticTranslation,SIGNAL(clicked()),this,SLOT(AlignCenters()));
  connect(m_Controls.m_XTransSlider, SIGNAL(valueChanged(int)), this, SLOT(xTrans_valueChanged(int)));
  connect(m_Controls.m_YTransSlider, SIGNAL(valueChanged(int)), this, SLOT(yTrans_valueChanged(int)));
  connect(m_Controls.m_ZTransSlider, SIGNAL(valueChanged(int)), this, SLOT(zTrans_valueChanged(int)));
  connect(m_Controls.m_XRotSlider, SIGNAL(valueChanged(int)), this, SLOT(xRot_valueChanged(int)));
  connect(m_Controls.m_YRotSlider, SIGNAL(valueChanged(int)), this, SLOT(yRot_valueChanged(int)));
  connect(m_Controls.m_ZRotSlider, SIGNAL(valueChanged(int)), this, SLOT(zRot_valueChanged(int)));
  connect(m_Controls.m_XScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(xScale_valueChanged(int)));
  connect(m_Controls.m_YScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(yScale_valueChanged(int)));
  connect(m_Controls.m_ZScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(zScale_valueChanged(int)));

  connect(m_Controls.m_UseMaskingCB, SIGNAL(stateChanged(int)),this,SLOT(OnUseMaskingChanged(int)));
  connect(m_Controls.m_FixedImageCB, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),this,SLOT(OnFixedMaskImageChanged(const mitk::DataNode*)));
  connect(m_Controls.m_MovingImageCB, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),this,SLOT(OnMovingMaskImageChanged(const mitk::DataNode*)));
}

void QmitkRigidRegistrationView::Activated()
{
  m_Deactivated = false;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Activated();
  if (m_SelListener.isNull())
  {
    m_SelListener.reset(new SelListenerRigidRegistration(this));
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener.data());
    berry::ISelection::ConstPointer sel(
      this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
    m_CurrentSelection = sel.Cast<const IStructuredSelection>();
    static_cast<SelListenerRigidRegistration*>(m_SelListener.data())->DoSelectionChanged(sel);
  }
  this->OpacityUpdate(m_Controls.m_OpacitySlider->value());
  this->ShowRedGreen(m_Controls.m_ShowRedGreenValues->isChecked());
  this->CheckCalculateEnabled();
}

void QmitkRigidRegistrationView::Deactivated()
{
  m_Deactivated = true;
  this->SetImageColor(false);
  if (m_FixedNode.IsNotNull())
    m_FixedNode->SetOpacity(1.0);
  m_FixedNode = NULL;
  m_MovingNode = NULL;
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s) s->RemovePostSelectionListener(m_SelListener.data());
  m_SelListener.reset();
}

void QmitkRigidRegistrationView::DataNodeHasBeenRemoved(const mitk::DataNode* node)
{
  if(node == m_FixedNode || node == m_MovingNode)
  {
    m_Controls.m_StatusLabel->show();
    m_Controls.TextLabelFixed->hide();
    m_Controls.m_FixedLabel->hide();
    m_Controls.TextLabelMoving->hide();
    m_Controls.m_MovingLabel->hide();
    m_Controls.m_OpacityLabel->setEnabled(false);
    m_Controls.m_OpacitySlider->setEnabled(false);
    m_Controls.label_2->setEnabled(false);
    m_Controls.m_ShowRedGreenValues->setEnabled(false);
    m_Controls.m_SwitchImages->hide();
  }

  else if(node == m_ContourHelperNode)
  {
    // can this cause a memory leak?
    m_ContourHelperNode = NULL;
  }
}

void QmitkRigidRegistrationView::FixedSelected(mitk::DataNode::Pointer fixedImage)
{
  if (m_FixedNode.IsNotNull())
  {
    this->SetImageColor(false);
    m_FixedNode->SetOpacity(1.0);
  }
  m_FixedNode = fixedImage;
  if (m_FixedNode.IsNotNull())
  {
    m_FixedNode->SetOpacity(0.5);
    m_FixedNode->SetVisibility(true);
    m_Controls.TextLabelFixed->setText(QString::fromStdString(m_FixedNode->GetName()));
    m_Controls.m_FixedLabel->show();
    m_Controls.TextLabelFixed->show();
    m_Controls.m_SwitchImages->show();
    mitk::ColorProperty::Pointer colorProperty;
    colorProperty = dynamic_cast<mitk::ColorProperty*>(m_FixedNode->GetProperty("color"));
    if ( colorProperty.IsNotNull() )
    {
      m_FixedColor = colorProperty->GetColor();
    }
    this->SetImageColor(m_ShowRedGreen);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    if (dynamic_cast<mitk::Image*>(m_FixedNode->GetData()))
    {
      m_FixedDimension = dynamic_cast<mitk::Image*>(m_FixedNode->GetData())->GetDimension();
      m_Controls.qmitkRigidRegistrationSelector1->SetFixedDimension(m_FixedDimension);
      m_Controls.qmitkRigidRegistrationSelector1->SetFixedNode(m_FixedNode);
    }

    // what's about masking?
    m_Controls.m_UseMaskingCB->show();

    // Modify slider range
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
    int min = (int)image->GetStatistics()->GetScalarValueMin();
    int max = (int)image->GetStatistics()->GetScalarValueMax();
    m_Controls.m_ContourSlider->setRange(min, max);

    // Set slider to a default value
    int avg = (min+max) / 2;
    m_Controls.m_ContourSlider->setSliderPosition(avg);
    m_Controls.m_ThresholdLabel->setText(QString::number(avg));
  }
  else
  {
    m_Controls.m_FixedLabel->hide();
    m_Controls.TextLabelFixed->hide();
    m_Controls.m_SwitchImages->hide();
  }
  this->CheckCalculateEnabled();
  if(this->GetActiveStdMultiWidget())
  {
    m_TimeStepperAdapter = new QmitkStepperAdapter((QObject*) m_Controls.timeSlider, m_MultiWidget->GetTimeNavigationController()->GetTime(), "sliceNavigatorTimeFromRigidRegistration");
    connect( m_TimeStepperAdapter, SIGNAL( Refetch() ), this, SLOT( UpdateTimestep() ) );
  }
}

void QmitkRigidRegistrationView::MovingSelected(mitk::DataNode::Pointer movingImage)
{
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_OriginalOpacity);
    if (m_FixedNode == m_MovingNode)
      m_FixedNode->SetOpacity(0.5);
    this->SetImageColor(false);
  }

  // selection did not change - do onot reset
  if( m_MovingNode.IsNotNull() && m_MovingNode == movingImage)
  {

  }
  else
  {

    m_MovingNode = movingImage;
    if (m_MovingNode.IsNotNull())
    {
      m_MovingNode->SetVisibility(true);
      m_Controls.TextLabelMoving->setText(QString::fromStdString(m_MovingNode->GetName()));
      m_Controls.m_MovingLabel->show();
      m_Controls.TextLabelMoving->show();
      mitk::ColorProperty::Pointer colorProperty;
      colorProperty = dynamic_cast<mitk::ColorProperty*>(m_MovingNode->GetProperty("color"));
      if ( colorProperty.IsNotNull() )
      {
        m_MovingColor = colorProperty->GetColor();
      }
      this->SetImageColor(m_ShowRedGreen);
      m_MovingNode->GetFloatProperty("opacity", m_OriginalOpacity);
      this->OpacityUpdate(m_Opacity);

      // what's about masking?
      m_Controls.m_UseMaskingCB->show();
    }
    else
    {
      m_Controls.m_MovingLabel->hide();
      m_Controls.TextLabelMoving->hide();
      m_Controls.m_UseMaskingCB->hide();
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    this->MovingImageChanged();
    this->CheckCalculateEnabled();
  }
}

void QmitkRigidRegistrationView::ShowRedGreen(bool redGreen)
{
  m_ShowRedGreen = redGreen;
  this->SetImageColor(m_ShowRedGreen);
}

void QmitkRigidRegistrationView::EnableContour(bool show)
{
  if(show)
    ShowContour();

  // Can happen when the m_ContourHelperNode was deleted before and now the show contour checkbox is turned off
  if(m_ContourHelperNode.IsNull())
    return;

  m_Controls.m_ContourSlider->setEnabled(show);
  m_ContourHelperNode->SetProperty("visible", mitk::BoolProperty::New(show));

  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void QmitkRigidRegistrationView::ShowContour()
{
  int threshold = m_Controls.m_ContourSlider->value();

  bool show = m_Controls.m_ShowContour->isChecked();

  if(m_FixedNode.IsNull() || !show)
    return;

  mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(m_FixedNode->GetData());

  typedef itk::Image<float,3> FloatImageType;
  typedef itk::Image<short,3> ShortImageType;

  // Create a binary image using the given treshold
  typedef itk::BinaryThresholdImageFilter<FloatImageType, ShortImageType> ThresholdFilterType;

  FloatImageType::Pointer floatImage = FloatImageType::New();
  mitk::CastToItkImage(image, floatImage);


  ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
  thresholdFilter->SetInput(floatImage);
  thresholdFilter->SetLowerThreshold(threshold);
  thresholdFilter->SetUpperThreshold((int)image->GetStatistics()->GetScalarValueMax());
  thresholdFilter->SetInsideValue(1);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->Update();

  ShortImageType::Pointer binaryImage = thresholdFilter->GetOutput();
  mitk::Image::Pointer mitkBinaryImage = mitk::Image::New();
  mitk::CastToMitkImage(binaryImage, mitkBinaryImage);




  // Create a contour from the binary image
  mitk::ManualSegmentationToSurfaceFilter::Pointer surfaceFilter = mitk::ManualSegmentationToSurfaceFilter::New();
  surfaceFilter->SetInput( mitkBinaryImage );
  surfaceFilter->SetThreshold( 1 ); //expects binary image with zeros and ones
  surfaceFilter->SetUseGaussianImageSmooth(false); // apply gaussian to thresholded image ?
  surfaceFilter->SetMedianFilter3D(false); // apply median to segmentation before marching cubes ?
  surfaceFilter->SetDecimate( mitk::ImageToSurfaceFilter::NoDecimation );

  surfaceFilter->UpdateLargestPossibleRegion();

  // calculate normals for nicer display
  mitk::Surface::Pointer surface = surfaceFilter->GetOutput();


  if(m_ContourHelperNode.IsNull())
  {
    m_ContourHelperNode = mitk::DataNode::New();
    m_ContourHelperNode->SetData(surface);
    m_ContourHelperNode->SetProperty("opacity", mitk::FloatProperty::New(1.0) );
    m_ContourHelperNode->SetProperty("line width", mitk::IntProperty::New(2) );
    m_ContourHelperNode->SetProperty("scalar visibility", mitk::BoolProperty::New(false) );
    m_ContourHelperNode->SetProperty( "name", mitk::StringProperty::New("surface") );
    m_ContourHelperNode->SetProperty("color", mitk::ColorProperty::New(1.0, 0.0, 0.0));
    m_ContourHelperNode->SetBoolProperty("helper object", true);
    this->GetDataStorage()->Add(m_ContourHelperNode);
  }
  else
  {
    m_ContourHelperNode->SetData(surface);
  }


  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}


void QmitkRigidRegistrationView::SetImageColor(bool redGreen)
{
  if (!redGreen && m_FixedNode.IsNotNull())
  {
    m_FixedNode->SetColor(m_FixedColor);
  }
  if (!redGreen && m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetColor(m_MovingColor);
  }
  if (redGreen && m_FixedNode.IsNotNull())
  {
    m_FixedNode->SetColor(1.0f, 0.0f, 0.0f);
  }
  if (redGreen && m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetColor(0.0f, 1.0f, 0.0f);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkRigidRegistrationView::OpacityUpdate(float opacity)
{
  m_Opacity = opacity;
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_Opacity);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkRigidRegistrationView::OpacityUpdate(int opacity)
{
  m_Controls.label_2->setText(QString::number(opacity) + QString("%"));
  float fValue = ((float)opacity)/100.0f;
  this->OpacityUpdate(fValue);
}

void QmitkRigidRegistrationView::Translate(int* translateVector)
{
  if (m_MovingNode.IsNotNull())
  {
    mitk::Vector3D translateVec;

    translateVec[0] = SLIDER_SENCITIVITY * (translateVector[0] - m_TranslateSliderPos[0]);
    translateVec[1] = SLIDER_SENCITIVITY * (translateVector[1] - m_TranslateSliderPos[1]);
    translateVec[2] = SLIDER_SENCITIVITY * (translateVector[2] - m_TranslateSliderPos[2]);

    m_TranslateSliderPos[0] = translateVector[0];
    m_TranslateSliderPos[1] = translateVector[1];
    m_TranslateSliderPos[2] = translateVector[2];

    vtkMatrix4x4* translationMatrix = vtkMatrix4x4::New();
    translationMatrix->Identity();

    double (*transMatrix)[4] = translationMatrix->Element;

    transMatrix[0][3] = -translateVec[0];
    transMatrix[1][3] = -translateVec[1];
    transMatrix[2][3] = -translateVec[2];

    translationMatrix->Invert();

    m_MovingNode->GetData()->GetGeometry()->Compose( translationMatrix );
    m_MovingNode->GetData()->Modified();
    mitk::DataStorage::SetOfObjects::ConstPointer children = this->GetDataStorage()->GetDerivations(m_MovingNode);
    unsigned long size;
    size = children->Size();
    mitk::DataNode::Pointer childNode;
    for (unsigned long i = 0; i < size; ++i)
    {
      childNode = children->GetElement(i);
      childNode->GetData()->GetGeometry()->Compose( translationMatrix );
      childNode->GetData()->Modified();
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkRigidRegistrationView::Rotate(int* rotateVector)
{
  if (m_MovingNode.IsNotNull())
  {
    mitk::Vector3D rotateVec;

    rotateVec[0] = rotateVector[0] - m_RotateSliderPos[0];
    rotateVec[1] = rotateVector[1] - m_RotateSliderPos[1];
    rotateVec[2] = rotateVector[2] - m_RotateSliderPos[2];

    m_RotateSliderPos[0] = rotateVector[0];
    m_RotateSliderPos[1] = rotateVector[1];
    m_RotateSliderPos[2] = rotateVector[2];

    vtkMatrix4x4* rotationMatrix = vtkMatrix4x4::New();
    vtkMatrix4x4* translationMatrix = vtkMatrix4x4::New();
    rotationMatrix->Identity();
    translationMatrix->Identity();

    double (*rotMatrix)[4] = rotationMatrix->Element;
    double (*transMatrix)[4] = translationMatrix->Element;

    mitk::Point3D centerBB = m_MovingNode->GetData()->GetGeometry()->GetCenter();

    transMatrix[0][3] = centerBB[0];
    transMatrix[1][3] = centerBB[1];
    transMatrix[2][3] = centerBB[2];

    translationMatrix->Invert();

    m_MovingNode->GetData()->GetGeometry()->Compose( translationMatrix );
    mitk::DataStorage::SetOfObjects::ConstPointer children = this->GetDataStorage()->GetDerivations(m_MovingNode);
    unsigned long size;
    size = children->Size();
    mitk::DataNode::Pointer childNode;
    for (unsigned long i = 0; i < size; ++i)
    {
      childNode = children->GetElement(i);
      childNode->GetData()->GetGeometry()->Compose( translationMatrix );
      childNode->GetData()->Modified();
    }

    double radianX = rotateVec[0] * vnl_math::pi / 180;
    double radianY = rotateVec[1] * vnl_math::pi / 180;
    double radianZ = rotateVec[2] * vnl_math::pi / 180;

    if ( rotateVec[0] != 0 )
    {
      rotMatrix[1][1] = cos( radianX );
      rotMatrix[1][2] = -sin( radianX );
      rotMatrix[2][1] = sin( radianX );
      rotMatrix[2][2] = cos( radianX );
    }
    else if ( rotateVec[1] != 0 )
    {
      rotMatrix[0][0] = cos( radianY );
      rotMatrix[0][2] = sin( radianY );
      rotMatrix[2][0] = -sin( radianY );
      rotMatrix[2][2] = cos( radianY );
    }
    else if ( rotateVec[2] != 0 )
    {
      rotMatrix[0][0] = cos( radianZ );
      rotMatrix[0][1] = -sin( radianZ );
      rotMatrix[1][0] = sin( radianZ );
      rotMatrix[1][1] = cos( radianZ );
    }

    m_MovingNode->GetData()->GetGeometry()->Compose( rotationMatrix );
    for (unsigned long i = 0; i < size; ++i)
    {
      childNode = children->GetElement(i);
      childNode->GetData()->GetGeometry()->Compose( rotationMatrix );
      childNode->GetData()->Modified();
    }

    translationMatrix->Invert();

    m_MovingNode->GetData()->GetGeometry()->Compose( translationMatrix );
    for (unsigned long i = 0; i < size; ++i)
    {
      childNode = children->GetElement(i);
      childNode->GetData()->GetGeometry()->Compose( translationMatrix );
      childNode->GetData()->Modified();
    }
    m_MovingNode->GetData()->Modified();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkRigidRegistrationView::Scale(int* scaleVector)
{
  if (m_MovingNode.IsNotNull())
  {
    mitk::Vector3D scaleVec;

    scaleVec[0] = scaleVector[0] - m_ScaleSliderPos[0];
    scaleVec[1] = scaleVector[1] - m_ScaleSliderPos[1];
    scaleVec[2] = scaleVector[2] - m_ScaleSliderPos[2];

    m_ScaleSliderPos[0] = scaleVector[0];
    m_ScaleSliderPos[1] = scaleVector[1];
    m_ScaleSliderPos[2] = scaleVector[2];

    vtkMatrix4x4* scalingMatrix = vtkMatrix4x4::New();
    scalingMatrix->Identity();

    double (*scaleMatrix)[4] = scalingMatrix->Element;

    if (scaleVec[0] >= 0)
    {
      for(int i = 0; i<scaleVec[0]; i++)
      {
        scaleMatrix[0][0] *= 0.95;
      }
    }
    else
    {
      for(int i = 0; i<-scaleVec[0]; i++)
      {
        scaleMatrix[0][0] *= 1/0.95;
      }
    }

    if (scaleVec[1] >= 0)
    {
      for(int i = 0; i<scaleVec[1]; i++)
      {
        scaleMatrix[1][1] *= 0.95;
      }
    }
    else
    {
      for(int i = 0; i<-scaleVec[1]; i++)
      {
        scaleMatrix[1][1] *= 1/0.95;
      }
    }

    if (scaleVec[2] >= 0)
    {
      for(int i = 0; i<scaleVec[2]; i++)
      {
        scaleMatrix[2][2] *= 0.95;
      }
    }
    else
    {
      for(int i = 0; i<-scaleVec[2]; i++)
      {
        scaleMatrix[2][2] *= 1/0.95;
      }
    }

    scalingMatrix->Invert();

    m_MovingNode->GetData()->GetGeometry()->Compose( scalingMatrix );
    m_MovingNode->GetData()->Modified();
    mitk::DataStorage::SetOfObjects::ConstPointer children = this->GetDataStorage()->GetDerivations(m_MovingNode);
    unsigned long size;
    size = children->Size();
    mitk::DataNode::Pointer childNode;
    for (unsigned long i = 0; i < size; ++i)
    {
      childNode = children->GetElement(i);
      childNode->GetData()->GetGeometry()->Compose( scalingMatrix );
      childNode->GetData()->Modified();
    }
    
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkRigidRegistrationView::AlignCenters()
{
  if (m_FixedNode.IsNotNull() && m_MovingNode.IsNotNull())
  {
    mitk::Point3D fixedPoint = m_FixedNode->GetData()->GetGeometry()->GetCenter();
    mitk::Point3D movingPoint = m_MovingNode->GetData()->GetGeometry()->GetCenter();
    mitk::Vector3D translateVec;
    translateVec =  fixedPoint - movingPoint;
    translateVec *= 10; // account for slider sensitivity
    m_Controls.m_XTransSlider->setValue((int)m_Controls.m_XTransSlider->value() + (int)translateVec[0]);
    m_Controls.m_YTransSlider->setValue((int)m_Controls.m_YTransSlider->value() + (int)translateVec[1]);
    m_Controls.m_ZTransSlider->setValue((int)m_Controls.m_ZTransSlider->value() + (int)translateVec[2]);
  }
}

void QmitkRigidRegistrationView::CheckCalculateEnabled()
{
  if (m_FixedNode.IsNotNull() && m_MovingNode.IsNotNull() )
  {
    m_Controls.m_ManualFrame->setEnabled(true);
  }
  else
  {
    m_Controls.m_ManualFrame->setEnabled(false);
  }
}
void QmitkRigidRegistrationView::xTrans_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    translationParams[0]=v;
    translationParams[1]=m_Controls.m_YTransSlider->value();
    translationParams[2]=m_Controls.m_ZTransSlider->value();
    Translate(translationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::yTrans_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    translationParams[0]=m_Controls.m_XTransSlider->value();
    translationParams[1]=v;
    translationParams[2]=m_Controls.m_ZTransSlider->value();
    Translate(translationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::zTrans_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    translationParams[0]=m_Controls.m_XTransSlider->value();
    translationParams[1]=m_Controls.m_YTransSlider->value();
    translationParams[2]=v;
    Translate(translationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::xRot_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    rotationParams[0]=v;
    rotationParams[1]=m_Controls.m_YRotSlider->value();
    rotationParams[2]=m_Controls.m_ZRotSlider->value();
    Rotate(rotationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::yRot_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    rotationParams[0]=m_Controls.m_XRotSlider->value();
    rotationParams[1]=v;
    rotationParams[2]=m_Controls.m_ZRotSlider->value();
    Rotate(rotationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::zRot_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    rotationParams[0]=m_Controls.m_XRotSlider->value();
    rotationParams[1]=m_Controls.m_YRotSlider->value();
    rotationParams[2]=v;
    Rotate(rotationParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::xScale_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    scalingParams[0]=v;
    scalingParams[1]=m_Controls.m_YScaleSlider->value();
    scalingParams[2]=m_Controls.m_ZScaleSlider->value();
    Scale(scalingParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::yScale_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    scalingParams[0]=m_Controls.m_XScaleSlider->value();
    scalingParams[1]=v;
    scalingParams[2]=m_Controls.m_ZScaleSlider->value();
    Scale(scalingParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::zScale_valueChanged( int v )
{
  if (m_MovingNode.IsNotNull())
  {
    scalingParams[0]=m_Controls.m_XScaleSlider->value();
    scalingParams[1]=m_Controls.m_YScaleSlider->value();
    scalingParams[2]=v;
    Scale(scalingParams);
  }
  else
  {
    MovingImageChanged();
  }
}

void QmitkRigidRegistrationView::MovingImageChanged()
{
  if (dynamic_cast<mitk::Image*>(m_MovingNode->GetData()))
  {
    m_Controls.m_XTransSlider->setValue(0);
    m_Controls.m_YTransSlider->setValue(0);
    m_Controls.m_ZTransSlider->setValue(0);
    translationParams[0]=0;
    translationParams[1]=0;
    translationParams[2]=0;
    m_Controls.m_XRotSlider->setValue(0);
    m_Controls.m_YRotSlider->setValue(0);
    m_Controls.m_ZRotSlider->setValue(0);
    rotationParams[0]=0;
    rotationParams[1]=0;
    rotationParams[2]=0;
    m_Controls.m_XScaleSlider->setValue(0);
    m_Controls.m_YScaleSlider->setValue(0);
    m_Controls.m_ZScaleSlider->setValue(0);
    scalingParams[0]=0;
    scalingParams[1]=0;
    scalingParams[2]=0;
    m_MovingDimension = dynamic_cast<mitk::Image*>(m_MovingNode->GetData())->GetDimension();
    m_Controls.qmitkRigidRegistrationSelector1->SetMovingDimension(m_MovingDimension);
    m_Controls.qmitkRigidRegistrationSelector1->SetMovingNode(m_MovingNode);
    this->CheckCalculateEnabled();
  }
}

void QmitkRigidRegistrationView::UpdateTimestep()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkRigidRegistrationView::ShowManualRegistrationFrame()
{
  m_Controls.m_ManualFrame->show();
}

void QmitkRigidRegistrationView::SetImagesVisible(berry::ISelection::ConstPointer /*selection*/)
{
  if (this->m_CurrentSelection->Size() == 0)
  {
    // show all images
    mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects = this->GetDataStorage()->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
      ; nodeIt != setOfObjects->End(); ++nodeIt)  // for each node
    {
      if ( (nodeIt->Value().IsNotNull()) && (nodeIt->Value()->GetProperty("visible")) && dynamic_cast<mitk::PlaneGeometryData*>(nodeIt->Value()->GetData())==NULL)
      {
        nodeIt->Value()->SetVisibility(true);
      }
    }
  }
  else
  {
    // hide all images
    mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects = this->GetDataStorage()->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
      ; nodeIt != setOfObjects->End(); ++nodeIt)  // for each node
    {
      if ( (nodeIt->Value().IsNotNull()) && (nodeIt->Value()->GetProperty("visible")) && dynamic_cast<mitk::PlaneGeometryData*>(nodeIt->Value()->GetData())==NULL)
      {
        nodeIt->Value()->SetVisibility(false);
      }
    }
  }
}

void QmitkRigidRegistrationView::TabChanged(int index)
{
  if (index == 0)
  {
    m_Controls.frame->hide();
  }
  else
  {
    m_Controls.frame->show();
  }
}

void QmitkRigidRegistrationView::SwitchImages()
{
  mitk::DataNode::Pointer newMoving = m_FixedNode;
  mitk::DataNode::Pointer newFixed = m_MovingNode;
  this->FixedSelected(newFixed);
  this->MovingSelected(newMoving);

  if(m_ContourHelperNode.IsNotNull())
  {
    // Update the contour
    ShowContour();
  }

  if(m_Controls.m_UseMaskingCB->isChecked())
  {
    mitk::DataNode::Pointer tempMovingMask = NULL;
    mitk::DataNode::Pointer tempFixedMask = NULL;

    if(m_FixedMaskNode.IsNotNull()) // it is initialized
    {
      tempFixedMask = m_Controls.m_FixedImageCB->GetSelectedNode();
    }

    if(m_MovingMaskNode.IsNotNull())  // it is initialized
    {
      tempMovingMask = m_Controls.m_MovingImageCB->GetSelectedNode();
    }
    m_Controls.m_FixedImageCB->SetSelectedNode(tempMovingMask);
    m_Controls.m_MovingImageCB->SetSelectedNode(tempFixedMask);
  }
}

void QmitkRigidRegistrationView::OnUseMaskingChanged( int state )
{
  if(state == Qt::Checked)
  {
    m_Controls.m_FixedImageCB->show();
    m_Controls.m_MovingImageCB->show();
    m_Controls.m_MovingMaskLB->show();
    m_Controls.m_FixedMaskLB->show();
  }
  else
  {
    m_Controls.m_FixedImageCB->hide();
    m_Controls.m_MovingImageCB->hide();
    m_Controls.m_MovingMaskLB->hide();
    m_Controls.m_FixedMaskLB->hide();

    m_FixedMaskNode = NULL;
    m_MovingMaskNode = NULL;
  }
}

void QmitkRigidRegistrationView::OnFixedMaskImageChanged( const mitk::DataNode* node )
{
  if(m_Controls.m_UseMaskingCB->isChecked())
    m_FixedMaskNode = const_cast<mitk::DataNode*>(node);
  else
    m_FixedMaskNode = NULL;
}

void QmitkRigidRegistrationView::OnMovingMaskImageChanged( const mitk::DataNode* node )
{
  if(m_Controls.m_UseMaskingCB->isChecked())
    m_MovingMaskNode = const_cast<mitk::DataNode*>(node);
  else
    m_MovingMaskNode = NULL;
}
