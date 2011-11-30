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
#include "mitkResliceMethodProperty.h"
#include "mitkRenderingManager.h"

#include "mitkTbssImage.h"
#include "mitkPlanarFigure.h"
#include "mitkFiberBundle.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
#include "mitkFiberBundleInteractor.h"
#include "mitkPlanarFigureInteractor.h"
#include <mitkQBallImage.h>
#include <mitkTensorImage.h>
#include <mitkDiffusionImage.h>

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

    m_View->m_Controls->m_ImageControlsFrame->setVisible(false);
    m_View->m_Controls->m_PlanarFigureControlsFrame->setVisible(false);
    m_View->m_Controls->m_BundleControlsFrame->setVisible(false);
    m_View->m_SelectedNode = 0;

    if(m_View->m_CurrentSelection.IsNull())
      return;

    if(m_View->m_CurrentSelection->Size() == 1)
    {
      mitk::DataNodeObject::Pointer nodeObj = m_View->m_CurrentSelection->Begin()->Cast<mitk::DataNodeObject>();
      if(nodeObj.IsNotNull())
      {
        mitk::DataNode::Pointer node = nodeObj->GetDataNode();

        if(dynamic_cast<mitk::PlanarFigure*>(node->GetData()) != 0)
        {
          m_View->m_Controls->m_PlanarFigureControlsFrame->setVisible(true);
          m_View->m_SelectedNode = node;

          float val;
          node->GetFloatProperty("planarfigure.line.width", val);
          m_View->m_Controls->m_PFWidth->setValue((int)(val*10.0));

          QString label = "Width %1";
          label = label.arg(val);
          m_View->m_Controls->label_pfwidth->setText(label);

          float color[3];
          node->GetColor( color, NULL, "planarfigure.default.line.color");
          QString styleSheet = "background-color:rgb(";
          styleSheet.append(QString::number(color[0]*255.0));
          styleSheet.append(",");
          styleSheet.append(QString::number(color[1]*255.0));
          styleSheet.append(",");
          styleSheet.append(QString::number(color[2]*255.0));
          styleSheet.append(")");
          m_View->m_Controls->m_PFColor->setAutoFillBackground(true);
          m_View->m_Controls->m_PFColor->setStyleSheet(styleSheet);

          node->GetColor( color, NULL, "color");
          styleSheet = "background-color:rgb(";
          styleSheet.append(QString::number(color[0]*255.0));
          styleSheet.append(",");
          styleSheet.append(QString::number(color[1]*255.0));
          styleSheet.append(",");
          styleSheet.append(QString::number(color[2]*255.0));
          styleSheet.append(")");

          m_View->PlanarFigureFocus();
        }

        if(dynamic_cast<mitk::FiberBundle*>(node->GetData()) != 0)
        {
          m_View->m_Controls->m_BundleControlsFrame->setVisible(true);
          m_View->m_SelectedNode = node;

          if(m_View->m_CurrentPickingNode != 0 && node.GetPointer() != m_View->m_CurrentPickingNode)
          {
            m_View->m_Controls->m_Crosshair->setEnabled(false);
          }
          else
          {
            m_View->m_Controls->m_Crosshair->setEnabled(true);
          }

          float val;
          node->GetFloatProperty("TubeRadius", val);
          m_View->m_Controls->m_TubeRadius->setValue((int)(val * 100.0));

          QString label = "Radius %1";
          label = label.arg(val);
          m_View->m_Controls->label_tuberadius->setText(label);

          int width;
          node->GetIntProperty("LineWidth", width);
          m_View->m_Controls->m_LineWidth->setValue(width);

          label = "Width %1";
          label = label.arg(width);
          m_View->m_Controls->label_linewidth->setText(label);

          float range;
          node->GetFloatProperty("Fiber2DSliceThickness",range);
          label = "Range %1";
          label = label.arg(range*0.1);
          m_View->m_Controls->label_range->setText(label);

//          mitk::ColorProperty* nodecolor= mitk::ColorProperty::New();
//          node->GetProperty<mitk::ColorProperty>(nodecolor,"color");
//          m_View->m_Controls->m_Color->setAutoFillBackground(true);
//          QString styleSheet = "background-color:rgb(";
//          styleSheet.append(QString::number(nodecolor->GetColor().GetRed()*255.0));
//          styleSheet.append(",");
//          styleSheet.append(QString::number(nodecolor->GetColor().GetGreen()*255.0));
//          styleSheet.append(",");
//          styleSheet.append(QString::number(nodecolor->GetColor().GetBlue()*255.0));
//          styleSheet.append(")");
//          m_View->m_Controls->m_Color->setStyleSheet(styleSheet);


        }
      }
    }

    if(m_View->m_CurrentSelection->Size() > 0 && m_View->m_SelectedNode == 0)
    {
      m_View->m_Controls->m_ImageControlsFrame->setVisible(true);

      bool foundDiffusionImage = false;
      bool foundQBIVolume = false;
      bool foundTensorVolume = false;
      bool foundImage = false;
      bool foundMultipleOdfImages = false;
      bool foundRGBAImage = false;
      bool foundTbssImage = false;

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

              QString label = "Channel %1";
              label = label.arg(val);
              m_View->m_Controls->label_channel->setText(label);

              int maxVal = (dynamic_cast<mitk::DiffusionImage<short>* >(node->GetData()))->GetVectorImage()->GetVectorLength();
              m_View->m_Controls->m_DisplayIndex->setMaximum(maxVal-1);
            }

            if(QString("TbssImage").compare(node->GetData()->GetNameOfClass())==0)
            {
              foundTbssImage = true;
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

              QString label = "Channel %1";
              label = label.arg(val);
              m_View->m_Controls->label_channel->setText(label);

              int maxVal = (dynamic_cast<mitk::TbssImage* >(node->GetData()))->GetImage()->GetVectorLength();
              m_View->m_Controls->m_DisplayIndex->setMaximum(maxVal-1);
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
              mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(node->GetData());
              if(img.IsNotNull() && img->GetPixelType().GetItkTypeId() == &typeid(itk::RGBAPixel<unsigned char>) )
              {
                foundRGBAImage = true;
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
            }
          }
        }
      }

      m_View->m_FoundSingleOdfImage = (foundQBIVolume || foundTensorVolume)
                                      && !foundMultipleOdfImages;
      m_View->m_Controls->m_NumberGlyphsFrame->setVisible(m_View->m_FoundSingleOdfImage);

      m_View->m_Controls->m_NormalizationDropdown->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->label->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->m_ScalingFactor->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->m_AdditionalScaling->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->m_NormalizationScalingFrame->setVisible(m_View->m_FoundSingleOdfImage);

      m_View->m_Controls->OpacMinFrame->setVisible(foundRGBAImage || m_View->m_FoundSingleOdfImage);

      // changed for SPIE paper, Principle curvature scaling
      //m_View->m_Controls->params_frame->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->params_frame->setVisible(false);

      m_View->m_Controls->m_VisibleOdfsON_T->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->m_VisibleOdfsON_S->setVisible(m_View->m_FoundSingleOdfImage);
      m_View->m_Controls->m_VisibleOdfsON_C->setVisible(m_View->m_FoundSingleOdfImage);

      bool foundAnyImage = foundDiffusionImage ||
                           foundQBIVolume || foundTensorVolume || foundImage || foundTbssImage;

      m_View->m_Controls->m_Reinit->setVisible(foundAnyImage);
      m_View->m_Controls->m_TextureIntON->setVisible(foundAnyImage);
      m_View->m_Controls->m_TSMenu->setVisible(foundAnyImage);

      if(m_View->m_IsInitialized)
      {
        //m_View->GetSite()->GetWorkbenchWindow()->GetActivePage()
        //  ->HideView(IViewPart::Pointer(m_View));

        //berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()
        //  ->ShowView(QmitkControlVisualizationPropertiesView::VIEW_ID,
        //  "", berry::IWorkbenchPage::VIEW_VISIBLE);

      }
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
  m_NodeUsedForOdfVisualization(NULL),
  m_IconTexOFF(new QIcon(":/QmitkDiffusionImaging/texIntOFFIcon.png")),
  m_IconTexON(new QIcon(":/QmitkDiffusionImaging/texIntONIcon.png")),
  m_IconGlyOFF_T(new QIcon(":/QmitkDiffusionImaging/glyphsoff_T.png")),
  m_IconGlyON_T(new QIcon(":/QmitkDiffusionImaging/glyphson_T.png")),
  m_IconGlyOFF_C(new QIcon(":/QmitkDiffusionImaging/glyphsoff_C.png")),
  m_IconGlyON_C(new QIcon(":/QmitkDiffusionImaging/glyphson_C.png")),
  m_IconGlyOFF_S(new QIcon(":/QmitkDiffusionImaging/glyphsoff_S.png")),
  m_IconGlyON_S(new QIcon(":/QmitkDiffusionImaging/glyphson_S.png")),
  m_CurrentSelection(0),
  m_CurrentPickingNode(0),
  m_GlyIsOn_S(false),
  m_GlyIsOn_C(false),
  m_GlyIsOn_T(false)
{
  currentThickSlicesMode = 1;
  m_MyMenu = NULL;
}

