/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkControlVisualizationPropertiesView.h"

#include "mitkNodePredicateDataType.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPage.h"
#include "berryISelectionService.h"
#include "berryConstants.h"
#include "berryPlatformUI.h"

const std::string QmitkControlVisualizationPropertiesView::VIEW_ID = "org.mitk.views.controlvisualizationpropertiesview";

using namespace berry;

struct CvpSelListener : ISelectionListener
{

  berryObjectMacro(CvpSelListener);

  CvpSelListener(QmitkControlVisualizationPropertiesView* view)
  {
    m_View = view;
  }

  void ApplySettings(mitk::DataNode::Pointer node)
  {
    bool do_vis;
    node->GetBoolProperty("VisibleOdfs", do_vis);
    if(do_vis)
    {
      m_View->m_Controls->m_VisibleOdfsON->setIcon(*m_View->m_IconGlyON);
      m_View->m_Controls->m_VisibleOdfsON->setChecked(true);
      m_View->m_GlyIsOn = true;
    }
    else
    {
      m_View->m_Controls->m_VisibleOdfsON->setIcon(*m_View->m_IconGlyOFF);
      m_View->m_Controls->m_VisibleOdfsON->setChecked(false);
      m_View->m_GlyIsOn = false;
    }

    bool tex_int;
    node->GetBoolProperty("texture interpolation", tex_int);
    if(tex_int)
    {
      m_View->m_Controls->m_TextureIntON->setIcon(*m_View->m_IconTexON);
      m_View->m_Controls->m_TextureIntON->setChecked(true);
      m_View->m_TexIsOn = true;
    }
    else
    {
      m_View->m_Controls->m_TextureIntON->setIcon(*m_View->m_IconTexOFF);
      m_View->m_Controls->m_TextureIntON->setChecked(false);
      m_View->m_TexIsOn = false;
    }

    int val;
    node->GetIntProperty("ShowMaxNumber", val);
    m_View->m_Controls->m_ShowMaxNumber->setValue(val);

    m_View->m_Controls->m_NormalizationDropdown->setCurrentIndex(dynamic_cast<mitk::EnumerationProperty*>(node->GetProperty("Normalization"))->GetValueAsId());

    float fval;
    node->GetFloatProperty("Scaling",fval);
    m_View->m_Controls->m_ScalingFactor->setValue(fval);

    m_View->m_Controls->m_AdditionalScaling->setCurrentIndex(dynamic_cast<mitk::EnumerationProperty*>(node->GetProperty("ScaleBy"))->GetValueAsId());

    node->GetFloatProperty("IndexParam1",fval);
    m_View->m_Controls->m_IndexParam1->setValue(fval);

    node->GetFloatProperty("IndexParam2",fval);
    m_View->m_Controls->m_IndexParam2->setValue(fval);
  }

