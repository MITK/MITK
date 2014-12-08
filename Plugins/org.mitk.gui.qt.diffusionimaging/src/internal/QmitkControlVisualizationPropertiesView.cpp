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

#include "QmitkControlVisualizationPropertiesView.h"

#include "mitkNodePredicateDataType.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include "mitkResliceMethodProperty.h"
#include "mitkRenderingManager.h"

#include "mitkTbssImage.h"
#include "mitkPlanarFigure.h"
#include "mitkFiberBundleX.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
#include "mitkFiberBundleInteractor.h"
#include "mitkPlanarFigureInteractor.h"
#include <mitkQBallImage.h>
#include <mitkTensorImage.h>
#include <mitkDiffusionImage.h>
#include <mitkConnectomicsNetwork.h>
#include "mitkGlobalInteraction.h"
#include "usModuleRegistry.h"

#include "mitkPlaneGeometry.h"

#include "berryIWorkbenchWindow.h"
#include "berryIWorkbenchPage.h"
#include "berryISelectionService.h"
#include "berryConstants.h"
#include "berryPlatformUI.h"

#include "itkRGBAPixel.h"
#include <itkTractDensityImageFilter.h>

#include "qwidgetaction.h"
#include "qcolordialog.h"
#include <itkMultiThreader.h>

#define ROUND(a) ((a)>0 ? (int)((a)+0.5) : -(int)(0.5-(a)))