QmitkControlVisualizationPropertiesView::QmitkControlVisualizationPropertiesView(const QmitkControlVisualizationPropertiesView& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QmitkControlVisualizationPropertiesView::~QmitkControlVisualizationPropertiesView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
}

void QmitkControlVisualizationPropertiesView::OnThickSlicesModeSelected( QAction* action )
{
  currentThickSlicesMode = action->data().toInt();

  switch(currentThickSlicesMode)
  {
  default:
  case 1:
    this->m_Controls->m_TSMenu->setText("MIP");
    break;
  case 2:
    this->m_Controls->m_TSMenu->setText("SUM");
    break;
  case 3:
    this->m_Controls->m_TSMenu->setText("WEIGH");
    break;
  }

  mitk::DataNode* n;
  n = GetDataStorage()->GetNamedNode("widget1Plane"); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
  n = GetDataStorage()->GetNamedNode("widget2Plane"); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
  n = GetDataStorage()->GetNamedNode("widget3Plane"); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );

  mitk::BaseRenderer::Pointer renderer =
      this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderer();
  if(renderer.IsNotNull())
  {
    renderer->SendUpdateSlice();
  }
  renderer = this->GetActiveStdMultiWidget()->GetRenderWindow2()->GetRenderer();
  if(renderer.IsNotNull())
  {
    renderer->SendUpdateSlice();
  }
  renderer = this->GetActiveStdMultiWidget()->GetRenderWindow3()->GetRenderer();
  if(renderer.IsNotNull())
  {
    renderer->SendUpdateSlice();
  }
  renderer->GetRenderingManager()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::OnTSNumChanged(int num)
{
  if(num==0)
  {
    mitk::DataNode* n;
    n = GetDataStorage()->GetNamedNode("widget1Plane"); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( 0 ) );
    n = GetDataStorage()->GetNamedNode("widget2Plane"); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( 0 ) );
    n = GetDataStorage()->GetNamedNode("widget3Plane"); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( 0 ) );
  }
  else
  {
    mitk::DataNode* n;
    n = GetDataStorage()->GetNamedNode("widget1Plane"); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
    n = GetDataStorage()->GetNamedNode("widget2Plane"); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
    n = GetDataStorage()->GetNamedNode("widget3Plane"); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );

    n = GetDataStorage()->GetNamedNode("widget1Plane"); if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
    n = GetDataStorage()->GetNamedNode("widget2Plane"); if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
    n = GetDataStorage()->GetNamedNode("widget3Plane"); if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
  }

  m_TSLabel->setText(QString::number(num*2+1));

  mitk::BaseRenderer::Pointer renderer =
      this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderer();
  if(renderer.IsNotNull())
  {
    renderer->SendUpdateSlice();
  }
  renderer = this->GetActiveStdMultiWidget()->GetRenderWindow2()->GetRenderer();
  if(renderer.IsNotNull())
  {
    renderer->SendUpdateSlice();
  }
  renderer = this->GetActiveStdMultiWidget()->GetRenderWindow3()->GetRenderer();
  if(renderer.IsNotNull())
  {
    renderer->SendUpdateSlice();
  }
  renderer->GetRenderingManager()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);
}

