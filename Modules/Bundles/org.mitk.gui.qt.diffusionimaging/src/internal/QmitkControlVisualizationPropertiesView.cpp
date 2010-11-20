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

#include "mitkDiffusionImage.h"

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
    node->GetBoolProperty("VisibleOdfs_T", do_vis);
    if(do_vis)
    {
      m_View->m_Controls->m_VisibleOdfsON_T->setIcon(*m_View->m_IconGlyON_T);
      m_View->m_Controls->m_VisibleOdfsON_T->setChecked(true);
      m_View->m_GlyIsOn_T = true;
    }
    else
    {
      m_View->m_Controls->m_VisibleOdfsON_T->setIcon(*m_View->m_IconGlyOFF_T);
      m_View->m_Controls->m_VisibleOdfsON_T->setChecked(false);
      m_View->m_GlyIsOn_T = false;
    }

    node->GetBoolProperty("VisibleOdfs_C", do_vis);
    if(do_vis)
    {
      m_View->m_Controls->m_VisibleOdfsON_C->setIcon(*m_View->m_IconGlyON_C);
      m_View->m_Controls->m_VisibleOdfsON_C->setChecked(true);
      m_View->m_GlyIsOn_C = true;
    }
    else
    {
      m_View->m_Controls->m_VisibleOdfsON_C->setIcon(*m_View->m_IconGlyOFF_C);
      m_View->m_Controls->m_VisibleOdfsON_C->setChecked(false);
      m_View->m_GlyIsOn_C = false;
    }

    node->GetBoolProperty("VisibleOdfs_S", do_vis);
    if(do_vis)
    {
      m_View->m_Controls->m_VisibleOdfsON_S->setIcon(*m_View->m_IconGlyON_S);
      m_View->m_Controls->m_VisibleOdfsON_S->setChecked(true);
      m_View->m_GlyIsOn_S = true;
    }
    else
    {
      m_View->m_Controls->m_VisibleOdfsON_S->setIcon(*m_View->m_IconGlyOFF_S);
      m_View->m_Controls->m_VisibleOdfsON_S->setChecked(false);
      m_View->m_GlyIsOn_S = false;
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

    m_View->m_Controls->m_VisibleOdfsON_T->setVisible(false);
    m_View->m_Controls->m_VisibleOdfsON_S->setVisible(false);
    m_View->m_Controls->m_VisibleOdfsON_C->setVisible(false);
    m_View->m_Controls->m_TextureIntON->setVisible(false);

    bool foundDiffusionImage = false;
    bool foundQBIVolume = false;
    bool foundTensorVolume = false;
    bool foundImage = false;
    bool foundMultipleOdfImages = false;

    // do something with the selected items
    if(m_View->m_CurrentSelection)
    {
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

            int maxVal = (dynamic_cast<mitk::DiffusionImage<short>* >(node->GetData()))->GetVectorImage()->GetVectorLength();
            m_View->m_Controls->m_DisplayIndex->setMaximum(maxVal);
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
    }

    m_View->m_Controls->m_DisplayIndex->setVisible(foundDiffusionImage);
    m_View->m_Controls->label_channel->setVisible(foundDiffusionImage);

    m_View->m_FoundSingleOdfImage = (foundQBIVolume || foundTensorVolume)
                                    && !foundMultipleOdfImages;
    m_View->m_Controls->m_NumberGlyphsFrame->setVisible(m_View->m_FoundSingleOdfImage);

    m_View->m_Controls->m_NormalizationDropdown->setVisible(m_View->m_FoundSingleOdfImage);
    m_View->m_Controls->label->setVisible(m_View->m_FoundSingleOdfImage);
    m_View->m_Controls->m_ScalingFactor->setVisible(m_View->m_FoundSingleOdfImage);
    m_View->m_Controls->m_AdditionalScaling->setVisible(m_View->m_FoundSingleOdfImage);
    m_View->m_Controls->m_NormalizationScalingFrame->setVisible(m_View->m_FoundSingleOdfImage);

    m_View->m_Controls->OpacMinFrame->setVisible(m_View->m_FoundSingleOdfImage);

    // changed for SPIE paper, Principle curvature scaling
    //m_View->m_Controls->params_frame->setVisible(m_View->m_FoundSingleOdfImage);
    m_View->m_Controls->params_frame->setVisible(false);

    m_View->m_Controls->m_VisibleOdfsON_T->setVisible(m_View->m_FoundSingleOdfImage);
    m_View->m_Controls->m_VisibleOdfsON_S->setVisible(m_View->m_FoundSingleOdfImage);
    m_View->m_Controls->m_VisibleOdfsON_C->setVisible(m_View->m_FoundSingleOdfImage);

    bool foundAnyImage = foundDiffusionImage ||
                         foundQBIVolume || foundTensorVolume || foundImage;

    m_View->m_Controls->m_Reinit->setVisible(foundAnyImage);
    m_View->m_Controls->m_TextureIntON->setVisible(foundAnyImage);

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
  m_IconGlyOFF_T(new QIcon(":/QmitkDiffusionImaging/glyphsoff_T.png")),
  m_IconGlyON_T(new QIcon(":/QmitkDiffusionImaging/glyphson_T.png")),
  m_IconGlyOFF_C(new QIcon(":/QmitkDiffusionImaging/glyphsoff_C.png")),
  m_IconGlyON_C(new QIcon(":/QmitkDiffusionImaging/glyphson_C.png")),
  m_IconGlyOFF_S(new QIcon(":/QmitkDiffusionImaging/glyphsoff_S.png")),
  m_IconGlyON_S(new QIcon(":/QmitkDiffusionImaging/glyphson_S.png"))
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
    m_Controls->m_VisibleOdfsON_T->setCheckable(true);
    m_Controls->m_VisibleOdfsON_S->setCheckable(true);
    m_Controls->m_VisibleOdfsON_C->setCheckable(true);

#ifndef DIFFUSION_IMAGING_EXTENDED
    int size = m_Controls->m_AdditionalScaling->count();
    for(int t=0; t<size; t++)
    {
      if(m_Controls->m_AdditionalScaling->itemText(t).toStdString() == "Scale by ASR")
      {
        m_Controls->m_AdditionalScaling->removeItem(t);
      }
    }
#endif

    m_Controls->m_OpacitySlider->setRange(0.0,1.0);
    m_Controls->m_OpacitySlider->setLowerValue(0.0);
    m_Controls->m_OpacitySlider->setUpperValue(0.0);

    m_Controls->m_ScalingFrame->setVisible(false);
    m_Controls->m_NormalizationFrame->setVisible(false);

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

    connect( (QObject*)(m_Controls->m_VisibleOdfsON_T), SIGNAL(clicked()), this, SLOT(VisibleOdfsON_T()) );
    connect( (QObject*)(m_Controls->m_VisibleOdfsON_S), SIGNAL(clicked()), this, SLOT(VisibleOdfsON_S()) );
    connect( (QObject*)(m_Controls->m_VisibleOdfsON_C), SIGNAL(clicked()), this, SLOT(VisibleOdfsON_C()) );

    connect( (QObject*)(m_Controls->m_ShowMaxNumber), SIGNAL(editingFinished()), this, SLOT(ShowMaxNumberChanged()) );
    connect( (QObject*)(m_Controls->m_NormalizationDropdown), SIGNAL(currentIndexChanged(int)), this, SLOT(NormalizationDropdownChanged(int)) );
    connect( (QObject*)(m_Controls->m_ScalingFactor), SIGNAL(valueChanged(double)), this, SLOT(ScalingFactorChanged(double)) );
    connect( (QObject*)(m_Controls->m_AdditionalScaling), SIGNAL(currentIndexChanged(int)), this, SLOT(AdditionalScaling(int)) );
    connect( (QObject*)(m_Controls->m_IndexParam1), SIGNAL(valueChanged(double)), this, SLOT(IndexParam1Changed(double)) );
    connect( (QObject*)(m_Controls->m_IndexParam2), SIGNAL(valueChanged(double)), this, SLOT(IndexParam2Changed(double)) );

    connect( (QObject*)(m_Controls->m_NormalizationCheckbox), SIGNAL(clicked()), this, SLOT(NormalizationCheckbox()) );
    connect( (QObject*)(m_Controls->m_ScalingCheckbox), SIGNAL(clicked()), this, SLOT(ScalingCheckbox()) );

    connect( (QObject*)(m_Controls->m_OpacitySlider), SIGNAL(spanChanged(double,double)), this, SLOT(OpacityChanged(double,double)) );

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

int QmitkControlVisualizationPropertiesView::ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult)
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
  if(set.IsNotNull())
  {

    mitk::DataStorage::SetOfObjects::const_iterator itemiter( set->begin() );
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( set->end() );
    while ( itemiter != itemiterend )
    {
      (*itemiter)->SetBoolProperty(name.c_str(), value);
      ++itemiter;
    }
  }
}

