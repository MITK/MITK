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

#include "QmitkPointBasedRegistrationView.h"
#include "ui_QmitkPointBasedRegistrationViewControls.h"
#include "QmitkPointListWidget.h"

#include <vtkIterativeClosestPointTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>

#include <vtkLandmarkTransform.h>
#include <QmitkStdMultiWidget.h>
#include "QmitkCommonFunctionality.h"
#include "qradiobutton.h"
#include "qapplication.h"
#include <qcursor.h>
#include <qinputdialog.h>
#include <qlcdnumber.h>
#include <qlabel.h>
#include "qmessagebox.h"

#include "LandmarkWarping.h"
#include <mitkPointOperation.h>
#include <mitkPositionEvent.h>
#include "mitkOperationEvent.h"
#include "mitkUndoController.h"
#include <mitkPointSetWriter.h>
#include <mitkPointSetReader.h>
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateNOT.h"

#include "mitkDataTreeNodeObject.h"

#include "cherryIWorkbenchWindow.h"
#include "cherryISelectionService.h"


const std::string QmitkPointBasedRegistrationView::VIEW_ID = "org.mitk.views.pointbasedregistration";

using namespace cherry;

struct SelListenerPointBasedRegistration : ISelectionListener
{
  cherryObjectMacro(SelListenerPointBasedRegistration);

  SelListenerPointBasedRegistration(QmitkPointBasedRegistrationView* view)
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
          m_View->m_Controls.m_StatusLabel->setText("You have to select two images from Datamanager for Registration!");
          m_View->m_Controls.m_StatusLabel->show();
          m_View->m_Controls.TextLabelFixed->hide();
          m_View->m_Controls.m_FixedLabel->hide();
          m_View->m_Controls.line2->hide();
          m_View->m_Controls.m_FixedPointListWidget->hide();
          m_View->m_Controls.TextLabelMoving->hide();
          m_View->m_Controls.m_MovingLabel->hide();
          m_View->m_Controls.line1->hide();
          m_View->m_Controls.m_MovingPointListWidget->hide();
          m_View->m_Controls.m_OpacityLabel->hide();
          m_View->m_Controls.m_OpacitySlider->hide();
          m_View->m_Controls.m_SwitchImages->hide();
          m_View->m_Controls.m_ShowRedGreenValues->setEnabled(false);
        }
      }
      else
      {
        m_View->m_Controls.m_StatusLabel->hide();
        bool foundFixedImage = false;
        mitk::DataTreeNode::Pointer fixedNode;
        // iterate selection
        for (IStructuredSelection::iterator i = m_View->m_CurrentSelection->Begin(); 
          i != m_View->m_CurrentSelection->End(); ++i)
        {
          // extract datatree node
          if (mitk::DataTreeNodeObject::Pointer nodeObj = i->Cast<mitk::DataTreeNodeObject>())
          {
            mitk::TNodePredicateDataType<mitk::BaseData>::Pointer isBaseData(mitk::TNodePredicateDataType<mitk::BaseData>::New());
            mitk::TNodePredicateDataType<mitk::PointSet>::Pointer isPointSet(mitk::TNodePredicateDataType<mitk::PointSet>::New());
            mitk::NodePredicateNOT::Pointer notPointSet = mitk::NodePredicateNOT::New(isPointSet);
            mitk::TNodePredicateDataType<mitk::Geometry2DData>::Pointer isGeometry2DData(mitk::TNodePredicateDataType<mitk::Geometry2DData>::New());
            mitk::NodePredicateNOT::Pointer notGeometry2DData = mitk::NodePredicateNOT::New(isGeometry2DData);
            mitk::NodePredicateAND::Pointer notPointSetAndNotGeometry2DData = mitk::NodePredicateAND::New( notPointSet, notGeometry2DData );
            mitk::NodePredicateAND::Pointer predicate = mitk::NodePredicateAND::New( isBaseData, notPointSetAndNotGeometry2DData );


            mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects = m_View->GetDataStorage()->GetSubset(predicate);

            mitk::DataTreeNode::Pointer node = nodeObj->GetDataTreeNode();

            // only look at interesting types
            for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
              ; nodeIt != setOfObjects->End(); ++nodeIt)  // for each node
            {
              if(nodeIt->Value().GetPointer() == node.GetPointer())
              {
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
                  m_View->m_Controls.m_ShowRedGreenValues->setEnabled(true);
                }
              }
            }
          } 
        }
        if (m_View->m_FixedNode.IsNull() || m_View->m_MovingNode.IsNull())
        {
          m_View->m_Controls.m_StatusLabel->setText("You have to select two images from Datamanager for Registration!");
          m_View->m_Controls.m_StatusLabel->show();
        }
      }
    }
    else if (m_View->m_FixedNode.IsNull() || m_View->m_MovingNode.IsNull())
    {
      m_View->m_Controls.m_StatusLabel->setText("You have to select two images from Datamanager for Registration!");
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

  QmitkPointBasedRegistrationView* m_View;
};


QmitkPointBasedRegistrationView::QmitkPointBasedRegistrationView(QObject * /*parent*/, const char * /*name*/)
: QmitkFunctionality(), m_SelListener(0), m_MultiWidget(NULL), m_FixedLandmarks(NULL), m_MovingLandmarks(NULL), m_MovingNode(NULL),
m_FixedNode(NULL), m_ShowRedGreen(false), m_Opacity(0.5), m_OriginalOpacity(1.0), m_Transformation(0), m_HideFixedImage(false), m_HideMovingImage(false),
m_OldFixedLabel(""), m_OldMovingLabel(""), m_Deactivated (false), m_CurrentFixedLandmarksObserverID(0), m_CurrentMovingLandmarksObserverID(0)
{
  m_FixedLandmarksChangedCommand = itk::SimpleMemberCommand<QmitkPointBasedRegistrationView>::New();
  m_FixedLandmarksChangedCommand->SetCallbackFunction(this, &QmitkPointBasedRegistrationView::updateFixedLandmarksList);
  m_MovingLandmarksChangedCommand = itk::SimpleMemberCommand<QmitkPointBasedRegistrationView>::New();
  m_MovingLandmarksChangedCommand->SetCallbackFunction(this, &QmitkPointBasedRegistrationView::updateMovingLandmarksList);
}