void QmitkControlVisualizationPropertiesView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkControlVisualizationPropertiesViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_MyMenu = new QMenu(parent);
    connect( m_MyMenu, SIGNAL( aboutToShow() ), this, SLOT(OnMenuAboutToShow()) );

    // button for changing rotation mode
    m_Controls->m_TSMenu->setMenu( m_MyMenu );
    //m_CrosshairModeButton->setIcon( QIcon( iconCrosshairMode_xpm ) );

    m_Controls->params_frame->setVisible(false);

    QIcon icon5(":/QmitkDiffusionImaging/Refresh_48.png");
    m_Controls->m_Reinit->setIcon(icon5);
    m_Controls->m_Focus->setIcon(icon5);

    QIcon iconColor(":/QmitkDiffusionImaging/color24.gif");
    m_Controls->m_PFColor->setIcon(iconColor);
    m_Controls->m_Color->setIcon(iconColor);

    QIcon iconReset(":/QmitkDiffusionImaging/reset.png");
    m_Controls->m_ResetColoring->setIcon(iconReset);

    m_Controls->m_PFColor->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QIcon iconCrosshair(":/QmitkDiffusionImaging/crosshair.png");
    m_Controls->m_Crosshair->setIcon(iconCrosshair);
// was is los
    QIcon iconPaint(":/QmitkDiffusionImaging/paint2.png");
    m_Controls->m_TDI->setIcon(iconPaint);

    QIcon iconFiberFade(":/QmitkDiffusionImaging/MapperEfx2D.png");
    m_Controls->m_FiberFading2D->setIcon(iconFiberFade);

    m_Controls->m_TextureIntON->setCheckable(true);

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

