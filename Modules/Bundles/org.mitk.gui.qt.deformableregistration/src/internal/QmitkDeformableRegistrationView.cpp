/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkDeformableRegistrationView.h"
#include "ui_QmitkDeformableRegistrationViewControls.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkCommonFunctionality.h"
#include "qinputdialog.h"
#include "qmessagebox.h"
#include "qcursor.h"
#include "qapplication.h"
#include "qradiobutton.h"
#include "qslider.h"

#include <vtkTransform.h>

#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateNOT.h"
#include "mitkVectorImageMapper2D.h"

#include "itkImageFileReader.h"
#include "itkWarpImageFilter.h"

#include "mitkDataNodeObject.h"

#include "berryIWorkbenchWindow.h"
#include "berryISelectionService.h"

typedef itk::Vector< float, 3 >       VectorType;
typedef itk::Image< VectorType, 3 >   DeformationFieldType;

typedef itk::ImageFileReader< DeformationFieldType > ImageReaderType;

const std::string QmitkDeformableRegistrationView::VIEW_ID = "org.mitk.views.deformableregistration";

using namespace berry;

struct SelListenerDeformableRegistration : ISelectionListener
{
  berryObjectMacro(SelListenerDeformableRegistration);

  SelListenerDeformableRegistration(QmitkDeformableRegistrationView* view)
  {
    m_View = view;
  }

  void DoSelectionChanged(ISelection::ConstPointer selection)
  {
//    if(!m_View->IsVisible())
//      return;
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
          m_View->m_Controls.m_SwitchImages->hide();
          m_View->m_Controls.m_FixedLabel->hide();
          m_View->m_Controls.TextLabelMoving->hide();
          m_View->m_Controls.m_MovingLabel->hide();
          m_View->m_Controls.m_OpacityLabel->setEnabled(false);
          m_View->m_Controls.m_OpacitySlider->setEnabled(false);
          m_View->m_Controls.label->setEnabled(false);
          m_View->m_Controls.label_2->setEnabled(false);
          m_View->m_Controls.m_ShowRedGreenValues->setEnabled(false);
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
            if(QString("Image").compare(node->GetData()->GetNameOfClass())==0)
            {
              if (dynamic_cast<mitk::Image*>(node->GetData())->GetDimension() == 4)
              {
                m_View->m_Controls.m_StatusLabel->show();
                QMessageBox::information( NULL, "DeformableRegistration", "Only 2D or 3D images can be processed.", QMessageBox::Ok );
                return;
              }
              if (foundFixedImage == false)
              {
                fixedNode = node;
                foundFixedImage = true;
              }
              else
              {
                m_View->SetImagesVisible(selection);
                m_View->FixedSelected(fixedNode);
                m_View->MovingSelected(node);
                m_View->m_Controls.m_StatusLabel->hide();
                m_View->m_Controls.TextLabelFixed->show();
                m_View->m_Controls.m_SwitchImages->show();
                m_View->m_Controls.m_FixedLabel->show();
                m_View->m_Controls.TextLabelMoving->show();
                m_View->m_Controls.m_MovingLabel->show();
                m_View->m_Controls.m_OpacityLabel->setEnabled(true);
                m_View->m_Controls.m_OpacitySlider->setEnabled(true);
                m_View->m_Controls.label->setEnabled(true);
                m_View->m_Controls.label_2->setEnabled(true);
                m_View->m_Controls.m_ShowRedGreenValues->setEnabled(true);
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

  void SelectionChanged(IWorkbenchPart::Pointer part, ISelection::ConstPointer selection)
  {
    // check, if selection comes from datamanager
    if (part)
    {
      QString partname(part->GetPartName().c_str());
      if(partname.compare("Datamanager")==0)
      {
        // apply selection
        DoSelectionChanged(selection);
      }
    }
  }

  QmitkDeformableRegistrationView* m_View;
};

QmitkDeformableRegistrationView::QmitkDeformableRegistrationView(QObject * /*parent*/, const char * /*name*/)
: QmitkFunctionality() , m_MultiWidget(NULL), m_MovingNode(NULL), m_FixedNode(NULL), m_ShowRedGreen(false), 
  m_Opacity(0.5), m_OriginalOpacity(1.0), m_Deactivated(false)
{
}

QmitkDeformableRegistrationView::~QmitkDeformableRegistrationView()
{
  if (m_SelListener.IsNotNull())
  {
    berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
    if(s)
      s->RemovePostSelectionListener(m_SelListener);
    m_SelListener = NULL;
  }
}

void QmitkDeformableRegistrationView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);
  m_Parent->setEnabled(false);
  this->CreateConnections(); 
  m_Controls.TextLabelFixed->hide();
  m_Controls.m_SwitchImages->hide();
  m_Controls.m_FixedLabel->hide();
  m_Controls.TextLabelMoving->hide();
  m_Controls.m_MovingLabel->hide();
  m_Controls.m_OpacityLabel->setEnabled(false);
  m_Controls.m_OpacitySlider->setEnabled(false);
  m_Controls.label->setEnabled(false);
  m_Controls.label_2->setEnabled(false);
  m_Controls.m_ShowRedGreenValues->setEnabled(false);
  m_Controls.m_DeformableTransform->hide();
  if (m_Controls.m_DeformableTransform->currentIndex() == 0)
  {
    m_Controls.m_QmitkDemonsRegistrationViewControls->show();
    m_Controls.m_QmitkBSplineRegistrationViewControls->hide();
  }
  else
  {
    m_Controls.m_QmitkDemonsRegistrationViewControls->hide();
    m_Controls.m_QmitkBSplineRegistrationViewControls->show();
  }
  this->CheckCalculateEnabled();
}


void QmitkDeformableRegistrationView::ApplyDeformationField()
{
  
  ImageReaderType::Pointer reader  = ImageReaderType::New();
  reader->SetFileName( m_Controls.m_QmitkBSplineRegistrationViewControls->m_Controls.m_DeformationField->text().toStdString() );
  reader->Update();
      
  DeformationFieldType::Pointer deformationField = reader->GetOutput();

  mitk::Image * mimage = dynamic_cast<mitk::Image*> (m_MovingNode->GetData());
  mitk::Image * fimage = dynamic_cast<mitk::Image*> (m_FixedNode->GetData());
  
  typedef itk::Image<float, 3> FloatImageType;  

  FloatImageType::Pointer itkMovingImage = FloatImageType::New();
  FloatImageType::Pointer itkFixedImage = FloatImageType::New();
  mitk::CastToItkImage(mimage, itkMovingImage);
  mitk::CastToItkImage(fimage, itkFixedImage);

  typedef itk::WarpImageFilter<
                            FloatImageType, 
                            FloatImageType,
                            DeformationFieldType  >     WarperType;

  typedef itk::LinearInterpolateImageFunction<
                                    FloatImageType,
                                    double          >  InterpolatorType;

  WarperType::Pointer warper = WarperType::New();
  InterpolatorType::Pointer interpolator = InterpolatorType::New();

  warper->SetInput( itkMovingImage );
  warper->SetInterpolator( interpolator );
  warper->SetOutputSpacing( itkFixedImage->GetSpacing() );
  warper->SetOutputOrigin( itkFixedImage->GetOrigin() );
  warper->SetOutputDirection (itkFixedImage->GetDirection() );
  warper->SetDeformationField( deformationField );
  warper->Update();

  FloatImageType::Pointer outputImage = warper->GetOutput();
  mitk::Image::Pointer result = mitk::Image::New();

  mitk::CastToMitkImage(outputImage, result);

  // Create new DataNode
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData( result );   
  newNode->SetProperty( "name", mitk::StringProperty::New("warped image") );

  // add the new datatree node to the datatree
  this->GetDefaultDataStorage()->Add(newNode); 
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); 

