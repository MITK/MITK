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

#include "QmitkNavigationButtonsView.h"

#include "mitkNodePredicateDataType.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include "mitkResliceMethodProperty.h"
#include "mitkRenderingManager.h"

#include "mitkDiffusionImage.h"
#include "mitkPlanarFigure.h"
#include "mitkFiberBundle.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
#include "mitkFiberBundleInteractor.h"
#include "mitkPlanarFigureInteractor.h"

#include "mitkGlobalInteraction.h"

#include "mitkGeometry2D.h"

#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPage.h"
#include "berryISelectionService.h"
#include "berryConstants.h"
#include "berryPlatformUI.h"

#include "itkRGBAPixel.h"
#include "itkTractsToProbabilityImageFilter.h"

#include "qwidgetaction.h"
#include "qcolordialog.h"

const std::string QmitkNavigationButtonsView::VIEW_ID = "org.mitk.views.NavigationButtonsview";

using namespace berry;


QmitkNavigationButtonsView::QmitkNavigationButtonsView()
  : QmitkFunctionality(),
  m_Controls(NULL),
  m_MultiWidget(NULL),
{
}

QmitkNavigationButtonsView::QmitkNavigationButtonsView(const QmitkNavigationButtonsView& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QmitkNavigationButtonsView::~QmitkNavigationButtonsView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
}



void QmitkNavigationButtonsView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkNavigationButtonsViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

  }
}


void QmitkNavigationButtonsView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkNavigationButtonsView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkNavigationButtonsView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_TextureIntON), SIGNAL(clicked()), this, SLOT(TextIntON()) );
  }
}

void QmitkNavigationButtonsView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkNavigationButtonsView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

int QmitkNavigationButtonsView::GetSizeFlags(bool width)
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

int QmitkNavigationButtonsView::ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult)
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
    QmitkNavigationButtonsView::ActiveSet(std::string classname)
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