void QmitkControlVisualizationPropertiesView::OnMenuAboutToShow ()
{
  // THICK SLICE SUPPORT
  QMenu *myMenu = m_MyMenu;
  myMenu->clear();

  QActionGroup* thickSlicesActionGroup = new QActionGroup(myMenu);
  thickSlicesActionGroup->setExclusive(true);

  mitk::BaseRenderer::Pointer renderer =
      this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderer();

  int currentTSMode = 0;
  {
    mitk::ResliceMethodProperty::Pointer m = dynamic_cast<mitk::ResliceMethodProperty*>(renderer->GetCurrentWorldGeometry2DNode()->GetProperty( "reslice.thickslices" ));
    if( m.IsNotNull() )
      currentTSMode = m->GetValueAsId();
  }

  const int maxTS = 30;

  int currentNum = 0;
  {
    mitk::IntProperty::Pointer m = dynamic_cast<mitk::IntProperty*>(renderer->GetCurrentWorldGeometry2DNode()->GetProperty( "reslice.thickslices.num" ));
    if( m.IsNotNull() )
    {
      currentNum = m->GetValue();
      if(currentNum < 0) currentNum = 0;
      if(currentNum > maxTS) currentNum = maxTS;
    }
  }

  if(currentTSMode==0)
    currentNum=0;

  QSlider *m_TSSlider = new QSlider(myMenu);
  m_TSSlider->setMinimum(0);
  m_TSSlider->setMaximum(maxTS-1);
  m_TSSlider->setValue(currentNum);

  m_TSSlider->setOrientation(Qt::Horizontal);

  connect( m_TSSlider, SIGNAL( valueChanged(int) ), this, SLOT( OnTSNumChanged(int) ) );

  QHBoxLayout* _TSLayout = new QHBoxLayout;
  _TSLayout->setContentsMargins(4,4,4,4);
  _TSLayout->addWidget(m_TSSlider);
  _TSLayout->addWidget(m_TSLabel=new QLabel(QString::number(currentNum*2+1),myMenu));

  QWidget* _TSWidget = new QWidget;
  _TSWidget->setLayout(_TSLayout);

  QActionGroup* thickSliceModeActionGroup = new QActionGroup(myMenu);
  thickSliceModeActionGroup->setExclusive(true);

  QWidgetAction *m_TSSliderAction = new QWidgetAction(myMenu);
  m_TSSliderAction->setDefaultWidget(_TSWidget);
  myMenu->addAction(m_TSSliderAction);

  QAction* mipThickSlicesAction = new QAction(myMenu);
  mipThickSlicesAction->setActionGroup(thickSliceModeActionGroup);
  mipThickSlicesAction->setText("MIP (max. intensity proj.)");
  mipThickSlicesAction->setCheckable(true);
  mipThickSlicesAction->setChecked(currentThickSlicesMode==1);
  mipThickSlicesAction->setData(1);
  myMenu->addAction( mipThickSlicesAction );

  QAction* sumThickSlicesAction = new QAction(myMenu);
  sumThickSlicesAction->setActionGroup(thickSliceModeActionGroup);
  sumThickSlicesAction->setText("SUM (sum intensity proj.)");
  sumThickSlicesAction->setCheckable(true);
  sumThickSlicesAction->setChecked(currentThickSlicesMode==2);
  sumThickSlicesAction->setData(2);
  myMenu->addAction( sumThickSlicesAction );

  QAction* weightedThickSlicesAction = new QAction(myMenu);
  weightedThickSlicesAction->setActionGroup(thickSliceModeActionGroup);
  weightedThickSlicesAction->setText("WEIGHTED (gaussian proj.)");
  weightedThickSlicesAction->setCheckable(true);
  weightedThickSlicesAction->setChecked(currentThickSlicesMode==3);
  weightedThickSlicesAction->setData(3);
  myMenu->addAction( weightedThickSlicesAction );

  connect( thickSliceModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnThickSlicesModeSelected(QAction*)) );
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

    connect( (QObject*)(m_Controls->m_ScalingCheckbox), SIGNAL(clicked()), this, SLOT(ScalingCheckbox()) );

    connect( (QObject*)(m_Controls->m_OpacitySlider), SIGNAL(spanChanged(double,double)), this, SLOT(OpacityChanged(double,double)) );

    connect((QObject*) m_Controls->m_Wire, SIGNAL(clicked()), (QObject*) this, SLOT(BundleRepresentationWire()));
    connect((QObject*) m_Controls->m_Tube, SIGNAL(clicked()), (QObject*) this, SLOT(BundleRepresentationTube()));
    connect((QObject*) m_Controls->m_Color, SIGNAL(clicked()), (QObject*) this, SLOT(BundleRepresentationColor()));
    connect((QObject*) m_Controls->m_ResetColoring, SIGNAL(clicked()), (QObject*) this, SLOT(BundleRepresentationResetColoring()));
    connect((QObject*) m_Controls->m_Focus, SIGNAL(clicked()), (QObject*) this, SLOT(PlanarFigureFocus()));
    connect((QObject*) m_Controls->m_FiberFading2D, SIGNAL(clicked()), (QObject*) this, SLOT( Fiber2DfadingEFX() ) );
    connect((QObject*) m_Controls->m_FiberThicknessSlider, SIGNAL(sliderReleased()), (QObject*) this, SLOT( FiberSlicingThickness2D() ) );
    connect((QObject*) m_Controls->m_FiberThicknessSlider, SIGNAL(valueChanged(int)), (QObject*) this, SLOT( FiberSlicingUpdateLabel(int) ));

    connect((QObject*) m_Controls->m_Crosshair, SIGNAL(clicked()), (QObject*) this, SLOT(SetInteractor()));

    connect((QObject*) m_Controls->m_PFWidth, SIGNAL(valueChanged(int)), (QObject*) this, SLOT(PFWidth(int)));
    connect((QObject*) m_Controls->m_PFColor, SIGNAL(clicked()), (QObject*) this, SLOT(PFColor()));

    connect((QObject*) m_Controls->m_TDI, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateTdi()));

    connect((QObject*) m_Controls->m_LineWidth, SIGNAL(valueChanged(int)), (QObject*) this, SLOT(LineWidthChanged(int)));
    connect((QObject*) m_Controls->m_TubeRadius, SIGNAL(valueChanged(int)), (QObject*) this, SLOT(TubeRadiusChanged(int)));

    connect((QObject*) m_Controls->m_Welcome, SIGNAL(clicked()), (QObject*) this, SLOT(Welcome()));

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