static bool DetermineAffectedImageSlice( const mitk::Image* image, const mitk::PlaneGeometry* plane, int& affectedDimension, int& affectedSlice )
{
  assert(image);
  assert(plane);

  // compare normal of plane to the three axis vectors of the image
  mitk::Vector3D normal       = plane->GetNormal();
  mitk::Vector3D imageNormal0 = image->GetSlicedGeometry()->GetAxisVector(0);
  mitk::Vector3D imageNormal1 = image->GetSlicedGeometry()->GetAxisVector(1);
  mitk::Vector3D imageNormal2 = image->GetSlicedGeometry()->GetAxisVector(2);

  normal.Normalize();
  imageNormal0.Normalize();
  imageNormal1.Normalize();
  imageNormal2.Normalize();

  imageNormal0.SetVnlVector( vnl_cross_3d<mitk::ScalarType>(normal.GetVnlVector(),imageNormal0.GetVnlVector()) );
  imageNormal1.SetVnlVector( vnl_cross_3d<mitk::ScalarType>(normal.GetVnlVector(),imageNormal1.GetVnlVector()) );
  imageNormal2.SetVnlVector( vnl_cross_3d<mitk::ScalarType>(normal.GetVnlVector(),imageNormal2.GetVnlVector()) );

  double eps( 0.00001 );
  // axial
  if ( imageNormal2.GetNorm() <= eps )
  {
    affectedDimension = 2;
  }
  // sagittal
  else if ( imageNormal1.GetNorm() <= eps )
  {
    affectedDimension = 1;
  }
  // frontal
  else if ( imageNormal0.GetNorm() <= eps )
  {
    affectedDimension = 0;
  }
  else
  {
    affectedDimension = -1; // no idea
    return false;
  }

  // determine slice number in image
  mitk::BaseGeometry* imageGeometry = image->GetGeometry(0);
  mitk::Point3D testPoint = imageGeometry->GetCenter();
  mitk::Point3D projectedPoint;
  plane->Project( testPoint, projectedPoint );

  mitk::Point3D indexPoint;

  imageGeometry->WorldToIndex( projectedPoint, indexPoint );
  affectedSlice = ROUND( indexPoint[affectedDimension] );
  MITK_DEBUG << "indexPoint " << indexPoint << " affectedDimension " << affectedDimension << " affectedSlice " << affectedSlice;

  // check if this index is still within the image
  if ( affectedSlice < 0 || affectedSlice >= static_cast<int>(image->GetDimension(affectedDimension)) ) return false;

  return true;
}

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

        // check if node has data,
        // if some helper nodes are shown in the DataManager, the GetData() returns 0x0 which would lead to SIGSEV
        mitk::BaseData* nodeData = node->GetData();

        if(nodeData != NULL )
        {
          if(dynamic_cast<mitk::PlanarFigure*>(nodeData) != 0)
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

          if(dynamic_cast<mitk::FiberBundleX*>(nodeData) != 0)
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

            int width;
            node->GetIntProperty("LineWidth", width);
            m_View->m_Controls->m_LineWidth->setValue(width);

            float range;
            node->GetFloatProperty("Fiber2DSliceThickness",range);
            mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(node->GetData());
            mitk::BaseGeometry::Pointer geo = fib->GetGeometry();
            mitk::ScalarType max = geo->GetExtentInMM(0);
            max = std::max(max, geo->GetExtentInMM(1));
            max = std::max(max, geo->GetExtentInMM(2));

            m_View->m_Controls->m_FiberThicknessSlider->setMaximum(max * 10);

            m_View->m_Controls->m_FiberThicknessSlider->setValue(range * 10);

          }

        } // check node data != NULL
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

            mitk::BaseData* nodeData = node->GetData();

            if(nodeData != NULL )
            {
              // only look at interesting types
              if(QString("DiffusionImage").compare(nodeData->GetNameOfClass())==0)
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
                m_View->m_Controls->m_DisplayIndexSpinBox->setValue(val);

                QString label = "Channel %1";
                label = label.arg(val);
                m_View->m_Controls->label_channel->setText(label);

                int maxVal = (dynamic_cast<mitk::DiffusionImage<short>* >(nodeData))->GetVectorImage()->GetVectorLength();
                m_View->m_Controls->m_DisplayIndex->setMaximum(maxVal-1);
                m_View->m_Controls->m_DisplayIndexSpinBox->setMaximum(maxVal-1);
              }

              if(QString("TbssImage").compare(nodeData->GetNameOfClass())==0)
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
                m_View->m_Controls->m_DisplayIndexSpinBox->setValue(val);

                QString label = "Channel %1";
                label = label.arg(val);
                m_View->m_Controls->label_channel->setText(label);

                int maxVal = (dynamic_cast<mitk::TbssImage* >(nodeData))->GetImage()->GetVectorLength();
                m_View->m_Controls->m_DisplayIndex->setMaximum(maxVal-1);
                m_View->m_Controls->m_DisplayIndexSpinBox->setMaximum(maxVal-1);
              }


              else if(QString("QBallImage").compare(nodeData->GetNameOfClass())==0)
              {
                foundMultipleOdfImages = foundQBIVolume || foundTensorVolume;
                foundQBIVolume = true;
                ApplySettings(node);
              }

              else if(QString("TensorImage").compare(nodeData->GetNameOfClass())==0)
              {
                foundMultipleOdfImages = foundQBIVolume || foundTensorVolume;
                foundTensorVolume = true;
                ApplySettings(node);
              }

              else if(QString("Image").compare(nodeData->GetNameOfClass())==0)
              {
                foundImage = true;
                mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(nodeData);
                if(img.IsNotNull()
                  && img->GetPixelType().GetPixelType() == itk::ImageIOBase::RGBA
                  && img->GetPixelType().GetComponentType() == itk::ImageIOBase::UCHAR )
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

            } // END CHECK node != NULL
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

    }
  }

  void SelectionChanged(IWorkbenchPart::Pointer part, ISelection::ConstPointer selection)
  {
    // check, if selection comes from datamanager
    if (part)
    {
      QString partname(part->GetPartName().c_str());
      if(partname.compare("Data Manager")==0)
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
  m_GlyIsOn_T(false),
  m_FiberBundleObserverTag(0),
  m_Color(NULL)
{
  currentThickSlicesMode = 1;
  m_MyMenu = NULL;
  int numThread = itk::MultiThreader::GetGlobalMaximumNumberOfThreads();
  if (numThread > 12)
    numThread = 12;
  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(numThread);
}

QmitkControlVisualizationPropertiesView::QmitkControlVisualizationPropertiesView(const QmitkControlVisualizationPropertiesView& other)
{
  Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
}

QmitkControlVisualizationPropertiesView::~QmitkControlVisualizationPropertiesView()
{
  if(m_SlicesRotationObserverTag1 )
  {
    mitk::SlicesCoordinator::Pointer coordinator = m_MultiWidget->GetSlicesRotator();
    if( coordinator.IsNotNull() )
      coordinator->RemoveObserver(m_SlicesRotationObserverTag1);
  }
  if( m_SlicesRotationObserverTag2)
  {
    mitk::SlicesCoordinator::Pointer coordinator = m_MultiWidget->GetSlicesRotator();
    if( coordinator.IsNotNull() )
      coordinator->RemoveObserver(m_SlicesRotationObserverTag1);
  }

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
  n = this->m_MultiWidget->GetWidgetPlane1(); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
  n = this->m_MultiWidget->GetWidgetPlane2(); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
  n = this->m_MultiWidget->GetWidgetPlane3(); if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );

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
    n = this->m_MultiWidget->GetWidgetPlane1();
    if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( 0 ) );
    if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
    if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( false ) );

    n = this->m_MultiWidget->GetWidgetPlane2();
    if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( 0 ) );
    if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
    if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( false ) );

    n = this->m_MultiWidget->GetWidgetPlane3();
    if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( 0 ) );
    if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
    if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( false ) );
  }
  else
  {
    mitk::DataNode* n;
    n = this->m_MultiWidget->GetWidgetPlane1();
    if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
    if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
    if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( (num>0) ) );

    n = this->m_MultiWidget->GetWidgetPlane2();
    if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
    if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
    if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( (num>0) ) );

    n = this->m_MultiWidget->GetWidgetPlane3();
    if(n) n->SetProperty( "reslice.thickslices", mitk::ResliceMethodProperty::New( currentThickSlicesMode ) );
    if(n) n->SetProperty( "reslice.thickslices.num", mitk::IntProperty::New( num ) );
    if(n) n->SetProperty( "reslice.thickslices.showarea", mitk::BoolProperty::New( (num>0) ) );
  }

  m_TSLabel->setText(QString::number(num*2+1));

  mitk::BaseRenderer::Pointer renderer = this->GetActiveStdMultiWidget()->GetRenderWindow1()->GetRenderer();
  if(renderer.IsNotNull())
    renderer->SendUpdateSlice();

  renderer = this->GetActiveStdMultiWidget()->GetRenderWindow2()->GetRenderer();
  if(renderer.IsNotNull())
    renderer->SendUpdateSlice();

  renderer = this->GetActiveStdMultiWidget()->GetRenderWindow3()->GetRenderer();
  if(renderer.IsNotNull())
    renderer->SendUpdateSlice();

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

    // hide warning (ODFs in rotated planes)
    m_Controls->m_lblRotatedPlanesWarning->hide();

    m_MyMenu = new QMenu(parent);

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
    m_Controls->m_OpacitySlider->setMinimumValue(0.0);
    m_Controls->m_OpacitySlider->setMaximumValue(0.0);

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

  if (m_MultiWidget)
  {
    mitk::SlicesCoordinator* coordinator = m_MultiWidget->GetSlicesRotator();
    if (coordinator)
    {
      itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::Pointer command2 = itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::New();
      command2->SetCallbackFunction( this, &QmitkControlVisualizationPropertiesView::SliceRotation );
      m_SlicesRotationObserverTag1 = coordinator->AddObserver( mitk::SliceRotationEvent(), command2 );
    }

    coordinator = m_MultiWidget->GetSlicesSwiveller();
    if (coordinator)
    {
      itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::Pointer command2 = itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::New();
      command2->SetCallbackFunction( this, &QmitkControlVisualizationPropertiesView::SliceRotation );
      m_SlicesRotationObserverTag2 = coordinator->AddObserver( mitk::SliceRotationEvent(), command2 );
    }
  }
}