void QmitkNavigationButtonsView::SetBoolProp(
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

void QmitkNavigationButtonsView::SetIntProp(
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

void QmitkNavigationButtonsView::SetFloatProp(
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

void QmitkNavigationButtonsView::SetLevelWindowProp(
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

void QmitkNavigationButtonsView::SetEnumProp(
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

void QmitkNavigationButtonsView::DisplayIndexChanged(int dispIndex)
{

  QString label = "Channel %1";
  label = label.arg(dispIndex);
  m_Controls->label_channel->setText(label);

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

void QmitkNavigationButtonsView::Reinit()
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

void QmitkNavigationButtonsView::TextIntON()
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

void QmitkNavigationButtonsView::VisibleOdfsON_S()
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

void QmitkNavigationButtonsView::VisibleOdfsON_T()
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

void QmitkNavigationButtonsView::VisibleOdfsON_C()
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

void QmitkNavigationButtonsView::VisibleOdfsON(int view)
{

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkNavigationButtonsView::ShowMaxNumberChanged()
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

void QmitkNavigationButtonsView::NormalizationDropdownChanged(int normDropdown)
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

void QmitkNavigationButtonsView::ScalingFactorChanged(double scalingFactor)
{

  mitk::DataStorage::SetOfObjects::Pointer set =
      ActiveSet("QBallImage");
  SetFloatProp(set,"Scaling", scalingFactor);

  set = ActiveSet("TensorImage");
  SetFloatProp(set,"Scaling", scalingFactor);

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkNavigationButtonsView::AdditionalScaling(int additionalScaling)
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

void QmitkNavigationButtonsView::IndexParam1Changed(double param1)
{
  mitk::DataStorage::SetOfObjects::Pointer set =
      ActiveSet("QBallImage");
  SetFloatProp(set,"IndexParam1", param1);

  set = ActiveSet("TensorImage");
  SetFloatProp(set,"IndexParam1", param1);

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();
}

void QmitkNavigationButtonsView::IndexParam2Changed(double param2)
{
  mitk::DataStorage::SetOfObjects::Pointer set =
      ActiveSet("QBallImage");
  SetFloatProp(set,"IndexParam2", param2);

  set = ActiveSet("TensorImage");
  SetFloatProp(set,"IndexParam2", param2);

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();
}

void QmitkNavigationButtonsView::OpacityChanged(double l, double u)
{
  mitk::LevelWindow olw;
  olw.SetRangeMinMax(l*255, u*255);

  mitk::DataStorage::SetOfObjects::Pointer set =
      ActiveSet("QBallImage");
  SetLevelWindowProp(set,"opaclevelwindow", olw);

  set = ActiveSet("TensorImage");
  SetLevelWindowProp(set,"opaclevelwindow", olw);

  set = ActiveSet("Image");
  SetLevelWindowProp(set,"opaclevelwindow", olw);

  m_Controls->m_OpacityMinFaLabel->setText(QString::number(l,'f',2) + " : " + QString::number(u,'f',2));

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkNavigationButtonsView::ScalingCheckbox()
{
  m_Controls->m_ScalingFrame->setVisible(
      m_Controls->m_ScalingCheckbox->isChecked());
}


void QmitkNavigationButtonsView::BundleRepresentationWire()
{
  if(m_SelectedNode)
  {
    int width = m_Controls->m_LineWidth->value();
    m_SelectedNode->SetProperty("LineWidth",mitk::IntProperty::New(width));
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(15));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(18));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(1));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(2));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(3));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(4));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(0));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void QmitkNavigationButtonsView::BundleRepresentationTube()
{
  if(m_SelectedNode)
  {
    float radius = m_Controls->m_TubeRadius->value() / 100.0;
    m_SelectedNode->SetProperty("TubeRadius",mitk::FloatProperty::New(radius));
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(17));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(13));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(16));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(0));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void QmitkNavigationButtonsView::BundleRepresentationColor()
{
  if(m_SelectedNode)
  {
    QColor color = QColorDialog::getColor();

    m_Controls->m_Color->setAutoFillBackground(true);
    QString styleSheet = "background-color:rgb(";
    styleSheet.append(QString::number(color.red()));
    styleSheet.append(",");
    styleSheet.append(QString::number(color.green()));
    styleSheet.append(",");
    styleSheet.append(QString::number(color.blue()));
    styleSheet.append(")");
    m_Controls->m_Color->setStyleSheet(styleSheet);

    m_SelectedNode->SetProperty("color",mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0));
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(14));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(3));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(0));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void QmitkNavigationButtonsView::PlanarFigureFocus()
{
  if(m_SelectedNode)
  {
    mitk::PlanarFigure* _PlanarFigure = 0;
    _PlanarFigure = dynamic_cast<mitk::PlanarFigure*> (m_SelectedNode->GetData());

    if (_PlanarFigure)
    {

      QmitkRenderWindow* selectedRenderWindow = 0;
      bool PlanarFigureInitializedWindow = false;

      QmitkRenderWindow* RenderWindow1 =
          this->GetActiveStdMultiWidget()->GetRenderWindow1();

      if (m_SelectedNode->GetBoolProperty("PlanarFigureInitializedWindow",
        PlanarFigureInitializedWindow, RenderWindow1->GetRenderer()))
      {
        selectedRenderWindow = RenderWindow1;
      }

      QmitkRenderWindow* RenderWindow2 =
          this->GetActiveStdMultiWidget()->GetRenderWindow2();

      if (!selectedRenderWindow && m_SelectedNode->GetBoolProperty(
          "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
          RenderWindow2->GetRenderer()))
      {
        selectedRenderWindow = RenderWindow2;
      }

      QmitkRenderWindow* RenderWindow3 =
          this->GetActiveStdMultiWidget()->GetRenderWindow3();

      if (!selectedRenderWindow && m_SelectedNode->GetBoolProperty(
          "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
          RenderWindow3->GetRenderer()))
      {
        selectedRenderWindow = RenderWindow3;
      }

      QmitkRenderWindow* RenderWindow4 =
          this->GetActiveStdMultiWidget()->GetRenderWindow4();

      if (!selectedRenderWindow && m_SelectedNode->GetBoolProperty(
          "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
          RenderWindow4->GetRenderer()))
      {
        selectedRenderWindow = RenderWindow4;
      }

      const mitk::PlaneGeometry
          * _PlaneGeometry =
          dynamic_cast<const mitk::PlaneGeometry*> (_PlanarFigure->GetGeometry2D());
      mitk::VnlVector normal = _PlaneGeometry->GetNormalVnl();

      mitk::Geometry2D::ConstPointer worldGeometry1 =
        RenderWindow1->GetRenderer()->GetCurrentWorldGeometry2D();
      mitk::PlaneGeometry::ConstPointer _Plane1 =
        dynamic_cast<const mitk::PlaneGeometry*>( worldGeometry1.GetPointer() );
      mitk::VnlVector normal1 = _Plane1->GetNormalVnl();

      mitk::Geometry2D::ConstPointer worldGeometry2 =
        RenderWindow2->GetRenderer()->GetCurrentWorldGeometry2D();
      mitk::PlaneGeometry::ConstPointer _Plane2 =
        dynamic_cast<const mitk::PlaneGeometry*>( worldGeometry2.GetPointer() );
      mitk::VnlVector normal2 = _Plane2->GetNormalVnl();

      mitk::Geometry2D::ConstPointer worldGeometry3 =
        RenderWindow3->GetRenderer()->GetCurrentWorldGeometry2D();
      mitk::PlaneGeometry::ConstPointer _Plane3 =
        dynamic_cast<const mitk::PlaneGeometry*>( worldGeometry3.GetPointer() );
      mitk::VnlVector normal3 = _Plane3->GetNormalVnl();

      normal[0]  = fabs(normal[0]);  normal[1]  = fabs(normal[1]);  normal[2]  = fabs(normal[2]);
      normal1[0] = fabs(normal1[0]); normal1[1] = fabs(normal1[1]); normal1[2] = fabs(normal1[2]);
      normal2[0] = fabs(normal2[0]); normal2[1] = fabs(normal2[1]); normal2[2] = fabs(normal2[2]);
      normal3[0] = fabs(normal3[0]); normal3[1] = fabs(normal3[1]); normal3[2] = fabs(normal3[2]);

      double ang1 = angle(normal, normal1);
      double ang2 = angle(normal, normal2);
      double ang3 = angle(normal, normal3);

      if(ang1 < ang2 && ang1 < ang3)
      {
        selectedRenderWindow = RenderWindow1;
      }
      else
      {
        if(ang2 < ang3)
        {
          selectedRenderWindow = RenderWindow2;
        }
        else
        {
          selectedRenderWindow = RenderWindow3;
        }
      }

      // make node visible
      if (selectedRenderWindow)
      {
        mitk::Point3D centerP = _PlaneGeometry->GetOrigin();
        selectedRenderWindow->GetSliceNavigationController()->ReorientSlices(
            centerP, _PlaneGeometry->GetNormal());
        selectedRenderWindow->GetSliceNavigationController()->SelectSliceByPoint(
            centerP);
      }
    }

    // set interactor for new node (if not already set)
    mitk::PlanarFigureInteractor::Pointer figureInteractor
        = dynamic_cast<mitk::PlanarFigureInteractor*>(m_SelectedNode->GetInteractor());

    if(figureInteractor.IsNull())
      figureInteractor = mitk::PlanarFigureInteractor::New("PlanarFigureInteractor", m_SelectedNode);

    mitk::GlobalInteraction::GetInstance()->AddInteractor(figureInteractor);

    m_SelectedNode->SetProperty("planarfigure.iseditable",mitk::BoolProperty::New(true));

  }
}

