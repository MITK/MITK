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

//#define MBILOG_ENABLE_DEBUG

#include "QmitkPreprocessingView.h"
#include "mitkDiffusionImagingConfigure.h"

// qt includes
#include <QMessageBox>

// itk includes
#include "itkTimeProbe.h"

// mitk includes
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"

#include "mitkNodePredicateDataType.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include "itkVectorContainer.h"

#include "mitkProperties.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunction.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"

#include "itkB0ImageExtractionImageFilter.h"
#include "itkBrainMaskExtractionImageFilter.h"
#include "itkCastImageFilter.h"

#include "berryIStructuredSelection.h"
#include "berryIWorkbenchWindow.h"
#include "berryISelectionService.h"

#include <QTableWidgetItem>
#include <QTableWidget>

const std::string QmitkPreprocessingView::VIEW_ID =
"org.mitk.views.preprocessing";

#define DI_INFO MITK_INFO("DiffusionImaging")


typedef float TTensorPixelType;

using namespace berry;

struct PrpSelListener : ISelectionListener
{

  berryObjectMacro(PrpSelListener);

  PrpSelListener(QmitkPreprocessingView* view)
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
      bool foundDwiVolume = false;
      m_View->m_DiffusionImage = NULL;

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
            foundDwiVolume = true;
            m_View->m_DiffusionImage = dynamic_cast<mitk::DiffusionImage<DiffusionPixelType>*>(node->GetData());
          }
        }
      }

      m_View->m_Controls->m_ButtonBrainMask->setEnabled(foundDwiVolume);
      m_View->m_Controls->m_ButtonAverageGradients->setEnabled(foundDwiVolume);
      m_View->m_Controls->m_ButtonExtractB0->setEnabled(foundDwiVolume);
      m_View->m_Controls->m_ModifyMeasurementFrame->setEnabled(foundDwiVolume);
      m_View->m_Controls->m_MeasurementFrameTable->setEnabled(foundDwiVolume);

      if (foundDwiVolume)
      {
        vnl_matrix_fixed< double, 3, 3 > mf = m_View->m_DiffusionImage->GetMeasurementFrame();
        for (int r=0; r<3; r++)
          for (int c=0; c<3; c++)
          {
            QTableWidgetItem* item = m_View->m_Controls->m_MeasurementFrameTable->item(r,c);
            delete item;
            item = new QTableWidgetItem();
            item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            item->setText(QString::number(mf.get(r,c)));
            m_View->m_Controls->m_MeasurementFrameTable->setItem(r,c,item);
          }
        m_View->m_Controls->m_GradientsLabel->setText(QString::number(m_View->m_DiffusionImage->GetNumDirections()));

        if (m_View->m_DiffusionImage->IsMultiBval())
          m_View->m_Controls->m_BvalLabel->setText("Acquisition with multiple b-values!");
        else
          m_View->m_Controls->m_BvalLabel->setText(QString::number(m_View->m_DiffusionImage->GetB_Value()));
      }
      else
      {
        for (int r=0; r<3; r++)
          for (int c=0; c<3; c++)
          {
            QTableWidgetItem* item = m_View->m_Controls->m_MeasurementFrameTable->item(r,c);
            delete item;
            item = new QTableWidgetItem();
            m_View->m_Controls->m_MeasurementFrameTable->setItem(r,c,item);
          }
        m_View->m_Controls->m_GradientsLabel->setText("-");
        m_View->m_Controls->m_BvalLabel->setText("-");
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

  QmitkPreprocessingView* m_View;
};

QmitkPreprocessingView::QmitkPreprocessingView()
: QmitkFunctionality(),
m_Controls(NULL),
m_MultiWidget(NULL),
m_DiffusionImage(NULL)
{
}

QmitkPreprocessingView::QmitkPreprocessingView(const QmitkPreprocessingView& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QmitkPreprocessingView::~QmitkPreprocessingView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
}

void QmitkPreprocessingView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkPreprocessingViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_MeasurementFrameTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    m_Controls->m_MeasurementFrameTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  }

  m_SelListener = berry::ISelectionListener::Pointer(new PrpSelListener(this));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<PrpSelListener>()->DoSelectionChanged(sel);
}

void QmitkPreprocessingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkPreprocessingView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkPreprocessingView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ButtonAverageGradients), SIGNAL(clicked()), this, SLOT(AverageGradients()) );
    connect( (QObject*)(m_Controls->m_ButtonExtractB0), SIGNAL(clicked()), this, SLOT(ExtractB0()) );
    connect( (QObject*)(m_Controls->m_ButtonBrainMask), SIGNAL(clicked()), this, SLOT(BrainMask()) );
    connect( (QObject*)(m_Controls->m_ModifyMeasurementFrame), SIGNAL(clicked()), this, SLOT(ApplyMesurementFrame()) );
  }
}