QmitkPointBasedRegistrationView::~QmitkPointBasedRegistrationView()
{
  if(m_SelListener.IsNotNull())
  {
    cherry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
    if(s)
      s->RemovePostSelectionListener(m_SelListener);
    m_SelListener = NULL;
  }
  if (m_FixedPointSetNode.IsNotNull())
  {
    m_Controls.m_FixedPointListWidget->DeactivateInteractor(true);
    m_FixedPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldFixedLabel));
  }
  if (m_MovingPointSetNode.IsNotNull())
  {
    m_Controls.m_MovingPointListWidget->DeactivateInteractor(true);
    m_MovingPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldMovingLabel));
  }
  m_Controls.m_FixedPointListWidget->SetPointSetNode(NULL);
  m_Controls.m_MovingPointListWidget->SetPointSetNode(NULL);
}

void QmitkPointBasedRegistrationView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);
  m_Parent->setEnabled(false);
  m_Controls.m_MeanErrorLCD->hide();
  m_Controls.m_MeanError->hide();
  m_Controls.TextLabelFixed->hide();
  m_Controls.line2->hide();
  m_Controls.m_FixedPointListWidget->hide();
  m_Controls.m_FixedLabel->hide();
  m_Controls.TextLabelMoving->hide();
  m_Controls.m_MovingLabel->hide();
  m_Controls.line1->hide();
  m_Controls.m_MovingPointListWidget->hide();
  m_Controls.m_OpacityLabel->hide();
  m_Controls.m_OpacitySlider->hide();
  m_Controls.m_SwitchImages->hide();
  m_Controls.m_ShowRedGreenValues->setEnabled(false);

  this->CreateConnections();

  // let the point set widget know about the multi widget (cross hair updates)
  m_Controls.m_FixedPointListWidget->SetMultiWidget( m_MultiWidget );
  m_Controls.m_MovingPointListWidget->SetMultiWidget( m_MultiWidget );
}

void QmitkPointBasedRegistrationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_Parent->setEnabled(true);
  m_MultiWidget = &stdMultiWidget;
  m_MultiWidget->SetWidgetPlanesVisibility(true);
  m_Controls.m_FixedPointListWidget->SetMultiWidget( m_MultiWidget );
  m_Controls.m_MovingPointListWidget->SetMultiWidget( m_MultiWidget );
}

void QmitkPointBasedRegistrationView::StdMultiWidgetNotAvailable()
{
  m_Parent->setEnabled(false);
  m_MultiWidget = NULL;
  m_Controls.m_FixedPointListWidget->SetMultiWidget( NULL );
  m_Controls.m_MovingPointListWidget->SetMultiWidget( NULL );
}

void QmitkPointBasedRegistrationView::CreateConnections()
{
  connect( (QObject*)(m_Controls.m_FixedPointListWidget), SIGNAL(EditPointSets(bool)), (QObject*)(m_Controls.m_MovingPointListWidget), SLOT(DeactivateInteractor(bool)));
  connect( (QObject*)(m_Controls.m_MovingPointListWidget), SIGNAL(EditPointSets(bool)), (QObject*)(m_Controls.m_FixedPointListWidget), SLOT(DeactivateInteractor(bool)));
  connect( (QObject*)(m_Controls.m_FixedPointListWidget), SIGNAL(EditPointSets(bool)), this, SLOT(HideMovingImage(bool)));
  connect( (QObject*)(m_Controls.m_MovingPointListWidget), SIGNAL(EditPointSets(bool)), this, SLOT(HideFixedImage(bool)));
  connect( (QObject*)(m_Controls.m_FixedPointListWidget), SIGNAL(PointListChanged()), this, SLOT(updateFixedLandmarksList()));
  connect( (QObject*)(m_Controls.m_MovingPointListWidget), SIGNAL(PointListChanged()), this, SLOT(updateMovingLandmarksList()));
  connect((QObject*)(m_Controls.m_Calculate),SIGNAL(clicked()),this,SLOT(calculate()));
  connect((QObject*)(m_Controls.m_SwitchImages),SIGNAL(clicked()),this,SLOT(SwitchImages()));
  connect((QObject*)(m_Controls.m_UndoTransformation),SIGNAL(clicked()),this,SLOT(UndoTransformation()));
  connect((QObject*)(m_Controls.m_RedoTransformation),SIGNAL(clicked()),this,SLOT(RedoTransformation()));
  connect((QObject*)(m_Controls.m_ShowRedGreenValues),SIGNAL(toggled(bool)),this,SLOT(showRedGreen(bool)));
  connect((QObject*)(m_Controls.m_OpacitySlider),SIGNAL(sliderMoved(int)),this,SLOT(OpacityUpdate(int)));
  connect((QObject*)(m_Controls.m_SelectedTransformationClass),SIGNAL(activated(int)), this,SLOT(transformationChanged(int)));
  connect((QObject*)(m_Controls.m_UseICP),SIGNAL(toggled(bool)), this,SLOT(checkCalculateEnabled()));
  connect((QObject*)(m_Controls.m_UseICP),SIGNAL(toggled(bool)), this,SLOT(checkLandmarkError()));
}