  //Image::Pointer outputImage = this->GetOutput();
  //mitk::CastToMitkImage( warper->GetOutput(), outputImage );

  
}

void QmitkDeformableRegistrationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_Parent->setEnabled(true);
  m_MultiWidget = &stdMultiWidget;
  m_MultiWidget->SetWidgetPlanesVisibility(true);
}

void QmitkDeformableRegistrationView::StdMultiWidgetNotAvailable()
{
  m_Parent->setEnabled(false);
  m_MultiWidget = NULL;
}

void QmitkDeformableRegistrationView::CreateConnections()
{
  connect(m_Controls.m_ShowRedGreenValues, SIGNAL(toggled(bool)), this, SLOT(ShowRedGreen(bool)));
  connect(m_Controls.m_DeformableTransform, SIGNAL(currentChanged(int)), this, SLOT(TabChanged(int)));
  connect(m_Controls.m_OpacitySlider, SIGNAL(sliderMoved(int)), this, SLOT(OpacityUpdate(int)));
  connect(m_Controls.m_CalculateTransformation, SIGNAL(clicked()), this, SLOT(Calculate()));
  connect((QObject*)(m_Controls.m_SwitchImages),SIGNAL(clicked()),this,SLOT(SwitchImages()));
  connect(this,SIGNAL(calculateBSplineRegistration()),m_Controls.m_QmitkBSplineRegistrationViewControls,SLOT(CalculateTransformation()));
  connect( (QObject*)(m_Controls.m_QmitkBSplineRegistrationViewControls->m_Controls.m_ApplyDeformationField),
    SIGNAL(clicked()), 
    (QObject*) this,
    SLOT(ApplyDeformationField()) );
}