void QmitkControlVisualizationPropertiesView::SliceRotation(const itk::EventObject&)
{
  // test if plane rotated
  if( m_GlyIsOn_T || m_GlyIsOn_C || m_GlyIsOn_S )
  {
    if( this->IsPlaneRotated() )
    {
      // show label
      m_Controls->m_lblRotatedPlanesWarning->show();
    }
    else
    {
      //hide label
      m_Controls->m_lblRotatedPlanesWarning->hide();
    }
  }
}


void QmitkControlVisualizationPropertiesView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkControlVisualizationPropertiesView::NodeRemoved(const mitk::DataNode* node)
{
}

#include <mitkMessage.h>
void QmitkControlVisualizationPropertiesView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_DisplayIndex), SIGNAL(valueChanged(int)), this, SLOT(DisplayIndexChanged(int)) );
    connect( (QObject*)(m_Controls->m_DisplayIndexSpinBox), SIGNAL(valueChanged(int)), this, SLOT(DisplayIndexChanged(int)) );
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
    connect((QObject*) m_Controls->m_LineWidth, SIGNAL(editingFinished()), (QObject*) this, SLOT(LineWidthChanged()));
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

    // if there is no b0 image in the dataset, the GetB0Indices() returns a vector of size 0
    // and hence we cannot set the Property directly to .front()
    int displayChannelPropertyValue = 0;
    mitk::DiffusionImage<short>::BValueMap map = dimg->GetBValueMap();
    if( map[0].size() > 0)
      displayChannelPropertyValue = map[0].front();

    notConst->SetIntProperty("DisplayChannel", displayChannelPropertyValue );
  }
}