void QmitkNavigationButtonsView::SetInteractor()
{
  typedef std::vector<mitk::DataNode*> Container;
  Container _NodeSet = this->GetDataManagerSelection();
  mitk::DataNode* node = 0;
  mitk::FiberBundle* bundle = 0;
  mitk::FiberBundleInteractor::Pointer bundleInteractor = 0;

  // finally add all nodes to the model
  for(Container::const_iterator it=_NodeSet.begin(); it!=_NodeSet.end()
    ; it++)
    {
    node = const_cast<mitk::DataNode*>(*it);
    bundle = dynamic_cast<mitk::FiberBundle*>(node->GetData());

    if(bundle)
    {
      bundleInteractor = dynamic_cast<mitk::FiberBundleInteractor*>(node->GetInteractor());

      if(bundleInteractor.IsNotNull())
        mitk::GlobalInteraction::GetInstance()->RemoveInteractor(bundleInteractor);

      if(!m_Controls->m_Crosshair->isChecked())
      {
        m_Controls->m_Crosshair->setChecked(false);
        this->GetActiveStdMultiWidget()->GetRenderWindow4()->setCursor(Qt::ArrowCursor);
        m_CurrentPickingNode = 0;
      }
      else
      {
        m_Controls->m_Crosshair->setChecked(true);
        bundleInteractor = mitk::FiberBundleInteractor::New("FiberBundleInteractor", node);
        mitk::GlobalInteraction::GetInstance()->AddInteractor(bundleInteractor);
        this->GetActiveStdMultiWidget()->GetRenderWindow4()->setCursor(Qt::CrossCursor);
        m_CurrentPickingNode = node;
      }

    }
  }

}

void QmitkNavigationButtonsView::PFWidth(int w)
{

  double width = w/10.0;
  m_SelectedNode->SetProperty("planarfigure.line.width", mitk::FloatProperty::New(width) );
  m_SelectedNode->SetProperty("planarfigure.shadow.widthmodifier", mitk::FloatProperty::New(width) );
  m_SelectedNode->SetProperty("planarfigure.outline.width", mitk::FloatProperty::New(width) );
  m_SelectedNode->SetProperty("planarfigure.helperline.width", mitk::FloatProperty::New(width) );

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  QString label = "Width %1";
  label = label.arg(width);
  m_Controls->label_pfwidth->setText(label);

}