void QmitkPointBasedRegistrationView::Activated()
{

  m_Deactivated = false;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Activated();
  this->clearTransformationLists();
  if (m_SelListener.IsNull())
  {
    m_SelListener = cherry::ISelectionListener::Pointer(new SelListenerPointBasedRegistration(this));
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
    cherry::ISelection::ConstPointer sel(
      this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
    m_CurrentSelection = sel.Cast<const IStructuredSelection>();
    m_SelListener.Cast<SelListenerPointBasedRegistration>()->DoSelectionChanged(sel);
  }
  this->OpacityUpdate(m_Controls.m_OpacitySlider->value());
  this->showRedGreen(m_Controls.m_ShowRedGreenValues->isChecked());
  
}

void QmitkPointBasedRegistrationView::Visible()
{
  /*
  m_Deactivated = false;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Activated();
  this->clearTransformationLists();
  if (m_SelListener.IsNull())
  {
    m_SelListener = cherry::ISelectionListener::Pointer(new SelListenerPointBasedRegistration(this));
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",* / m_SelListener);
    cherry::ISelection::ConstPointer sel(
      this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
    m_CurrentSelection = sel.Cast<const IStructuredSelection>();
    m_SelListener.Cast<SelListenerPointBasedRegistration>()->DoSelectionChanged(sel);
  }
  this->OpacityUpdate(m_Controls.m_OpacitySlider->value());
  this->showRedGreen(m_Controls.m_ShowRedGreenValues->isChecked());*/
}

void QmitkPointBasedRegistrationView::Deactivated()
{
  m_Deactivated = true;
  if (m_FixedPointSetNode.IsNotNull())
    m_FixedPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldFixedLabel));
  m_Controls.m_FixedPointListWidget->SetPointSetNode(NULL);
  m_Controls.m_FixedPointListWidget->DeactivateInteractor(true);
  if (m_MovingPointSetNode.IsNotNull())
    m_MovingPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldMovingLabel));
  m_Controls.m_MovingPointListWidget->SetPointSetNode(NULL);
  m_Controls.m_MovingPointListWidget->DeactivateInteractor(true);
  this->setImageColor(false);
  if (m_FixedNode.IsNotNull())
    m_FixedNode->SetOpacity(1.0);
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_OriginalOpacity);
  }
  this->clearTransformationLists();
  if (m_FixedPointSetNode.IsNotNull() && m_FixedLandmarks.IsNotNull() && m_FixedLandmarks->GetSize() == 0)
  {
    this->GetDataStorage()->Remove(m_FixedPointSetNode);
  }
  if (m_MovingPointSetNode.IsNotNull() && m_MovingLandmarks.IsNotNull() && m_MovingLandmarks->GetSize() == 0)
  {
    this->GetDataStorage()->Remove(m_MovingPointSetNode);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  m_FixedNode = NULL;
  m_MovingNode = NULL;
  if(m_FixedLandmarks.IsNotNull())
    m_FixedLandmarks->RemoveObserver(m_CurrentFixedLandmarksObserverID);
  m_FixedLandmarks = NULL;
  if(m_MovingLandmarks.IsNotNull())
    m_MovingLandmarks->RemoveObserver(m_CurrentMovingLandmarksObserverID);
  m_MovingLandmarks = NULL;
  m_FixedPointSetNode = NULL;
  m_MovingPointSetNode = NULL;
  m_Controls.m_FixedLabel->hide();
  m_Controls.TextLabelFixed->hide();
  m_Controls.line2->hide();
  m_Controls.m_FixedPointListWidget->hide();
  m_Controls.m_MovingLabel->hide();
  m_Controls.TextLabelMoving->hide();
  m_Controls.line1->hide();
  m_Controls.m_MovingPointListWidget->hide();
  m_Controls.m_OpacityLabel->hide();
  m_Controls.m_OpacitySlider->hide();
  m_Controls.m_SwitchImages->hide();
  cherry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->RemovePostSelectionListener(m_SelListener);
  m_SelListener = NULL;
  
}

void QmitkPointBasedRegistrationView::Hidden()
{
  /*
  m_Deactivated = true;
  if (m_FixedPointSetNode.IsNotNull())
    m_FixedPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldFixedLabel));
  m_Controls.m_FixedPointListWidget->SetPointSetNode(NULL);
  m_Controls.m_FixedPointListWidget->DeactivateInteractor(true);
  if (m_MovingPointSetNode.IsNotNull())
    m_MovingPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldMovingLabel));
  m_Controls.m_MovingPointListWidget->SetPointSetNode(NULL);
  m_Controls.m_MovingPointListWidget->DeactivateInteractor(true);
  this->setImageColor(false);
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_OriginalOpacity);
  }
  this->clearTransformationLists();
  if (m_FixedPointSetNode.IsNotNull() && m_FixedLandmarks.IsNotNull() && m_FixedLandmarks->GetSize() == 0)
  {
    this->GetDataStorage()->Remove(m_FixedPointSetNode);
  }
  if (m_MovingPointSetNode.IsNotNull() && m_MovingLandmarks.IsNotNull() && m_MovingLandmarks->GetSize() == 0)
  {
    this->GetDataStorage()->Remove(m_MovingPointSetNode);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  m_FixedNode = NULL;
  m_MovingNode = NULL;
  if(m_FixedLandmarks.IsNotNull())
    m_FixedLandmarks->RemoveObserver(m_CurrentFixedLandmarksObserverID);
  m_FixedLandmarks = NULL;
  if(m_MovingLandmarks.IsNotNull())
    m_MovingLandmarks->RemoveObserver(m_CurrentMovingLandmarksObserverID);
  m_MovingLandmarks = NULL;
  m_FixedPointSetNode = NULL;
  m_MovingPointSetNode = NULL;
  m_Controls.m_FixedLabel->hide();
  m_Controls.TextLabelFixed->hide();
  m_Controls.line2->hide();
  m_Controls.m_FixedPointListWidget->hide();
  m_Controls.m_MovingLabel->hide();
  m_Controls.TextLabelMoving->hide();
  m_Controls.line1->hide();
  m_Controls.m_MovingPointListWidget->hide();
  m_Controls.m_OpacityLabel->hide();
  m_Controls.m_OpacitySlider->hide();
  m_Controls.m_SwitchImages->hide();
  cherry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->RemovePostSelectionListener(m_SelListener);
  m_SelListener = NULL;
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  //QmitkFunctionality::Deactivated();*/
}