// set diffusion image channel to b0 volume
void QmitkControlVisualizationPropertiesView::NodeAdded(const mitk::DataNode *node)
{
  mitk::DataNode* notConst = const_cast<mitk::DataNode*>(node);
  if (dynamic_cast<mitk::DiffusionImage<short>*>(notConst->GetData()))
  {
    mitk::DiffusionImage<short>::Pointer dimg = dynamic_cast<mitk::DiffusionImage<short>*>(notConst->GetData());
    notConst->SetIntProperty("DisplayChannel", dimg->GetB0Indices().front());
  }
}

/* OnSelectionChanged is registered to SelectionService, therefore no need to
 implement SelectionService Listener explicitly */
void QmitkControlVisualizationPropertiesView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  if ( !this->IsVisible() )
  {
    // do nothing if nobody wants to see me :-(
    return;
  }

  // deactivate channel slider if no diffusion weighted image or tbss image is selected
  m_Controls->m_DisplayIndex->setVisible(false);
  m_Controls->label_channel->setVisible(false);
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;
    if (node.IsNotNull() && (dynamic_cast<mitk::TbssImage*>(node->GetData()) ||                             
                             dynamic_cast<mitk::DiffusionImage<short>*>(node->GetData())))
    {
      m_Controls->m_DisplayIndex->setVisible(true);
      m_Controls->label_channel->setVisible(true);
    }
  }

  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;
    if( node.IsNotNull() && (dynamic_cast<mitk::QBallImage*>(node->GetData()) || dynamic_cast<mitk::TensorImage*>(node->GetData())) )
    {
      if(m_NodeUsedForOdfVisualization.IsNotNull())
      {
        m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_S", false);
        m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_C", false);
        m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_T", false);
      }
      m_NodeUsedForOdfVisualization = node;
      m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_S", m_GlyIsOn_S);
      m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_C", m_GlyIsOn_C);
      m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_T", m_GlyIsOn_T);
      if(m_MultiWidget)
        m_MultiWidget->RequestUpdate();

      m_Controls->m_TSMenu->setVisible(false);  // deactivate mip etc. for tensor and q-ball images
      break;
    }
    else
      m_Controls->m_TSMenu->setVisible(true);
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

  QString label = "Channel %1";
  label = label.arg(dispIndex);
  m_Controls->label_channel->setText(label);

  std::vector<std::string> sets;
  sets.push_back("DiffusionImage");
  sets.push_back("TbssImage");

  std::vector<std::string>::iterator it = sets.begin();
  while(it != sets.end())
  {
    std::string s = *it;
    mitk::DataStorage::SetOfObjects::Pointer set =
      ActiveSet(s);

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

    it++;
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
  m_GlyIsOn_S = m_Controls->m_VisibleOdfsON_S->isChecked();
  if (m_NodeUsedForOdfVisualization.IsNull())
  {
    MITK_WARN << "ODF visualization activated but m_NodeUsedForOdfVisualization is NULL";
    return;
  }
  m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_S", m_GlyIsOn_S);
  VisibleOdfsON(0);
}