void QmitkNavigationButtonsView::PFColor()
{

  QColor color = QColorDialog::getColor();

  m_Controls->m_PFColor->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";
  styleSheet.append(QString::number(color.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.blue()));
  styleSheet.append(")");
  m_Controls->m_PFColor->setStyleSheet(styleSheet);

  m_SelectedNode->SetProperty( "planarfigure.default.line.color", mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0));
  m_SelectedNode->SetProperty( "planarfigure.default.outline.color", mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0));
  m_SelectedNode->SetProperty( "planarfigure.default.helperline.color", mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0));
  m_SelectedNode->SetProperty( "planarfigure.default.markerline.color", mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0));
  m_SelectedNode->SetProperty( "planarfigure.default.marker.color", mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0));

  m_SelectedNode->SetProperty( "planarfigure.hover.line.color", mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0)  );
  m_SelectedNode->SetProperty( "planarfigure.hover.outline.color", mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0)  );
  m_SelectedNode->SetProperty( "planarfigure.hover.helperline.color", mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0)  );
//  m_SelectedNode->SetProperty( "planarfigure.hover.markerline.color", mitk::ColorProperty::New(0.0,1.0,0.0)  );
//  m_SelectedNode->SetProperty( "planarfigure.hover.marker.color", mitk::ColorProperty::New(0.0,1.0,0.0)  );

//  m_SelectedNode->SetProperty( "planarfigure.selected.line.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
//  m_SelectedNode->SetProperty( "planarfigure.selected.outline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
//  m_SelectedNode->SetProperty( "planarfigure.selected.helperline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
//  m_SelectedNode->SetProperty( "planarfigure.selected.markerline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
//  m_SelectedNode->SetProperty( "planarfigure.selected.marker.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkNavigationButtonsView::PFColor3D()
{

  QColor color = QColorDialog::getColor();

  m_Controls->m_PFColor3D->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";
  styleSheet.append(QString::number(color.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.blue()));
  styleSheet.append(")");
  m_Controls->m_PFColor3D->setStyleSheet(styleSheet);

  m_SelectedNode->SetProperty( "color", mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0));

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkNavigationButtonsView::Heatmap()
{
  if(m_SelectedNode)
  {
    mitk::FiberBundle* bundle = dynamic_cast<mitk::FiberBundle*>(m_SelectedNode->GetData());
    if(!bundle)
      return;

    ///////////////////////////////
    // Generate unsigned char Image
    typedef unsigned char OutPixType2;

    // run generator
    typedef itk::Image< float, 3 >            WMPImageType;
    typedef itk::TractsToProbabilityImageFilter<WMPImageType,
        OutPixType2> ImageGeneratorType2;
    ImageGeneratorType2::Pointer generator = ImageGeneratorType2::New();
    //generator->SetInput(NULL);
    generator->SetFiberBundle(bundle);
    generator->SetInvertImage(false);
    generator->SetUpsamplingFactor(2);
    generator->SetBinaryEnvelope(false);
    generator->Update();

    // get result
    typedef itk::Image<OutPixType2,3> OutType2;
    OutType2::Pointer outImg = generator->GetOutput();

    mitk::Image::Pointer img2 = mitk::Image::New();
    img2->InitializeByItk(outImg.GetPointer());
    img2->SetVolume(outImg->GetBufferPointer());

    // to datastorage
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(img2);
    QString name(m_SelectedNode->GetName().c_str());
    name += "_heatmap";
    node->SetName(name.toStdString());
    node->SetVisibility(true);

    GetDataStorage()->Add(node);
  }
}

void QmitkNavigationButtonsView::LineWidthChanged(int w)
{
  m_SelectedNode->SetIntProperty("LineWidth", w);

  QString label = "Width %1";
  label = label.arg(w);
  m_Controls->label_linewidth->setText(label);
}

void QmitkNavigationButtonsView::TubeRadiusChanged(int r)
{
  m_SelectedNode->SetFloatProperty("TubeRadius", (float) r / 100.0);

  QString label = "Radius %1";
  label = label.arg(r / 100.0);
  m_Controls->label_tuberadius->setText(label);
}

void QmitkNavigationButtonsView::Welcome()
{
  berry::PlatformUI::GetWorkbench()->GetIntroManager()->ShowIntro(
   GetSite()->GetWorkbenchWindow(), false);
}