/* OnSelectionChanged is registered to SelectionService, therefore no need to
implement SelectionService Listener explicitly */
void QmitkControlVisualizationPropertiesView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{

  // deactivate channel slider if no diffusion weighted image or tbss image is selected
  m_Controls->m_DisplayIndex->setVisible(false);
  m_Controls->m_DisplayIndexSpinBox->setVisible(false);
  m_Controls->label_channel->setVisible(false);

  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;

    // check if node has data,
    // if some helper nodes are shown in the DataManager, the GetData() returns 0x0 which would lead to SIGSEV
    mitk::BaseData* nodeData = node->GetData();
    if(nodeData == NULL)
      continue;

    if (node.IsNotNull() && (dynamic_cast<mitk::TbssImage*>(nodeData) ||
      dynamic_cast<mitk::DiffusionImage<short>*>(nodeData)))
    {
      m_Controls->m_DisplayIndex->setVisible(true);
      m_Controls->m_DisplayIndexSpinBox->setVisible(true);
      m_Controls->label_channel->setVisible(true);
    }
    else if (node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()))
    {
      if (m_Color.IsNotNull())
        m_Color->RemoveObserver(m_FiberBundleObserverTag);

      itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::Pointer command = itk::ReceptorMemberCommand<QmitkControlVisualizationPropertiesView>::New();
      command->SetCallbackFunction( this, &QmitkControlVisualizationPropertiesView::SetFiberBundleCustomColor );
      m_Color = dynamic_cast<mitk::ColorProperty*>(node->GetProperty("color", NULL));
      if (m_Color.IsNotNull())
        m_FiberBundleObserverTag = m_Color->AddObserver( itk::ModifiedEvent(), command );
    }
  }

  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;

    // check if node has data,
    // if some helper nodes are shown in the DataManager, the GetData() returns 0x0 which would lead to SIGSEV
    mitk::BaseData* nodeData = node->GetData();
    if(nodeData == NULL)
      continue;

    if( node.IsNotNull() && (dynamic_cast<mitk::QBallImage*>(nodeData) || dynamic_cast<mitk::TensorImage*>(nodeData)) )
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
    else if( node.IsNotNull() && dynamic_cast<mitk::ConnectomicsNetwork*>(nodeData) )
      m_Controls->m_TSMenu->setVisible(false);
    else
      m_Controls->m_TSMenu->setVisible(true);
  }

  // if selection changes, set the current selction member and call SellListener::DoSelectionChanged
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<CvpSelListener>()->DoSelectionChanged(sel);

  // adapt thick slice controls
  // THICK SLICE SUPPORT

  if( nodes.size() < 1)
    return;

  mitk::DataNode::Pointer node = nodes.at(0);

  if( node.IsNull() )
    return;

  QMenu *myMenu = m_MyMenu;
  myMenu->clear();

  QActionGroup* thickSlicesActionGroup = new QActionGroup(myMenu);
  thickSlicesActionGroup->setExclusive(true);

  int currentTSMode = 0;
  {
    mitk::ResliceMethodProperty::Pointer m = dynamic_cast<mitk::ResliceMethodProperty*>(node->GetProperty( "reslice.thickslices" ));
    if( m.IsNotNull() )
      currentTSMode = m->GetValueAsId();
  }

  int maxTS = 30;

  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>((*it)->GetData());
    if (image)
    {
      int size = std::max(image->GetDimension(0), std::max(image->GetDimension(1), image->GetDimension(2)));
      if (size>maxTS)
        maxTS=size;
    }
  }
  maxTS /= 2;

  int currentNum = 0;
  {
    mitk::IntProperty::Pointer m = dynamic_cast<mitk::IntProperty*>(node->GetProperty( "reslice.thickslices.num" ));
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

        // check if node has data,
        // if some helper nodes are shown in the DataManager, the GetData() returns 0x0 which would lead to SIGSEV
        const mitk::BaseData* nodeData = node->GetData();
        if(nodeData == NULL)
          continue;

        if(QString(classname.c_str()).compare(nodeData->GetNameOfClass())==0)
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

  m_Controls->m_DisplayIndex->setValue(dispIndex);
  m_Controls->m_DisplayIndexSpinBox->setValue(dispIndex);

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
        basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true );
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