void QmitkControlVisualizationPropertiesView::VisibleOdfsON_T()
{
  m_GlyIsOn_T = m_Controls->m_VisibleOdfsON_T->isChecked();
  if (m_NodeUsedForOdfVisualization.IsNull())
  {
    MITK_WARN << "ODF visualization activated but m_NodeUsedForOdfVisualization is NULL";
    return;
  }
  m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_T", m_GlyIsOn_T);
  VisibleOdfsON(1);
}

void QmitkControlVisualizationPropertiesView::VisibleOdfsON_C()
{
  m_GlyIsOn_C = m_Controls->m_VisibleOdfsON_C->isChecked();
  if (m_NodeUsedForOdfVisualization.IsNull())
  {
    MITK_WARN << "ODF visualization activated but m_NodeUsedForOdfVisualization is NULL";
    return;
  }
  m_NodeUsedForOdfVisualization->SetBoolProperty("VisibleOdfs_C", m_GlyIsOn_C);
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

  set = ActiveSet("Image");
  SetLevelWindowProp(set,"opaclevelwindow", olw);

  m_Controls->m_OpacityMinFaLabel->setText(QString::number(l,'f',2) + " : " + QString::number(u,'f',2));

  if(m_MultiWidget)
    m_MultiWidget->RequestUpdate();

}