  void DoSelectionChanged(ISelection::ConstPointer selection)
  {
    // save current selection in member variable
    m_View->m_CurrentSelection = selection.Cast<const IStructuredSelection>();

    m_View->m_Controls->m_VisibleOdfsON->setVisible(false);
    m_View->m_Controls->m_TextureIntON->setVisible(false);

    // do something with the selected items
    if(m_View->m_CurrentSelection)
    {
      bool foundDiffusionImage = false;
      bool foundQBIVolume = false;
      bool foundTensorVolume = false;
      bool foundImage = false;
      bool foundMultipleOdfImages = false;

      // iterate selection
      for (IStructuredSelection::iterator i = m_View->m_CurrentSelection->Begin(); 
        i != m_View->m_CurrentSelection->End(); ++i)
      {

        // extract datatree node
        if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
        {
          mitk::DataNode::Pointer node = nodeObj->GetDataNode();

          // only look at interesting types
          if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
          {
            foundDiffusionImage = true;
            bool tex_int;
            node->GetBoolProperty("texture interpolation", tex_int);
            if(tex_int)
            {
              m_View->m_Controls->m_TextureIntON->setIcon(*m_View->m_IconTexON);
              m_View->m_Controls->m_TextureIntON->setChecked(true);
              m_View->m_TexIsOn = true;
            }
            else
            {
              m_View->m_Controls->m_TextureIntON->setIcon(*m_View->m_IconTexOFF);
              m_View->m_Controls->m_TextureIntON->setChecked(false);
              m_View->m_TexIsOn = false;
            }
            int val;
            node->GetIntProperty("DisplayChannel", val);
            m_View->m_Controls->m_DisplayIndex->setValue(val);

          }

          else if(QString("QBallImage").compare(node->GetData()->GetNameOfClass())==0)
          {
            foundMultipleOdfImages = foundQBIVolume || foundTensorVolume;
            foundQBIVolume = true;
            ApplySettings(node);
          }

          else if(QString("TensorImage").compare(node->GetData()->GetNameOfClass())==0)
          {
            foundMultipleOdfImages = foundQBIVolume || foundTensorVolume;
            foundTensorVolume = true;
            ApplySettings(node);
          }

          else if(QString("Image").compare(node->GetData()->GetNameOfClass())==0)
          {
            foundImage = true;
            bool tex_int;
            node->GetBoolProperty("texture interpolation", tex_int);
            if(tex_int)
            {
              m_View->m_Controls->m_TextureIntON->setIcon(*m_View->m_IconTexON);
              m_View->m_Controls->m_TextureIntON->setChecked(true);
              m_View->m_TexIsOn = true;
            }
            else
            {
              m_View->m_Controls->m_TextureIntON->setIcon(*m_View->m_IconTexOFF);
              m_View->m_Controls->m_TextureIntON->setChecked(false);
              m_View->m_TexIsOn = false;
            }
          }
        }
      }

      m_View->m_Controls->m_DisplayIndex->setVisible(foundDiffusionImage);

      m_View->m_FoundSingleOdfImage = (foundQBIVolume || foundTensorVolume) 
        && !foundMultipleOdfImages;
      m_View->m_Controls->m_ShowMaxNumber->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->m_NormalizationDropdown->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->label->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->m_ScalingFactor->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->m_AdditionalScaling->setVisible(m_View->m_FoundSingleOdfImage);
      
      // changed for SPIE paper, Principle curvature scaling
      //m_View->m_Controls->params_frame->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->params_frame->setVisible(false);

      m_View->m_Controls->m_VisibleOdfsON->setVisible(m_View->m_FoundSingleOdfImage);

      bool foundAnyImage = foundDiffusionImage || 
        foundQBIVolume || foundTensorVolume || foundImage;
      m_View->m_Controls->m_Reinit->setVisible(foundAnyImage);
      m_View->m_Controls->m_TextureIntON->setVisible(foundAnyImage);


    }

    if(m_View->m_IsInitialized)
    {
      //m_View->GetSite()->GetWorkbenchWindow()->GetActivePage()
      //  ->HideView(IViewPart::Pointer(m_View));

      //berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()
      //  ->ShowView(QmitkControlVisualizationPropertiesView::VIEW_ID,
      //  "", berry::IWorkbenchPage::VIEW_VISIBLE);

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

  QmitkControlVisualizationPropertiesView* m_View;
};

QmitkControlVisualizationPropertiesView::QmitkControlVisualizationPropertiesView()
: QmitkFunctionality(),
m_Controls(NULL),
m_MultiWidget(NULL),
m_IconTexOFF(new QIcon(":/QmitkDiffusionImaging/texIntOFFIcon.png")),
m_IconTexON(new QIcon(":/QmitkDiffusionImaging/texIntONIcon.png")),
m_IconGlyON(new QIcon(":/QmitkDiffusionImaging/glyphson.png")),
m_IconGlyOFF(new QIcon(":/QmitkDiffusionImaging/glyphsoff.png"))
{
}

QmitkControlVisualizationPropertiesView::~QmitkControlVisualizationPropertiesView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
}

void QmitkControlVisualizationPropertiesView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkControlVisualizationPropertiesViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->params_frame->setVisible(false);