bool QmitkControlVisualizationPropertiesView::IsPlaneRotated()
{

  // for all 2D renderwindows of m_MultiWidget check alignment
  mitk::PlaneGeometry::ConstPointer displayPlane = dynamic_cast<const mitk::PlaneGeometry*>( m_MultiWidget->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2D() );
  if (displayPlane.IsNull()) return false;

  mitk::Image* currentImage = dynamic_cast<mitk::Image* >( m_NodeUsedForOdfVisualization->GetData() );
  if( currentImage == NULL )
  {
    MITK_ERROR << " Casting problems. Returning false";
    return false;
  }

  int affectedDimension(-1);
  int affectedSlice(-1);
  return !(DetermineAffectedImageSlice( currentImage, displayPlane, affectedDimension, affectedSlice ));

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

  //  if(m_MultiWidget)
  //    m_MultiWidget->RequestUpdate();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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
  if (m_SelectedNode && dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData()) )
  {
    bool currentMode;
    m_SelectedNode->GetBoolProperty("Fiber2DfadeEFX", currentMode);
    m_SelectedNode->SetProperty("Fiber2DfadeEFX", mitk::BoolProperty::New(!currentMode));
    dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData())->RequestUpdate2D();
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }

}

void QmitkControlVisualizationPropertiesView::FiberSlicingThickness2D()
{
  if (m_SelectedNode && dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData()))
  {
    float fibThickness = m_Controls->m_FiberThicknessSlider->value() * 0.1;
    float currentThickness = 0;
    m_SelectedNode->GetFloatProperty("Fiber2DSliceThickness", currentThickness);
    if (fabs(fibThickness-currentThickness)<0.001)
      return;
    m_SelectedNode->SetProperty("Fiber2DSliceThickness", mitk::FloatProperty::New(fibThickness));
    dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData())->RequestUpdate2D();
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void QmitkControlVisualizationPropertiesView::FiberSlicingUpdateLabel(int value)
{
  QString label = "Range %1 mm";
  label = label.arg(value * 0.1);
  m_Controls->label_range->setText(label);
  this->FiberSlicingThickness2D();
}