void QmitkPointBasedRegistrationView::FixedSelected(mitk::DataTreeNode::Pointer fixedImage)
{
  if(m_FixedLandmarks.IsNotNull())
    m_FixedLandmarks->RemoveObserver(m_CurrentFixedLandmarksObserverID);
  if (fixedImage.IsNotNull())
  {
    if (m_FixedNode != fixedImage)
    {
      // remove changes on previous selected node
      if (m_FixedNode.IsNotNull())
      {
        this->setImageColor(false);
        m_FixedNode->SetOpacity(1.0);
        if (m_FixedPointSetNode.IsNotNull())
        {
          m_FixedPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldFixedLabel));
        }
      }
      // get selected node
      m_FixedNode = fixedImage;
      m_FixedNode->SetOpacity(0.5);
      m_FixedNode->SetVisibility(true);
      m_Controls.m_FixedLabel->setText(QString::fromStdString(m_FixedNode->GetName()));
      m_Controls.m_FixedLabel->show();
      m_Controls.m_SwitchImages->show();
      m_Controls.TextLabelFixed->show();
      m_Controls.line2->show();
      m_Controls.m_FixedPointListWidget->show();
      mitk::ColorProperty::Pointer colorProperty;
      colorProperty = dynamic_cast<mitk::ColorProperty*>(m_FixedNode->GetProperty("color"));
      if ( colorProperty.IsNotNull() )
      {
        m_FixedColor = colorProperty->GetColor();
      }
      this->setImageColor(m_ShowRedGreen);

      bool hasPointSetNode = false;
      mitk::DataStorage::SetOfObjects::ConstPointer children = this->GetDataStorage()->GetDerivations(m_FixedNode);
      unsigned long size;
      size = children->Size();
      for (unsigned long i = 0; i < size; ++i)
      {
        mitk::StringProperty::Pointer nameProp = dynamic_cast<mitk::StringProperty*>(children->GetElement(i)->GetProperty("name"));
        if(nameProp.IsNotNull() && nameProp->GetValueAsString()=="PointBasedRegistrationNode")
        {
          m_FixedPointSetNode=children->GetElement(i);
          m_FixedLandmarks = dynamic_cast<mitk::PointSet*> (m_FixedPointSetNode->GetData());
          this->GetDataStorage()->Remove(m_FixedPointSetNode);
          hasPointSetNode = true;
          break;
        }
      }
      if (!hasPointSetNode)
      {
        m_FixedLandmarks = mitk::PointSet::New();
        m_FixedPointSetNode = mitk::DataTreeNode::New();
        m_FixedPointSetNode->SetData(m_FixedLandmarks);
        m_FixedPointSetNode->SetProperty("name", mitk::StringProperty::New("PointBasedRegistrationNode"));
      }
      m_FixedPointSetNode->GetStringProperty("label", m_OldFixedLabel);
      m_FixedPointSetNode->SetProperty("label", mitk::StringProperty::New("F "));
      m_FixedPointSetNode->SetProperty("color", mitk::ColorProperty::New(0.0f, 1.0f, 1.0f));
      m_FixedPointSetNode->SetVisibility(true);
      m_Controls.m_FixedPointListWidget->SetPointSetNode(m_FixedPointSetNode);
      this->GetDataStorage()->Add(m_FixedPointSetNode, m_FixedNode);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  else
  {
    m_FixedNode = NULL;
    if (m_FixedPointSetNode.IsNotNull())
      m_FixedPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldFixedLabel));
    m_FixedPointSetNode = NULL;
    m_FixedLandmarks = NULL;
    m_Controls.m_FixedPointListWidget->SetPointSetNode(m_FixedPointSetNode);
    m_Controls.m_FixedLabel->hide();
    m_Controls.TextLabelFixed->hide();
    m_Controls.line2->hide();
    m_Controls.m_FixedPointListWidget->hide();
    m_Controls.m_SwitchImages->hide();
  }
  if(m_FixedLandmarks.IsNotNull())
    m_CurrentFixedLandmarksObserverID = m_FixedLandmarks->AddObserver(itk::ModifiedEvent(), m_FixedLandmarksChangedCommand);
}

void QmitkPointBasedRegistrationView::MovingSelected(mitk::DataTreeNode::Pointer movingImage)
{
  if(m_MovingLandmarks.IsNotNull())
    m_MovingLandmarks->RemoveObserver(m_CurrentMovingLandmarksObserverID);
  if (movingImage.IsNotNull())
  {
    if (m_MovingNode != movingImage)
    {
      if (m_MovingNode.IsNotNull())
      {
        m_MovingNode->SetOpacity(m_OriginalOpacity);
        if (m_FixedNode == m_MovingNode)
          m_FixedNode->SetOpacity(0.5);
        this->setImageColor(false);
        if (m_MovingNode != m_FixedNode)
        {
          m_MovingPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldMovingLabel));
        }
        else
        {
          m_OldFixedLabel = m_OldMovingLabel;
        }
      }
      if (m_MovingPointSetNode.IsNotNull())
        m_MovingPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldMovingLabel));
      m_MovingNode = movingImage;
      m_MovingNode->SetVisibility(true);
      m_Controls.m_MovingLabel->setText(QString::fromStdString(m_MovingNode->GetName()));
      m_Controls.m_MovingLabel->show();
      m_Controls.TextLabelMoving->show();
      m_Controls.line1->show();
      m_Controls.m_MovingPointListWidget->show();
      m_Controls.m_OpacityLabel->show();
      m_Controls.m_OpacitySlider->show();
      mitk::ColorProperty::Pointer colorProperty;
      colorProperty = dynamic_cast<mitk::ColorProperty*>(m_MovingNode->GetProperty("color"));
      if ( colorProperty.IsNotNull() )
      {
        m_MovingColor = colorProperty->GetColor();
      }
      this->setImageColor(m_ShowRedGreen);
      m_MovingNode->GetFloatProperty("opacity", m_OriginalOpacity);
      this->OpacityUpdate(m_Opacity);
      bool hasPointSetNode = false;
      mitk::DataStorage::SetOfObjects::ConstPointer children = this->GetDataStorage()->GetDerivations(m_MovingNode);
      unsigned long size;
      size = children->Size();
      for (unsigned long i = 0; i < size; ++i)
      {
        mitk::StringProperty::Pointer nameProp = dynamic_cast<mitk::StringProperty*>(children->GetElement(i)->GetProperty("name"));
        if(nameProp.IsNotNull() && nameProp->GetValueAsString()=="PointBasedRegistrationNode")
        {
          m_MovingPointSetNode=children->GetElement(i);
          m_MovingLandmarks = dynamic_cast<mitk::PointSet*> (m_MovingPointSetNode->GetData());
          this->GetDataStorage()->Remove(m_MovingPointSetNode);
          hasPointSetNode = true;
          break;
        }
      }
      if (!hasPointSetNode)
      {
        m_MovingLandmarks = mitk::PointSet::New();
        m_MovingPointSetNode = mitk::DataTreeNode::New();
        m_MovingPointSetNode->SetData(m_MovingLandmarks);
        m_MovingPointSetNode->SetProperty("name", mitk::StringProperty::New("PointBasedRegistrationNode"));
      }
      this->GetDataStorage()->Add(m_MovingPointSetNode, m_MovingNode);
      m_MovingPointSetNode->GetStringProperty("label", m_OldMovingLabel);
      m_MovingPointSetNode->SetProperty("label", mitk::StringProperty::New("M "));
      m_MovingPointSetNode->SetProperty("color", mitk::ColorProperty::New(1.0f, 1.0f, 0.0f));
      m_MovingPointSetNode->SetVisibility(true);
      m_Controls.m_MovingPointListWidget->SetPointSetNode(m_MovingPointSetNode);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      this->clearTransformationLists();
      this->OpacityUpdate(m_Opacity);
    }
  }
  else
  {
    m_MovingNode = NULL;
    if (m_MovingPointSetNode.IsNotNull())
      m_MovingPointSetNode->SetProperty("label", mitk::StringProperty::New(m_OldMovingLabel));
    m_MovingPointSetNode = NULL;
    m_MovingLandmarks = NULL;
    m_Controls.m_MovingPointListWidget->SetPointSetNode(m_MovingPointSetNode);
    m_Controls.m_MovingLabel->hide();
    m_Controls.TextLabelMoving->hide();
    m_Controls.line1->hide();
    m_Controls.m_MovingPointListWidget->hide();
    m_Controls.m_OpacityLabel->hide();
    m_Controls.m_OpacitySlider->hide();
  }
  if(m_MovingLandmarks.IsNotNull())
    m_CurrentMovingLandmarksObserverID = m_MovingLandmarks->AddObserver(itk::ModifiedEvent(), m_MovingLandmarksChangedCommand);
}