    QIcon icon5(":/QmitkDiffusionImaging/Refresh_48.png");
    m_Controls->m_Reinit->setIcon(icon5);

    m_Controls->m_TextureIntON->setCheckable(true);
    m_Controls->m_VisibleOdfsON->setCheckable(true);

  }

  m_IsInitialized = false;
  m_SelListener = berry::ISelectionListener::Pointer(new CvpSelListener(this));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<CvpSelListener>()->DoSelectionChanged(sel);
  m_IsInitialized = true;
}

void QmitkControlVisualizationPropertiesView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkControlVisualizationPropertiesView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkControlVisualizationPropertiesView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_DisplayIndex), SIGNAL(valueChanged(int)), this, SLOT(DisplayIndexChanged(int)) );
    connect( (QObject*)(m_Controls->m_TextureIntON), SIGNAL(clicked()), this, SLOT(TextIntON()) );
    connect( (QObject*)(m_Controls->m_Reinit), SIGNAL(clicked()), this, SLOT(Reinit()) );

    connect( (QObject*)(m_Controls->m_VisibleOdfsON), SIGNAL(clicked()), this, SLOT(VisibleOdfsON()) );
    connect( (QObject*)(m_Controls->m_ShowMaxNumber), SIGNAL(valueChanged(int)), this, SLOT(ShowMaxNumberChanged(int)) );
    connect( (QObject*)(m_Controls->m_NormalizationDropdown), SIGNAL(currentIndexChanged(int)), this, SLOT(NormalizationDropdownChanged(int)) );
    connect( (QObject*)(m_Controls->m_ScalingFactor), SIGNAL(valueChanged(double)), this, SLOT(ScalingFactorChanged(double)) );
    connect( (QObject*)(m_Controls->m_AdditionalScaling), SIGNAL(currentIndexChanged(int)), this, SLOT(AdditionalScaling(int)) );
    connect( (QObject*)(m_Controls->m_IndexParam1), SIGNAL(valueChanged(double)), this, SLOT(IndexParam1Changed(double)) );
    connect( (QObject*)(m_Controls->m_IndexParam2), SIGNAL(valueChanged(double)), this, SLOT(IndexParam2Changed(double)) );
  }
}

void QmitkControlVisualizationPropertiesView::Activated()
{
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<CvpSelListener>()->DoSelectionChanged(sel);
  QmitkFunctionality::Activated();
}

void QmitkControlVisualizationPropertiesView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

int QmitkControlVisualizationPropertiesView::GetSizeFlags(bool width)
{
  if(!width)
  {
    return berry::Constants::MIN | berry::Constants::MAX | berry::Constants::FILL;
  }
  else
  {
    return 0;
  }
}

int QmitkControlVisualizationPropertiesView::ComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredResult)
{
  if(width==false)
  {
    return m_FoundSingleOdfImage ? 120 : 80;
  }
  else
  {
    return preferredResult;
  }
}

mitk::DataStorage::SetOfObjects::Pointer
QmitkControlVisualizationPropertiesView::ActiveSet(std::string classname)
{
  if (m_CurrentSelection)
  {
    mitk::DataStorage::SetOfObjects::Pointer set =
      mitk::DataStorage::SetOfObjects::New();

    int at = 0;
    for (IStructuredSelection::iterator i = m_CurrentSelection->Begin(); 
      i != m_CurrentSelection->End(); 
      ++i)
    {

      if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
      {
        mitk::DataNode::Pointer node = nodeObj->GetDataNode();
        if(QString(classname.c_str()).compare(node->GetData()->GetNameOfClass())==0)
        {
          set->InsertElement(at++, node);
        }
      }
    }

    return set;
  }

  return 0;
}