void QmitkControlVisualizationPropertiesView::SetIntProp(
    mitk::DataStorage::SetOfObjects::Pointer set,
    std::string name, int value)
{
  if(set.IsNotNull())
  {

    mitk::DataStorage::SetOfObjects::const_iterator itemiter( set->begin() );
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( set->end() );
    while ( itemiter != itemiterend )
    {
      (*itemiter)->SetIntProperty(name.c_str(), value);
      ++itemiter;
    }
  }
}

void QmitkControlVisualizationPropertiesView::SetFloatProp(
    mitk::DataStorage::SetOfObjects::Pointer set,
    std::string name, float value)
{
  if(set.IsNotNull())
  {

    mitk::DataStorage::SetOfObjects::const_iterator itemiter( set->begin() );
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( set->end() );
    while ( itemiter != itemiterend )
    {
      (*itemiter)->SetFloatProperty(name.c_str(), value);
      ++itemiter;
    }
  }
}

void QmitkControlVisualizationPropertiesView::SetLevelWindowProp(
    mitk::DataStorage::SetOfObjects::Pointer set,
    std::string name, mitk::LevelWindow value)
{
  if(set.IsNotNull())
  {

    mitk::LevelWindowProperty::Pointer prop = mitk::LevelWindowProperty::New(value);

    mitk::DataStorage::SetOfObjects::const_iterator itemiter( set->begin() );
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( set->end() );
    while ( itemiter != itemiterend )
    {
      (*itemiter)->SetProperty(name.c_str(), prop);
      ++itemiter;
    }
  }
}