void QmitkPointBasedRegistrationView::updateMovingLandmarksList()
{
  m_MovingLandmarks = (mitk::PointSet*)m_MovingPointSetNode->GetData();
  this->checkLandmarkError();
  this->CheckCalculate();
}

void QmitkPointBasedRegistrationView::updateFixedLandmarksList()
{
  m_FixedLandmarks = (mitk::PointSet*)m_FixedPointSetNode->GetData();
  this->checkLandmarkError();
  this->CheckCalculate();
}

void QmitkPointBasedRegistrationView::HideFixedImage(bool hide)
{
  m_HideFixedImage = hide;
  if(m_FixedNode.IsNotNull())
  {
    m_FixedNode->SetVisibility(!hide);
  }
  if (hide)
  {
    //this->reinitMovingClicked();
  }
  if (!m_HideMovingImage && !m_HideFixedImage)
  {
    //this->globalReinitClicked();
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPointBasedRegistrationView::HideMovingImage(bool hide)
{
  m_HideMovingImage = hide;
  if(m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetVisibility(!hide);
  }
  if (hide)
  {
    //this->reinitFixedClicked();
  }
  if (!m_HideMovingImage && !m_HideFixedImage)
  {
    //this->globalReinitClicked();
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

bool QmitkPointBasedRegistrationView::CheckCalculate()
{
  if((m_MovingPointSetNode.IsNull())||(m_FixedPointSetNode.IsNull()||m_FixedLandmarks.IsNull()||m_MovingLandmarks.IsNull()))
    return false;
  if(m_MovingNode==m_FixedNode)
    return false;
  return this->checkCalculateEnabled();
}

void QmitkPointBasedRegistrationView::UndoTransformation()
{
  if(!m_UndoPointsGeometryList.empty())
  {
    mitk::AffineGeometryFrame3D::Pointer movingLandmarksGeometry = m_MovingLandmarks->GetGeometry(0)->Clone();
    m_RedoPointsGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingLandmarksGeometry.GetPointer()));
    m_MovingLandmarks->SetGeometry(m_UndoPointsGeometryList.back());
    m_UndoPointsGeometryList.pop_back();
    //\FIXME when geometry is substituted the matrix referenced by the actor created by the mapper
    //is still pointing to the old one. Workaround: delete mapper
    m_MovingPointSetNode->SetMapper(1, NULL);

    mitk::BaseData::Pointer movingData = m_MovingNode->GetData();
    mitk::AffineGeometryFrame3D::Pointer movingGeometry = movingData->GetGeometry(0)->Clone();
    m_RedoGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingGeometry.GetPointer()));
    movingData->SetGeometry(m_UndoGeometryList.back());
    m_UndoGeometryList.pop_back();
    //\FIXME when geometry is substituted the matrix referenced by the actor created by the mapper
    //is still pointing to the old one. Workaround: delete mapper
    m_MovingNode->SetMapper(1, NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_MultiWidget->mitkWidget4->GetRenderWindow());

    movingData->GetTimeSlicedGeometry()->UpdateInformation();
    m_MovingLandmarks->GetTimeSlicedGeometry()->UpdateInformation();
    m_Controls.m_RedoTransformation->setEnabled(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    this->checkLandmarkError();
  }
  if(!m_UndoPointsGeometryList.empty())
  {
    m_Controls.m_UndoTransformation->setEnabled(true);
  }
  else
  {
    m_Controls.m_UndoTransformation->setEnabled(false);
  }
}

void QmitkPointBasedRegistrationView::RedoTransformation()
{
  if(!m_RedoPointsGeometryList.empty())
  {
    mitk::AffineGeometryFrame3D::Pointer movingLandmarksGeometry = m_MovingLandmarks->GetGeometry(0)->Clone();
    m_UndoPointsGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingLandmarksGeometry.GetPointer()));
    m_MovingLandmarks->SetGeometry(m_RedoPointsGeometryList.back());
    m_RedoPointsGeometryList.pop_back();
    //\FIXME when geometry is substituted the matrix referenced by the actor created by the mapper
    //is still pointing to the old one. Workaround: delete mapper
    m_MovingPointSetNode->SetMapper(1, NULL);

    mitk::BaseData::Pointer movingData = m_MovingNode->GetData();
    mitk::AffineGeometryFrame3D::Pointer movingGeometry = movingData->GetGeometry(0)->Clone();
    m_UndoGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingGeometry.GetPointer()));
    movingData->SetGeometry(m_RedoGeometryList.back());
    m_RedoGeometryList.pop_back();
    //\FIXME when geometry is substituted the matrix referenced by the actor created by the mapper
    //is still pointing to the old one. Workaround: delete mapper
    m_MovingNode->SetMapper(1, NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_MultiWidget->mitkWidget4->GetRenderWindow());

    movingData->GetTimeSlicedGeometry()->UpdateInformation();
    m_MovingLandmarks->GetTimeSlicedGeometry()->UpdateInformation();
    m_Controls.m_UndoTransformation->setEnabled(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    this->checkLandmarkError();
  }
  if(!m_RedoPointsGeometryList.empty())
  {
    m_Controls.m_RedoTransformation->setEnabled(true);
  }
  else
  {
    m_Controls.m_RedoTransformation->setEnabled(false);
  }
}

void QmitkPointBasedRegistrationView::showRedGreen(bool redGreen)
{
  m_ShowRedGreen = redGreen;
  this->setImageColor(m_ShowRedGreen);
}

void QmitkPointBasedRegistrationView::setImageColor(bool redGreen)
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