void QmitkControlVisualizationPropertiesView::SetBoolProp(
  mitk::DataStorage::SetOfObjects::Pointer set,
  std::string name, bool value)
{
  mitk::DataStorage::SetOfObjects::const_iterator itemiter( set->begin() );  
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( set->end() );  
  while ( itemiter != itemiterend )
  {
    (*itemiter)->SetBoolProperty(name.c_str(), value);
    ++itemiter;
  }
}

void QmitkControlVisualizationPropertiesView::SetIntProp(
  mitk::DataStorage::SetOfObjects::Pointer set,
  std::string name, int value)
{
  mitk::DataStorage::SetOfObjects::const_iterator itemiter( set->begin() );  
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( set->end() );  
  while ( itemiter != itemiterend )
  {
    (*itemiter)->SetIntProperty(name.c_str(), value);
    ++itemiter;
  }
}

void QmitkControlVisualizationPropertiesView::SetFloatProp(
  mitk::DataStorage::SetOfObjects::Pointer set,
  std::string name, float value)
{
  mitk::DataStorage::SetOfObjects::const_iterator itemiter( set->begin() );  
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( set->end() );  
  while ( itemiter != itemiterend )
  {
    (*itemiter)->SetFloatProperty(name.c_str(), value);
    ++itemiter;
  }
}

void QmitkControlVisualizationPropertiesView::SetEnumProp(
  mitk::DataStorage::SetOfObjects::Pointer set,
  std::string name, mitk::EnumerationProperty::Pointer value)
{
  mitk::DataStorage::SetOfObjects::const_iterator itemiter( set->begin() );  
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( set->end() );  
  while ( itemiter != itemiterend )
  {
    (*itemiter)->SetProperty(name.c_str(), value);
    ++itemiter;
  }
}