void QmitkControlVisualizationPropertiesView::SetFiberBundleCustomColor(const itk::EventObject& /*e*/)
{
  float color[3];
  m_SelectedNode->GetColor(color);
  m_Controls->m_Color->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";
  styleSheet.append(QString::number(color[0]*255.0));
  styleSheet.append(",");
  styleSheet.append(QString::number(color[1]*255.0));
  styleSheet.append(",");
  styleSheet.append(QString::number(color[2]*255.0));
  styleSheet.append(")");
  m_Controls->m_Color->setStyleSheet(styleSheet);

  m_SelectedNode->SetProperty("color",mitk::ColorProperty::New(color[0], color[1], color[2]));
  mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData());
  fib->SetColorCoding(mitk::FiberBundleX::COLORCODING_CUSTOM);
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
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
    mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData());
    fib->SetColorCoding(mitk::FiberBundleX::COLORCODING_CUSTOM);
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void QmitkControlVisualizationPropertiesView::BundleRepresentationResetColoring()
{
  if(m_SelectedNode)
  {
    MITK_INFO << "reset colorcoding to oBased";
    m_Controls->m_Color->setAutoFillBackground(true);
    QString styleSheet = "background-color:rgb(255,255,255)";
    m_Controls->m_Color->setStyleSheet(styleSheet);
    //    m_SelectedNode->SetProperty("color",NULL);
    m_SelectedNode->SetProperty("color",mitk::ColorProperty::New(1.0, 1.0, 1.0));

    mitk::FiberBundleX::Pointer fib = dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData());
    fib->SetColorCoding(mitk::FiberBundleX::COLORCODING_ORIENTATION_BASED);
    fib->DoColorCodingOrientationBased();
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void QmitkControlVisualizationPropertiesView::PlanarFigureFocus()
{
  if(m_SelectedNode)
  {
    mitk::PlanarFigure* _PlanarFigure = 0;
    _PlanarFigure = dynamic_cast<mitk::PlanarFigure*> (m_SelectedNode->GetData());

    if (_PlanarFigure && _PlanarFigure->GetPlaneGeometry())
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

      const mitk::PlaneGeometry* _PlaneGeometry = _PlanarFigure->GetPlaneGeometry();

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
        const mitk::Point3D& centerP = _PlaneGeometry->GetOrigin();
        selectedRenderWindow->GetSliceNavigationController()->ReorientSlices(
          centerP, _PlaneGeometry->GetNormal());
      }
    }

    // set interactor for new node (if not already set)
    mitk::PlanarFigureInteractor::Pointer figureInteractor
      = dynamic_cast<mitk::PlanarFigureInteractor*>(m_SelectedNode->GetDataInteractor().GetPointer());

    if(figureInteractor.IsNull())
    {
      figureInteractor = mitk::PlanarFigureInteractor::New();
      us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "MitkPlanarFigure" );
      figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
      figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
      figureInteractor->SetDataNode( m_SelectedNode );
    }

    m_SelectedNode->SetProperty("planarfigure.iseditable",mitk::BoolProperty::New(true));
  }
}

void QmitkControlVisualizationPropertiesView::SetInteractor()
{
  typedef std::vector<mitk::DataNode*> Container;
  Container _NodeSet = this->GetDataManagerSelection();
  mitk::DataNode* node = 0;
  mitk::FiberBundleX* bundle = 0;
  mitk::FiberBundleInteractor::Pointer bundleInteractor = 0;

  // finally add all nodes to the model
  for(Container::const_iterator it=_NodeSet.begin(); it!=_NodeSet.end()
    ; it++)
  {
    node = const_cast<mitk::DataNode*>(*it);
    bundle = dynamic_cast<mitk::FiberBundleX*>(node->GetData());

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
    mitk::FiberBundleX* bundle = dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData());
    if(!bundle)
      return;

    typedef float OutPixType;
    typedef itk::Image<OutPixType, 3> OutImageType;

    // run generator
    itk::TractDensityImageFilter< OutImageType >::Pointer generator = itk::TractDensityImageFilter< OutImageType >::New();
    generator->SetFiberBundle(bundle);
    generator->SetOutputAbsoluteValues(true);
    generator->SetUpsamplingFactor(1);
    generator->Update();

    // get result
    OutImageType::Pointer outImg = generator->GetOutput();

    mitk::Image::Pointer img = mitk::Image::New();
    img->InitializeByItk(outImg.GetPointer());
    img->SetVolume(outImg->GetBufferPointer());

    // to datastorage
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(img);
    QString name(m_SelectedNode->GetName().c_str());
    name += "_TDI";
    node->SetName(name.toStdString());
    node->SetVisibility(true);

    GetDataStorage()->Add(node);
  }
}

void QmitkControlVisualizationPropertiesView::LineWidthChanged()
{
  if(m_SelectedNode && dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData()))
  {
    int newWidth = m_Controls->m_LineWidth->value();
    int currentWidth = 0;
    m_SelectedNode->GetIntProperty("LineWidth", currentWidth);
    if (currentWidth==newWidth)
      return;
    m_SelectedNode->SetIntProperty("LineWidth", newWidth);
    dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData())->RequestUpdate2D();
    dynamic_cast<mitk::FiberBundleX*>(m_SelectedNode->GetData())->RequestUpdate3D();
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  }
}

void QmitkControlVisualizationPropertiesView::Welcome()
{
  berry::PlatformUI::GetWorkbench()->GetIntroManager()->ShowIntro(
    GetSite()->GetWorkbenchWindow(), false);
}