void QmitkControlVisualizationPropertiesView::SetEnumProp(
    mitk::DataStorage::SetOfObjects::Pointer set,
    std::string name, mitk::EnumerationProperty::Pointer value)
{
  if(set.IsNotNull())
  {
    mitk::DataStorage::SetOfObjects::const_iterator itemiter( set->begin() );
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( set->end() );
    while ( itemiter != itemiterend )
    {
      (*itemiter)->SetProperty(name.c_str(), value);
      ++itemiter;
    }
  }
}

void QmitkControlVisualizationPropertiesView::DisplayIndexChanged(int dispIndex)
{
  mitk::DataStorage::SetOfObjects::Pointer set =
      ActiveSet("DiffusionImage");

  if(set.IsNotNull())
  {

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

void QmitkControlVisualizationPropertiesView::VisibleOdfsON_S()
{
  if(m_GlyIsOn_S)
  {
    m_Controls->m_VisibleOdfsON_S->setIcon(*m_IconGlyOFF_S);
  }
  else
  {
    m_Controls->m_VisibleOdfsON_S->setIcon(*m_IconGlyON_S);
  }

  mitk::DataStorage::SetOfObjects::Pointer set =
      ActiveSet("QBallImage");
  SetBoolProp(set,"VisibleOdfs_S", !m_GlyIsOn_S);

  set = ActiveSet("TensorImage");
  SetBoolProp(set,"VisibleOdfs_S", !m_GlyIsOn_S);

  m_GlyIsOn_S = !m_GlyIsOn_S;

   VisibleOdfsON(0);
}

void QmitkControlVisualizationPropertiesView::VisibleOdfsON_T()
{
  if(m_GlyIsOn_T)
  {
    m_Controls->m_VisibleOdfsON_T->setIcon(*m_IconGlyOFF_T);
  }
  else
  {
    m_Controls->m_VisibleOdfsON_T->setIcon(*m_IconGlyON_T);
  }

  mitk::DataStorage::SetOfObjects::Pointer set =
      ActiveSet("QBallImage");
  SetBoolProp(set,"VisibleOdfs_T", !m_GlyIsOn_T);

  set = ActiveSet("TensorImage");
  SetBoolProp(set,"VisibleOdfs_T", !m_GlyIsOn_T);

  m_GlyIsOn_T = !m_GlyIsOn_T;

  VisibleOdfsON(1);

}

void QmitkControlVisualizationPropertiesView::VisibleOdfsON_C()
{
  if(m_GlyIsOn_C)
  {
    m_Controls->m_VisibleOdfsON_C->setIcon(*m_IconGlyOFF_C);
  }
  else
  {
    m_Controls->m_VisibleOdfsON_C->setIcon(*m_IconGlyON_C);
  }

  mitk::DataStorage::SetOfObjects::Pointer set =
      ActiveSet("QBallImage");
  SetBoolProp(set,"VisibleOdfs_C", !m_GlyIsOn_C);

  set = ActiveSet("TensorImage");
  SetBoolProp(set,"VisibleOdfs_C", !m_GlyIsOn_C);

  m_GlyIsOn_C = !m_GlyIsOn_C;

  VisibleOdfsON(2);

}

void QmitkControlVisualizationPropertiesView::VisibleOdfsON(int view)
{

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkControlVisualizationPropertiesView::ShowMaxNumberChanged()
{
  int maxNr = m_Controls->m_ShowMaxNumber->value();
  if ( maxNr < 1 )
  {
    m_Controls->m_ShowMaxNumber->setValue( 1 );
    maxNr = 1;
  }

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
#ifdef DIFFUSION_IMAGING_EXTENDED
  case 2:
    scaleBy->SetScaleByPrincipalCurvature();
    // commented in for SPIE paper, Principle curvature scaling
    //m_Controls->params_frame->setVisible(true);
    break;
#endif
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

void QmitkControlVisualizationPropertiesView::OpacityChanged(double l, double u)
{
  mitk::LevelWindow olw;
  olw.SetRangeMinMax(l*255, u*255);

  mitk::DataStorage::SetOfObjects::Pointer set =
      ActiveSet("QBallImage");
  SetLevelWindowProp(set,"opaclevelwindow", olw);

  set = ActiveSet("TensorImage");
  SetLevelWindowProp(set,"opaclevelwindow", olw);

  m_Controls->m_OpacityMinFaLabel->setText(QString::number(l,'f',2) + " : " + QString::number(u,'f',2));

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkControlVisualizationPropertiesView::NormalizationCheckbox()
{
  m_Controls->m_NormalizationFrame->setVisible(
      m_Controls->m_NormalizationCheckbox->isChecked());
}

void QmitkControlVisualizationPropertiesView::ScalingCheckbox()
{
  m_Controls->m_ScalingFrame->setVisible(
      m_Controls->m_ScalingCheckbox->isChecked());
}