void QmitkPointBasedRegistrationView::OpacityUpdate(float opacity)
{
  if (opacity > 1)
  {
    opacity = opacity/100.0f;
  }
  m_Opacity = opacity;
  if (m_MovingNode.IsNotNull())
  {
    m_MovingNode->SetOpacity(m_Opacity);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPointBasedRegistrationView::OpacityUpdate(int opacity)
{
  float fValue = ((float)opacity)/100.0f;
  this->OpacityUpdate(fValue);
}

void QmitkPointBasedRegistrationView::clearTransformationLists()
{
  m_Controls.m_UndoTransformation->setEnabled(false);
  m_Controls.m_RedoTransformation->setEnabled(false);
  m_Controls.m_MeanErrorLCD->hide();
  m_Controls.m_MeanError->hide();
  m_UndoGeometryList.clear();
  m_UndoPointsGeometryList.clear();
  m_RedoGeometryList.clear();
  m_RedoPointsGeometryList.clear();
}

void QmitkPointBasedRegistrationView::checkLandmarkError()
{
  double totalDist = 0, dist = 0, dist2 = 0;
  mitk::Point3D point1, point2, point3;
  double p1[3], p2[3];
  if(m_Transformation < 3)
  {
    if (m_Controls.m_UseICP->isChecked())
    {
      if (m_MovingLandmarks.IsNotNull() && m_FixedLandmarks.IsNotNull()&& m_MovingLandmarks->GetSize() != 0 && m_FixedLandmarks->GetSize() != 0)
      {
        for(int pointId = 0; pointId < m_MovingLandmarks->GetSize(); ++pointId)
        {
          point1 = m_MovingLandmarks->GetPoint(pointId);
          point2 = m_FixedLandmarks->GetPoint(0);
          p1[0] = point1[0]; p1[1] = point1[1]; p1[2] = point1[2];
          p2[0] = point2[0]; p2[1] = point2[1]; p2[2] = point2[2];
          dist = vtkMath::Distance2BetweenPoints(p1, p2);
          for(int pointId2 = 1; pointId2 < m_FixedLandmarks->GetSize(); ++pointId2)
          {
            point2 = m_FixedLandmarks->GetPoint(pointId2);
            p1[0] = point1[0]; p1[1] = point1[1]; p1[2] = p1[2];
            p2[0] = point2[0]; p2[1] = point2[1]; p2[2] = p2[2];
            dist2 = vtkMath::Distance2BetweenPoints(p1, p2);
            if (dist2 < dist)
            {
              dist = dist2;
            }
          }
          totalDist += dist;
        }
        m_Controls.m_MeanErrorLCD->display(sqrt(totalDist/m_FixedLandmarks->GetSize()));
        m_Controls.m_MeanErrorLCD->show();
        m_Controls.m_MeanError->show();
      }
      else
      {
        m_Controls.m_MeanErrorLCD->hide();
        m_Controls.m_MeanError->hide();
      }
    }
    else
    {
      if (m_MovingLandmarks.IsNotNull() && m_FixedLandmarks.IsNotNull() && m_MovingLandmarks->GetSize() != 0 && m_FixedLandmarks->GetSize() != 0 && m_MovingLandmarks->GetSize() == m_FixedLandmarks->GetSize())
      {
        for(int pointId = 0; pointId < m_MovingLandmarks->GetSize(); ++pointId)
        {
          point1 = m_MovingLandmarks->GetPoint(pointId);
          point2 = m_FixedLandmarks->GetPoint(pointId);
          p1[0] = point1[0]; p1[1] = point1[1]; p1[2] = point1[2];
          p2[0] = point2[0]; p2[1] = point2[1]; p2[2] = point2[2];
          totalDist += vtkMath::Distance2BetweenPoints(p1, p2);
        }
        m_Controls.m_MeanErrorLCD->display(sqrt(totalDist/m_FixedLandmarks->GetSize()));
        m_Controls.m_MeanErrorLCD->show();
        m_Controls.m_MeanError->show();
      }
      else
      {
        m_Controls.m_MeanErrorLCD->hide();
        m_Controls.m_MeanError->hide();
      }
    }
  }
  else
  {
    if (m_MovingLandmarks.IsNotNull() && m_FixedLandmarks.IsNotNull() && m_MovingLandmarks->GetSize() != 0 && m_FixedLandmarks->GetSize() != 0 && m_MovingLandmarks->GetSize() == m_FixedLandmarks->GetSize())
    {
      for(int pointId = 0; pointId < m_MovingLandmarks->GetSize(); ++pointId)
      {
        point1 = m_MovingLandmarks->GetPoint(pointId);
        point2 = m_FixedLandmarks->GetPoint(pointId);
        p1[0] = point1[0]; p1[1] = point1[1]; p1[2] = point1[2];
        p2[0] = point2[0]; p2[1] = point2[1]; p2[2] = point2[2];
        totalDist += vtkMath::Distance2BetweenPoints(p1, p2);
      }
      m_Controls.m_MeanErrorLCD->display(sqrt(totalDist/m_FixedLandmarks->GetSize()));
      m_Controls.m_MeanErrorLCD->show();
      m_Controls.m_MeanError->show();
    }
    else
    {
      m_Controls.m_MeanErrorLCD->hide();
      m_Controls.m_MeanError->hide();
    }
  }
}

void QmitkPointBasedRegistrationView::transformationChanged(int transform)
{
  m_Transformation = transform;
  this->checkCalculateEnabled();
  this->checkLandmarkError();
}

// ICP with vtkLandmarkTransformation
void QmitkPointBasedRegistrationView::calculateLandmarkbasedWithICP()
{
  if(CheckCalculate())
  {
    mitk::Geometry3D::Pointer pointsGeometry = m_MovingLandmarks->GetGeometry(0);
    mitk::AffineGeometryFrame3D::Pointer movingLandmarksGeometry = m_MovingLandmarks->GetGeometry(0)->Clone();
    m_UndoPointsGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingLandmarksGeometry.GetPointer()));

    mitk::BaseData::Pointer originalData = m_MovingNode->GetData();
    mitk::AffineGeometryFrame3D::Pointer originalDataGeometry = originalData->GetGeometry(0)->Clone();
    m_UndoGeometryList.push_back(static_cast<mitk::Geometry3D *>(originalDataGeometry.GetPointer()));

    vtkIdType pointId;
    vtkPoints* vPointsSource=vtkPoints::New();
    vtkCellArray* vCellsSource=vtkCellArray::New();
    for(pointId=0; pointId<m_MovingLandmarks->GetSize();++pointId)
    {
      mitk::Point3D pointSource=m_MovingLandmarks->GetPoint(pointId);
      vPointsSource->InsertNextPoint(pointSource[0],pointSource[1],pointSource[2]);
      vCellsSource->InsertNextCell(1, &pointId);
    }

    vtkPoints* vPointsTarget=vtkPoints::New();
    vtkCellArray* vCellsTarget = vtkCellArray::New();
    for(pointId=0; pointId<m_FixedLandmarks->GetSize();++pointId)
    {
      mitk::Point3D pointTarget=m_FixedLandmarks->GetPoint(pointId);
      vPointsTarget->InsertNextPoint(pointTarget[0],pointTarget[1],pointTarget[2]);
      vCellsTarget->InsertNextCell(1, &pointId);
    }

    vtkPolyData* vPointSetSource=vtkPolyData::New();
    vtkPolyData* vPointSetTarget=vtkPolyData::New();

    vPointSetTarget->SetPoints(vPointsTarget);
    vPointSetTarget->SetVerts(vCellsTarget);
    vPointSetSource->SetPoints(vPointsSource);
    vPointSetSource->SetVerts(vCellsSource);

    vtkIterativeClosestPointTransform * icp=vtkIterativeClosestPointTransform::New();
    icp->SetCheckMeanDistance(1);
    icp->SetSource(vPointSetSource);
    icp->SetTarget(vPointSetTarget);
    icp->SetMaximumNumberOfIterations(50);
    icp->StartByMatchingCentroidsOn();
    vtkLandmarkTransform * transform=icp->GetLandmarkTransform();
    if(m_Transformation==0)
    {
      transform->SetModeToRigidBody();
    }
    if(m_Transformation==1)
    {
      transform->SetModeToSimilarity();
    }
    if(m_Transformation==2)
    {
      transform->SetModeToAffine();
    }

    vtkMatrix4x4 * matrix=icp->GetMatrix();

    double determinant = fabs(matrix->Determinant());
    if((determinant < mitk::eps) || (determinant > 100) || (determinant < 0.01)
      || (determinant==itk::NumericTraits<double>::infinity())
      || (determinant==itk::NumericTraits<double>::quiet_NaN())
      || (determinant==itk::NumericTraits<double>::signaling_NaN())
      || (determinant==-itk::NumericTraits<double>::infinity())
      || (determinant==-itk::NumericTraits<double>::quiet_NaN())
      || (determinant==-itk::NumericTraits<double>::signaling_NaN())
      || (!(determinant <= 0) && !(determinant > 0)))
    {
      QMessageBox msgBox;
      msgBox.setText("Suspicious determinant of matrix calculated by ICP.\n"
        "Please select more points or other points!" );
      msgBox.exec();
      return;
    }

    pointsGeometry->Compose(matrix);
    m_MovingLandmarks->GetTimeSlicedGeometry()->UpdateInformation();

    mitk::BaseData::Pointer movingData = m_MovingNode->GetData();
    mitk::Geometry3D::Pointer movingGeometry = movingData->GetGeometry(0);
    movingGeometry->Compose(matrix);
    movingData->GetTimeSlicedGeometry()->UpdateInformation();
    m_Controls.m_UndoTransformation->setEnabled(true);
    m_Controls.m_RedoTransformation->setEnabled(false);
    m_RedoGeometryList.clear();
    m_RedoPointsGeometryList.clear();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    this->checkLandmarkError();
  }
}