void QmitkPreprocessingView::Activated()
{
  QmitkFunctionality::Activated();

  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<PrpSelListener>()->DoSelectionChanged(sel);
}

void QmitkPreprocessingView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkPreprocessingView::ApplyMesurementFrame()
{
  if (m_DiffusionImage.IsNull())
    return;
  vnl_matrix_fixed< double, 3, 3 > mf;
  for (int r=0; r<3; r++)
    for (int c=0; c<3; c++)
    {
      QTableWidgetItem* item = m_Controls->m_MeasurementFrameTable->item(r,c);
      if (!item)
        return;
      mf[r][c] = item->text().toDouble();
    }
  m_DiffusionImage->SetMeasurementFrame(mf);
}

void QmitkPreprocessingView::ExtractB0()
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
        if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
        {
          set->InsertElement(at++, node);
        }
      }
    }

    DoExtractB0(set);

  }
}

void QmitkPreprocessingView::DoExtractB0
(mitk::DataStorage::SetOfObjects::Pointer inImages)
{
  typedef mitk::DiffusionImage<DiffusionPixelType>              DiffusionImageType;
  typedef DiffusionImageType::GradientDirectionContainerType    GradientContainerType;

  int nrFiles = inImages->size();
  if (!nrFiles) return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( inImages->begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( inImages->end() );

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    DiffusionImageType* vols =
      static_cast<DiffusionImageType*>(
      (*itemiter)->GetData());

    std::string nodename;
    (*itemiter)->GetStringProperty("name", nodename);

    // Extract image using found index
    typedef itk::B0ImageExtractionImageFilter<short, short> FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(vols->GetVectorImage());
    filter->SetDirections(vols->GetDirections());
    filter->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( filter->GetOutput() );
    mitkImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( mitkImage );
    node->SetProperty( "name", mitk::StringProperty::New(nodename + "_B0"));

    GetDefaultDataStorage()->Add(node);

    ++itemiter;
  }


}

void QmitkPreprocessingView::AverageGradients()
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
        if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
        {
          set->InsertElement(at++, node);
        }
      }
    }

    DoAverageGradients(set);

  }

}

void QmitkPreprocessingView::DoAverageGradients
(mitk::DataStorage::SetOfObjects::Pointer inImages)
{
  int nrFiles = inImages->size();
  if (!nrFiles) return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( inImages->begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( inImages->end() );

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    mitk::DiffusionImage<DiffusionPixelType>* vols =
      static_cast<mitk::DiffusionImage<DiffusionPixelType>*>(
      (*itemiter)->GetData());

    vols->AverageRedundantGradients(m_Controls->m_Blur->value());

    ++itemiter;
  }
}

void QmitkPreprocessingView::BrainMask()
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
        if(QString("DiffusionImage").compare(node->GetData()->GetNameOfClass())==0)
        {
          set->InsertElement(at++, node);
        }
      }
    }

    DoBrainMask(set);

  }

}

void QmitkPreprocessingView::DoBrainMask
(mitk::DataStorage::SetOfObjects::Pointer inImages)
{
  int nrFiles = inImages->size();
  if (!nrFiles) return;

  mitk::DataStorage::SetOfObjects::const_iterator itemiter( inImages->begin() );
  mitk::DataStorage::SetOfObjects::const_iterator itemiterend( inImages->end() );

  std::vector<mitk::DataNode::Pointer> nodes;
  while ( itemiter != itemiterend ) // for all items
  {

    mitk::DiffusionImage<DiffusionPixelType>* vols =
      static_cast<mitk::DiffusionImage<DiffusionPixelType>*>(
      (*itemiter)->GetData());

    std::string nodename;
    (*itemiter)->GetStringProperty("name", nodename);

    // Extract image using found index
    typedef itk::B0ImageExtractionImageFilter<short, short> FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput(vols->GetVectorImage());
    filter->SetDirections(vols->GetDirections());

    typedef itk::CastImageFilter<itk::Image<short,3>, itk::Image<unsigned short,3> > CastFilterType;
    CastFilterType::Pointer castfilter = CastFilterType::New();
    castfilter->SetInput(filter->GetOutput());

    typedef itk::BrainMaskExtractionImageFilter<unsigned char> MaskFilterType;
    MaskFilterType::Pointer maskfilter = MaskFilterType::New();
    maskfilter->SetInput(castfilter->GetOutput());
    maskfilter->Update();

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitkImage->InitializeByItk( maskfilter->GetOutput() );
    mitkImage->SetVolume( maskfilter->GetOutput()->GetBufferPointer() );
    mitk::DataNode::Pointer node=mitk::DataNode::New();
    node->SetData( mitkImage );
    node->SetProperty( "name", mitk::StringProperty::New(nodename + "_Mask"));

    GetDefaultDataStorage()->Add(node);

    ++itemiter;
  }
}