void QmitkDeformableRegistrationView::Activated()
{
  m_Deactivated = false;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Activated();
  if (m_SelListener.IsNull())
  {
    m_SelListener = berry::ISelectionListener::Pointer(new SelListenerDeformableRegistration(this));
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
    berry::ISelection::ConstPointer sel(
      this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
    m_CurrentSelection = sel.Cast<const IStructuredSelection>();
    m_SelListener.Cast<SelListenerDeformableRegistration>()->DoSelectionChanged(sel);
  }
  this->OpacityUpdate(m_Controls.m_OpacitySlider->value());
  this->ShowRedGreen(m_Controls.m_ShowRedGreenValues->isChecked());
}

void QmitkDeformableRegistrationView::Visible()
{
  /*
  m_Deactivated = false;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Activated();
  if (m_SelListener.IsNull())
  {
    m_SelListener = berry::ISelectionListener::Pointer(new SelListenerDeformableRegistration(this));
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener("org.mitk.views.datamanager", m_SelListener);
    berry::ISelection::ConstPointer sel(
      this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
    m_CurrentSelection = sel.Cast<const IStructuredSelection>();
    m_SelListener.Cast<SelListenerDeformableRegistration>()->DoSelectionChanged(sel);
  }
  this->OpacityUpdate(m_Controls.m_OpacitySlider->value());
  this->ShowRedGreen(m_Controls.m_ShowRedGreenValues->isChecked());*/
}

void QmitkDeformableRegistrationView::Deactivated()
{
  m_Deactivated = true;
  this->SetImageColor(false);
  if (m_FixedNode.IsNotNull())
    m_FixedNode->SetOpacity(1.0);
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_OriginalOpacity);
  }
  m_FixedNode = NULL;
  m_MovingNode = NULL;
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->RemovePostSelectionListener(m_SelListener);
  m_SelListener = NULL;
}

void QmitkDeformableRegistrationView::Hidden()
{
  /*
  m_Deactivated = true;
  this->SetImageColor(false);
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_OriginalOpacity);
  }
  m_FixedNode = NULL;
  m_MovingNode = NULL;
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->RemovePostSelectionListener(m_SelListener);
  m_SelListener = NULL;*/
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  //QmitkFunctionality::Deactivated();
}