// only vtkLandmarkTransformation
void QmitkPointBasedRegistrationView::calculateLandmarkbased()
{
  if(CheckCalculate())
  {
    mitk::Geometry3D::Pointer pointsGeometry = m_MovingLandmarks->GetGeometry(0);
    mitk::AffineGeometryFrame3D::Pointer movingLandmarksGeometry = m_MovingLandmarks->GetGeometry(0)->Clone();
    m_UndoPointsGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingLandmarksGeometry.GetPointer()));

    mitk::BaseData::Pointer originalData = m_MovingNode->GetData();
    mitk::AffineGeometryFrame3D::Pointer originalDataGeometry = originalData->GetGeometry(0)->Clone();
    m_UndoGeometryList.push_back(static_cast<mitk::Geometry3D *>(originalDataGeometry.GetPointer()));

    vtkIdType pointId;
    vtkPoints* vPointsSource=vtkPoints::New();
    for(pointId = 0; pointId < m_MovingLandmarks->GetSize(); ++pointId)
    {
      mitk::Point3D sourcePoint = m_MovingLandmarks->GetPoint(pointId);
      vPointsSource->InsertNextPoint(sourcePoint[0],sourcePoint[1],sourcePoint[2]);
    }
    vtkPoints* vPointsTarget=vtkPoints::New();
    for(pointId=0; pointId<m_FixedLandmarks->GetSize();++pointId)
    {
      mitk::Point3D targetPoint=m_FixedLandmarks->GetPoint(pointId);
      vPointsTarget->InsertNextPoint(targetPoint[0],targetPoint[1],targetPoint[2]);
    }

    vtkLandmarkTransform * transform= vtkLandmarkTransform::New();
    transform->SetSourceLandmarks(vPointsSource);
    transform->SetTargetLandmarks(vPointsTarget);
    if(m_Transformation==0)
    {
      transform->SetModeToRigidBody();
    }
    if(m_Transformation==1)
    {
      transform->SetModeToSimilarity();
    }
    if(m_Transformation==2)
    {
      transform->SetModeToAffine();
    }
    vtkMatrix4x4 * matrix=transform->GetMatrix();
    double determinant = fabs(matrix->Determinant());
    if((determinant < mitk::eps) || (determinant > 100) || (determinant < 0.01)
      || (determinant==itk::NumericTraits<double>::infinity())
      || (determinant==itk::NumericTraits<double>::quiet_NaN())
      || (determinant==itk::NumericTraits<double>::signaling_NaN())
      || (determinant==-itk::NumericTraits<double>::infinity())
      || (determinant==-itk::NumericTraits<double>::quiet_NaN())
      || (determinant==-itk::NumericTraits<double>::signaling_NaN())
      || (!(determinant <= 0) && !(determinant > 0)))
    {
      QMessageBox msgBox;
      msgBox.setText("Suspicious determinant of matrix calculated.\n"
        "Please select more points or other points!" );
      msgBox.exec();
      return;
    }
    pointsGeometry->Compose(matrix);
    m_MovingLandmarks->GetTimeSlicedGeometry()->UpdateInformation();

    mitk::BaseData::Pointer movingData = m_MovingNode->GetData();
    mitk::Geometry3D::Pointer movingGeometry = movingData->GetGeometry(0);
    movingGeometry->Compose(matrix);
    movingData->GetTimeSlicedGeometry()->UpdateInformation();
    m_Controls.m_UndoTransformation->setEnabled(true);
    m_Controls.m_RedoTransformation->setEnabled(false);
    m_RedoGeometryList.clear();
    m_RedoPointsGeometryList.clear();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    this->checkLandmarkError();
  }
}