void QmitkControlVisualizationPropertiesView::DisplayIndexChanged(int dispIndex)
{
  mitk::DataStorage::SetOfObjects::Pointer set =
    ActiveSet("DiffusionImage");

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( set->begin() );  
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( set->end() );  
  while ( itemiter != itemiterend )
  {
    (*itemiter)->SetIntProperty("DisplayChannel", dispIndex);
    ++itemiter;
  }

  //m_MultiWidget->RequestUpdate();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::Reinit()
{
  if (m_CurrentSelection)
  {
    mitk::DataNodeObject::Pointer nodeObj = 
      m_CurrentSelection->Begin()->Cast<mitk::DataNodeObject>();
    mitk::DataNode::Pointer node = nodeObj->GetDataNode();
    mitk::BaseData::Pointer basedata = node->GetData();
    if (basedata.IsNotNull())
    {
      mitk::RenderingManager::GetInstance()->InitializeViews(
        basedata->GetTimeSlicedGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkControlVisualizationPropertiesView::TextIntON()
{
  if(m_TexIsOn)
  {
    m_Controls->m_TextureIntON->setIcon(*m_IconTexOFF);
  }
  else
  {
    m_Controls->m_TextureIntON->setIcon(*m_IconTexON);
  }

  mitk::DataStorage::SetOfObjects::Pointer set =
    ActiveSet("DiffusionImage");
  SetBoolProp(set,"texture interpolation", !m_TexIsOn);

  set = ActiveSet("TensorImage");
  SetBoolProp(set,"texture interpolation", !m_TexIsOn);

  set = ActiveSet("QBallImage");
  SetBoolProp(set,"texture interpolation", !m_TexIsOn);

  set = ActiveSet("Image");
  SetBoolProp(set,"texture interpolation", !m_TexIsOn);

  m_TexIsOn = !m_TexIsOn;

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}


void QmitkControlVisualizationPropertiesView::VisibleOdfsON()
{
  if(m_GlyIsOn)
  {
    m_Controls->m_VisibleOdfsON->setIcon(*m_IconGlyOFF);
  }
  else
  {
    m_Controls->m_VisibleOdfsON->setIcon(*m_IconGlyON);
  }

  mitk::DataStorage::SetOfObjects::Pointer set =
    ActiveSet("QBallImage");
  SetBoolProp(set,"VisibleOdfs", !m_GlyIsOn);

  set = ActiveSet("TensorImage");
  SetBoolProp(set,"VisibleOdfs", !m_GlyIsOn);

  m_GlyIsOn = !m_GlyIsOn;

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkControlVisualizationPropertiesView::ShowMaxNumberChanged(int maxNr)
{
  mitk::DataStorage::SetOfObjects::Pointer set =
    ActiveSet("QBallImage");
  SetIntProp(set,"ShowMaxNumber", maxNr);

  set = ActiveSet("TensorImage");
  SetIntProp(set,"ShowMaxNumber", maxNr);

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkControlVisualizationPropertiesView::NormalizationDropdownChanged(int normDropdown)
{
  typedef mitk::OdfNormalizationMethodProperty PropType;
  PropType::Pointer normMeth = PropType::New();

  switch(normDropdown)
  {
  case 0:
    normMeth->SetNormalizationToMinMax();
    break;
  case 1:
    normMeth->SetNormalizationToMax();
    break;
  case 2:
    normMeth->SetNormalizationToNone();
    break;
  case 3:
    normMeth->SetNormalizationToGlobalMax();
    break;
  default:
    normMeth->SetNormalizationToMinMax();
  }

  mitk::DataStorage::SetOfObjects::Pointer set =
    ActiveSet("QBallImage");
  SetEnumProp(set,"Normalization", normMeth.GetPointer());

  set = ActiveSet("TensorImage");
  SetEnumProp(set,"Normalization", normMeth.GetPointer());

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkControlVisualizationPropertiesView::ScalingFactorChanged(double scalingFactor)
{

  mitk::DataStorage::SetOfObjects::Pointer set =
    ActiveSet("QBallImage");
  SetFloatProp(set,"Scaling", scalingFactor);

  set = ActiveSet("TensorImage");
  SetFloatProp(set,"Scaling", scalingFactor);

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkControlVisualizationPropertiesView::AdditionalScaling(int additionalScaling)
{

  typedef mitk::OdfScaleByProperty PropType;
  PropType::Pointer scaleBy = PropType::New();

  switch(additionalScaling)
  {
  case 0:
    scaleBy->SetScaleByNothing();
    break;
  case 1:
    scaleBy->SetScaleByGFA();
    //m_Controls->params_frame->setVisible(true);
    break;
  case 2:
    scaleBy->SetScaleByPrincipalCurvature();
    // commented in for SPIE paper, Principle curvature scaling
    //m_Controls->params_frame->setVisible(true);
    break;
  default:
    scaleBy->SetScaleByNothing();
  }

  mitk::DataStorage::SetOfObjects::Pointer set =
    ActiveSet("QBallImage");
  SetEnumProp(set,"ScaleBy", scaleBy.GetPointer());

  set = ActiveSet("TensorImage");
  SetEnumProp(set,"ScaleBy", scaleBy.GetPointer());

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkControlVisualizationPropertiesView::IndexParam1Changed(double param1)
{
  mitk::DataStorage::SetOfObjects::Pointer set =
    ActiveSet("QBallImage");
  SetFloatProp(set,"IndexParam1", param1);

  set = ActiveSet("TensorImage");
  SetFloatProp(set,"IndexParam1", param1);

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();
}

void QmitkControlVisualizationPropertiesView::IndexParam2Changed(double param2)
{
  mitk::DataStorage::SetOfObjects::Pointer set =
    ActiveSet("QBallImage");
  SetFloatProp(set,"IndexParam2", param2);

  set = ActiveSet("TensorImage");
  SetFloatProp(set,"IndexParam2", param2);

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();
}