void QmitkControlVisualizationPropertiesView::ScalingCheckbox()
{
  m_Controls->m_ScalingFrame->setVisible(
      m_Controls->m_ScalingCheckbox->isChecked());

  if(!m_Controls->m_ScalingCheckbox->isChecked())
  {
    m_Controls->m_AdditionalScaling->setCurrentIndex(0);
    m_Controls->m_ScalingFactor->setValue(1.0);
  }
}

void QmitkControlVisualizationPropertiesView::Fiber2DfadingEFX()
{
  if (m_SelectedNode)
  {
    bool currentMode;
    m_SelectedNode->GetBoolProperty("Fiber2DfadeEFX", currentMode);
    m_SelectedNode->SetProperty("Fiber2DfadeEFX", mitk::BoolProperty::New(!currentMode));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }

}

void QmitkControlVisualizationPropertiesView::FiberSlicingThickness2D()
{
  if (m_SelectedNode)
  {


    float fibThickness = m_Controls->m_FiberThicknessSlider->value() * 0.1;
    m_SelectedNode->SetProperty("Fiber2DSliceThickness", mitk::FloatProperty::New(fibThickness));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void QmitkControlVisualizationPropertiesView::FiberSlicingUpdateLabel(int value)
{
  QString label = "Range %1";
  label = label.arg(value * 0.1);
  m_Controls->label_range->setText(label);

}

void QmitkControlVisualizationPropertiesView::BundleRepresentationWire()
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

void QmitkControlVisualizationPropertiesView::BundleRepresentationTube()
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

void QmitkControlVisualizationPropertiesView::BundleRepresentationColor()
{
  if(m_SelectedNode)
  {
    QColor color = QColorDialog::getColor();
    if (!color.isValid())
      return;

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

void QmitkControlVisualizationPropertiesView::BundleRepresentationResetColoring()
{
    if(m_SelectedNode)
  {
    m_Controls->m_Color->setAutoFillBackground(true);
    QString styleSheet = "background-color:rgb(255,255,255)";
    m_Controls->m_Color->setStyleSheet(styleSheet);

    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(4));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    m_SelectedNode->SetProperty("ColorCoding",mitk::IntProperty::New(0));
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void QmitkControlVisualizationPropertiesView::PlanarFigureFocus()
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

void QmitkControlVisualizationPropertiesView::SetInteractor()
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

void QmitkControlVisualizationPropertiesView::PFWidth(int w)
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

void QmitkControlVisualizationPropertiesView::PFColor()
{

  QColor color = QColorDialog::getColor();
  if (!color.isValid())
    return;

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
  m_SelectedNode->SetProperty( "color", mitk::ColorProperty::New(color.red()/255.0, color.green()/255.0, color.blue()/255.0));

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkControlVisualizationPropertiesView::GenerateTdi()
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
    name += "_TDI";
    node->SetName(name.toStdString());
    node->SetVisibility(true);

    GetDataStorage()->Add(node);
  }
}

void QmitkControlVisualizationPropertiesView::LineWidthChanged(int w)
{
  QString label = "Width %1";
  label = label.arg(w);
  m_Controls->label_linewidth->setText(label);
  BundleRepresentationWire();
}

void QmitkControlVisualizationPropertiesView::TubeRadiusChanged(int r)
{
  QString label = "Radius %1";
  label = label.arg(r / 100.0);
  m_Controls->label_tuberadius->setText(label);
  this->BundleRepresentationTube();
}

void QmitkControlVisualizationPropertiesView::Welcome()
{
  berry::PlatformUI::GetWorkbench()->GetIntroManager()->ShowIntro(
   GetSite()->GetWorkbenchWindow(), false);
}