void QmitkDeformableRegistrationView::FixedSelected(mitk::DataNode::Pointer fixedImage)
{
  if (fixedImage.IsNotNull())
  {
    if (m_FixedNode != fixedImage)
    {
      // remove changes on previous selected node
      if (m_FixedNode.IsNotNull())
      {
        this->SetImageColor(false);
        m_FixedNode->SetOpacity(1.0);
        m_FixedNode->SetVisibility(false);
        m_FixedNode->SetProperty("selectedFixedImage", mitk::BoolProperty::New(false));
      }
      // get selected node
      m_FixedNode = fixedImage;
      m_FixedNode->SetOpacity(0.5);
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
      m_FixedNode->SetVisibility(true);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  else
  {
    m_FixedNode = fixedImage;
    m_Controls.m_FixedLabel->hide();
    m_Controls.TextLabelFixed->hide();
    m_Controls.m_SwitchImages->hide();
  }
  this->CheckCalculateEnabled();
}

void QmitkDeformableRegistrationView::MovingSelected(mitk::DataNode::Pointer movingImage)
{
  if (movingImage.IsNotNull())
  {
    if (m_MovingNode != movingImage)
    {
      if (m_MovingNode.IsNotNull())
      {
        m_MovingNode->SetOpacity(m_OriginalOpacity);
        if (m_FixedNode == m_MovingNode)
          m_FixedNode->SetOpacity(0.5);
        this->SetImageColor(false);
      }
      m_MovingNode = movingImage;
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
      m_MovingNode->SetVisibility(true);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  else
  {
    m_MovingNode = NULL;
    m_Controls.m_MovingLabel->hide();
    m_Controls.TextLabelMoving->hide();
  }
  this->CheckCalculateEnabled();
}

bool QmitkDeformableRegistrationView::CheckCalculate()
{
  if(m_MovingNode==m_FixedNode)
    return false;
  return true;
}

void QmitkDeformableRegistrationView::ShowRedGreen(bool redGreen)
{
  m_ShowRedGreen = redGreen;
  this->SetImageColor(m_ShowRedGreen);
}

void QmitkDeformableRegistrationView::SetImageColor(bool redGreen)
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

void QmitkDeformableRegistrationView::OpacityUpdate(float opacity)
{
  m_Opacity = opacity;
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_Opacity);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDeformableRegistrationView::OpacityUpdate(int opacity)
{
  float fValue = ((float)opacity)/100.0f;
  this->OpacityUpdate(fValue);
}

void QmitkDeformableRegistrationView::CheckCalculateEnabled()
{
  if (m_FixedNode.IsNotNull() && m_MovingNode.IsNotNull())
  {
    m_Controls.m_CalculateTransformation->setEnabled(true);
  }
  else
  {
    m_Controls.m_CalculateTransformation->setEnabled(false);
  }
}

void QmitkDeformableRegistrationView::Calculate()
{
  if (m_Controls.m_DeformableTransform->tabText(m_Controls.m_DeformableTransform->currentIndex()) == "Demons")
  {
    m_Controls.m_QmitkDemonsRegistrationViewControls->SetFixedNode(m_FixedNode);
    m_Controls.m_QmitkDemonsRegistrationViewControls->SetMovingNode(m_MovingNode);
    try
    {
      m_Controls.m_QmitkDemonsRegistrationViewControls->CalculateTransformation();
    }
    catch (itk::ExceptionObject& excpt)
    {
      QMessageBox::information( NULL, "Registration exception", excpt.GetDescription(), QMessageBox::Ok );
      return;
    }
    mitk::Image::Pointer resultImage = m_Controls.m_QmitkDemonsRegistrationViewControls->GetResultImage();
    mitk::Image::Pointer resultDeformationField = m_Controls.m_QmitkDemonsRegistrationViewControls->GetResultDeformationfield();   
    if (resultImage.IsNotNull())
    {
      mitk::DataNode::Pointer resultImageNode = mitk::DataNode::New();
      resultImageNode->SetData(resultImage);
      mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
      mitk::LevelWindow levelWindow;
      levelWindow.SetAuto( resultImage );
      levWinProp->SetLevelWindow(levelWindow);
      resultImageNode->GetPropertyList()->SetProperty("levelwindow",levWinProp);
      resultImageNode->SetStringProperty("name", "DeformableRegistrationResultImage");
      this->GetDataStorage()->Add(resultImageNode, m_MovingNode);
    }
    if (resultDeformationField.IsNotNull())
    {
      mitk::DataNode::Pointer resultDeformationFieldNode = mitk::DataNode::New();
      resultDeformationFieldNode->SetData(resultDeformationField);
      mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
      mitk::LevelWindow levelWindow;
      levelWindow.SetAuto( resultDeformationField );
      levWinProp->SetLevelWindow(levelWindow);
      resultDeformationFieldNode->GetPropertyList()->SetProperty("levelwindow",levWinProp);
      resultDeformationFieldNode->SetStringProperty("name", "DeformableRegistrationResultDeformationField");
      mitk::VectorImageMapper2D::Pointer mapper = mitk::VectorImageMapper2D::New();
      resultDeformationFieldNode->SetMapper(1, mapper);
      resultDeformationFieldNode->SetVisibility(false);
      this->GetDataStorage()->Add(resultDeformationFieldNode, m_MovingNode);
    }
  }

  else if (m_Controls.m_DeformableTransform->tabText(m_Controls.m_DeformableTransform->currentIndex()) == "B-Spline")
  {
    m_Controls.m_QmitkBSplineRegistrationViewControls->SetFixedNode(m_FixedNode);
    m_Controls.m_QmitkBSplineRegistrationViewControls->SetMovingNode(m_MovingNode);
    emit calculateBSplineRegistration();
  }
}

void QmitkDeformableRegistrationView::SetImagesVisible(berry::ISelection::ConstPointer selection)
{
  if (this->m_CurrentSelection->Size() == 0)
  {
    // show all images
    mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects = this->GetDataStorage()->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
      ; nodeIt != setOfObjects->End(); ++nodeIt)  // for each node
    {
      if ( (nodeIt->Value().IsNotNull()) && (nodeIt->Value()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(nodeIt->Value()->GetData())==NULL)
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
      if ( (nodeIt->Value().IsNotNull()) && (nodeIt->Value()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(nodeIt->Value()->GetData())==NULL)
      {
        nodeIt->Value()->SetVisibility(false);
      }
    }
  }
}

void QmitkDeformableRegistrationView::TabChanged(int index)
{
  if (index == 0)
  {
    m_Controls.m_QmitkDemonsRegistrationViewControls->show();
    m_Controls.m_QmitkBSplineRegistrationViewControls->hide();
  }
  else
  {
    m_Controls.m_QmitkDemonsRegistrationViewControls->hide();
    m_Controls.m_QmitkBSplineRegistrationViewControls->show();
  }
}

void QmitkDeformableRegistrationView::SwitchImages()
{
  mitk::DataNode::Pointer newMoving = m_FixedNode;
  mitk::DataNode::Pointer newFixed = m_MovingNode;
  this->FixedSelected(newFixed);
  this->MovingSelected(newMoving);
}