void QmitkPointBasedRegistrationView::calculateLandmarkWarping()
{
  LandmarkWarping* registration = new LandmarkWarping();

  LandmarkWarping::FixedImageType::Pointer fixedImage = LandmarkWarping::FixedImageType::New();
  mitk::Image::Pointer fimage = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
  if (fimage.IsNotNull() && (fimage->GetDimension() == 2 || fimage->GetDimension() == 3))
  {
    if(fimage.IsNotNull())
    {
      mitk::CastToItkImage(fimage, fixedImage);
    }
    LandmarkWarping::MovingImageType::Pointer movingImage = LandmarkWarping::MovingImageType::New();
    mitk::Image::Pointer mimage = dynamic_cast<mitk::Image*>(m_MovingNode->GetData());
    if(mimage.IsNotNull())
    {
      mitk::CastToItkImage(mimage, movingImage);
    }
    registration->SetFixedImage(fixedImage);
    registration->SetMovingImage(movingImage);
    unsigned int pointId;
    mitk::Point3D sourcePoint, targetPoint;
    LandmarkWarping::LandmarkContainerType::Pointer fixedLandmarks = LandmarkWarping::LandmarkContainerType::New();
    LandmarkWarping::LandmarkPointType point;
    for(pointId = 0; pointId < (unsigned int)m_FixedLandmarks->GetSize(); ++pointId)
    {
      fimage->GetGeometry(0)->WorldToItkPhysicalPoint(m_FixedLandmarks->GetPoint(pointId), point);
      fixedLandmarks->InsertElement( pointId, point);
    }
    LandmarkWarping::LandmarkContainerType::Pointer movingLandmarks = LandmarkWarping::LandmarkContainerType::New();
    for(pointId = 0; pointId < (unsigned int)m_MovingLandmarks->GetSize(); ++pointId)
    {
      mitk::BaseData::Pointer fixedData = m_FixedNode->GetData();
      mitk::Geometry3D::Pointer fixedGeometry = fixedData->GetGeometry(0);
      fixedGeometry->WorldToItkPhysicalPoint(m_MovingLandmarks->GetPoint(pointId), point);
      movingLandmarks->InsertElement( pointId, point);
    }
    registration->SetLandmarks(fixedLandmarks.GetPointer(), movingLandmarks.GetPointer());
    LandmarkWarping::MovingImageType::Pointer output = registration->Register();
    if (output.IsNotNull())
    {
      mitk::Image::Pointer image = mitk::Image::New();
      mitk::CastToMitkImage(output, image);
      m_MovingNode->SetData(image);
      mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
      mitk::LevelWindow levelWindow;
      levelWindow.SetAuto( image );
      levWinProp->SetLevelWindow(levelWindow);
      m_MovingNode->GetPropertyList()->SetProperty("levelwindow",levWinProp);
      movingLandmarks = registration->GetTransformedTargetLandmarks();
      mitk::PointSet::PointDataIterator it;
      it = m_MovingLandmarks->GetPointSet()->GetPointData()->Begin();
      //increase the eventId to encapsulate the coming operations
      mitk::OperationEvent::IncCurrObjectEventId();
      mitk::OperationEvent::ExecuteIncrement();
      for(pointId=0; pointId<movingLandmarks->Size();++pointId, ++it)
      {
        int position = it->Index();
        mitk::PointSet::PointType pt = m_MovingLandmarks->GetPoint(position);
        mitk::Point3D undoPoint = ( pt );
        point = movingLandmarks->GetElement(pointId);
        fimage->GetGeometry(0)->ItkPhysicalPointToWorld(point, pt);
        mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVE, pt, position);
        //undo operation
        mitk::PointOperation* undoOp = new mitk::PointOperation(mitk::OpMOVE, undoPoint, position);
        mitk::OperationEvent* operationEvent = new mitk::OperationEvent(m_MovingLandmarks, doOp, undoOp, "Move point");
        mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent(operationEvent);

        //execute the Operation
        m_MovingLandmarks->ExecuteOperation(doOp);
      }
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      this->clearTransformationLists();
      this->checkLandmarkError();
    }
  }
}

bool QmitkPointBasedRegistrationView::checkCalculateEnabled()
{
  if (m_FixedLandmarks.IsNotNull() && m_MovingLandmarks.IsNotNull())
  {
    int fixedPoints = m_FixedLandmarks->GetSize();
    int movingPoints = m_MovingLandmarks->GetSize();
    if (m_Transformation == 0 || m_Transformation == 1 || m_Transformation == 2)
    {
      if (m_Controls.m_UseICP->isChecked())
      {
        if((movingPoints > 0 && fixedPoints > 0))
        {
          m_Controls.m_Calculate->setEnabled(true);
          return true;
        }
        else
        {
          m_Controls.m_Calculate->setEnabled(false);
          return false;
        }
      }
      else
      {
        if ((movingPoints == fixedPoints) && movingPoints > 0)
        {
          m_Controls.m_Calculate->setEnabled(true);
          return true;
        }
        else
        {
          m_Controls.m_Calculate->setEnabled(false);
          return false;
        }
      }
    }
    else
    {
      m_Controls.m_Calculate->setEnabled(true);
      return true;
    }
  }
  else
  {
    return false;
  }
}

void QmitkPointBasedRegistrationView::calculate()
{
  if (m_Transformation == 0 || m_Transformation == 1 || m_Transformation == 2)
  {
    if (m_Controls.m_UseICP->isChecked())
    {
      if (m_MovingLandmarks->GetSize() == 1 && m_FixedLandmarks->GetSize() == 1)
      {
        this->calculateLandmarkbased();
      }
      else
      {
        this->calculateLandmarkbasedWithICP();
      }
    }
    else
    {
      this->calculateLandmarkbased();
    }
  }
  else
  {
    this->calculateLandmarkWarping();
  }
}

void QmitkPointBasedRegistrationView::SetImagesVisible(cherry::ISelection::ConstPointer selection)
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

void QmitkPointBasedRegistrationView::SwitchImages()
{
  mitk::DataTreeNode::Pointer newMoving = m_FixedNode;
  mitk::DataTreeNode::Pointer newFixed = m_MovingNode;
  this->FixedSelected(newFixed);
  this->MovingSelected(newMoving);
}


