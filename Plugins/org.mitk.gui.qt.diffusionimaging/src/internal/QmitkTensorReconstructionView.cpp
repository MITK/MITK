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

#include "QmitkTensorReconstructionView.h"
#include "mitkDiffusionImagingConfigure.h"

// qt includes
#include <QMessageBox>

// itk includes
#include "itkTimeProbe.h"
//#include "itkTensor.h"

// mitk includes
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"

#include "mitkNodePredicateDataType.h"
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include "mitkTeemDiffusionTensor3DReconstructionImageFilter.h"
#include "itkDiffusionTensor3DReconstructionImageFilter.h"
#include "itkTensorImageToDiffusionImageFilter.h"
#include "itkPointShell.h"
#include "itkVector.h"

#include "mitkProperties.h"
#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include "mitkDiffusionImageMapper.h"

#include "berryIStructuredSelection.h"
#include "berryIWorkbenchWindow.h"
#include "berryISelectionService.h"

#include <itkTensorImageToQBallImageFilter.h>

const std::string QmitkTensorReconstructionView::VIEW_ID =
"org.mitk.views.tensorreconstruction";

#define DI_INFO MITK_INFO("DiffusionImaging")

typedef float TTensorPixelType;

using namespace berry;

struct TrSelListener : ISelectionListener
{

  berryObjectMacro(TrSelListener);

  TrSelListener(QmitkTensorReconstructionView* view)
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
      bool foundDwiVolume = false;
      bool foundTensorVolume = false;

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
          }

          // only look at interesting types
          if(QString("TensorImage").compare(node->GetData()->GetNameOfClass())==0)
          {
            foundTensorVolume = true;
          }
        }
      }

      m_View->m_Controls->m_ItkReconstruction->setEnabled(foundDwiVolume);
      m_View->m_Controls->m_TeemReconstruction->setEnabled(foundDwiVolume);

      m_View->m_Controls->m_TensorsToDWIButton->setEnabled(foundTensorVolume);
      m_View->m_Controls->m_TensorsToQbiButton->setEnabled(foundTensorVolume);


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

  QmitkTensorReconstructionView* m_View;
};

QmitkTensorReconstructionView::QmitkTensorReconstructionView()
: QmitkFunctionality(),
m_Controls(NULL),
m_MultiWidget(NULL)
{
}

QmitkTensorReconstructionView::QmitkTensorReconstructionView(const QmitkTensorReconstructionView& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QmitkTensorReconstructionView::~QmitkTensorReconstructionView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->RemovePostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
}

void QmitkTensorReconstructionView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkTensorReconstructionViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    QStringList items;
    items << "LLS (Linear Least Squares)"
      << "MLE (Maximum Likelihood)"
      << "NLS (Nonlinear Least Squares)"
      << "WLS (Weighted Least Squares)";
    m_Controls->m_TensorEstimationTeemEstimationMethodCombo->addItems(items);
    m_Controls->m_TensorEstimationTeemEstimationMethodCombo->setCurrentIndex(0);

    m_Controls->m_TensorEstimationManualThreashold->setChecked(false);
    m_Controls->m_TensorEstimationTeemSigmaEdit->setText("NaN");
    m_Controls->m_TensorEstimationTeemNumItsSpin->setValue(1);
    m_Controls->m_TensorEstimationTeemFuzzyEdit->setText("0.0");
    m_Controls->m_TensorEstimationTeemMinValEdit->setText("1.0");

    m_Controls->m_TensorEstimationTeemNumItsLabel_2->setEnabled(true);
    m_Controls->m_TensorEstimationTeemNumItsSpin->setEnabled(true);

    m_Controls->m_TensorsToDWIBValueEdit->setText("1000");

    Advanced1CheckboxClicked();
    Advanced2CheckboxClicked();
    TeemCheckboxClicked();

#ifndef DIFFUSION_IMAGING_EXTENDED
    m_Controls->m_TeemToggle->setVisible(false);
#endif

    // define data type for combobox
    //m_Controls->m_ImageSelector->SetDataStorage( this->GetDefaultDataStorage() );
    //m_Controls->m_ImageSelector->SetPredicate( mitk::NodePredicateDataType::New("DiffusionImage") );
  }

  m_SelListener = berry::ISelectionListener::Pointer(new TrSelListener(this));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<TrSelListener>()->DoSelectionChanged(sel);
}

void QmitkTensorReconstructionView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<TrSelListener>()->DoSelectionChanged(sel);
  m_MultiWidget = &stdMultiWidget;
}

void QmitkTensorReconstructionView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkTensorReconstructionView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_TeemToggle), SIGNAL(clicked()), this, SLOT(TeemCheckboxClicked()) );
    connect( (QObject*)(m_Controls->m_ItkReconstruction), SIGNAL(clicked()), this, SLOT(ItkReconstruction()) );
    connect( (QObject*)(m_Controls->m_TeemReconstruction), SIGNAL(clicked()), this, SLOT(TeemReconstruction()) );
    connect( (QObject*)(m_Controls->m_TensorEstimationTeemEstimationMethodCombo), SIGNAL(currentIndexChanged(int)), this, SLOT(MethodChoosen(int)) );
    connect( (QObject*)(m_Controls->m_Advanced1), SIGNAL(clicked()), this, SLOT(Advanced1CheckboxClicked()) );
    connect( (QObject*)(m_Controls->m_Advanced2), SIGNAL(clicked()), this, SLOT(Advanced2CheckboxClicked()) );
    connect( (QObject*)(m_Controls->m_TensorEstimationManualThreashold), SIGNAL(clicked()), this, SLOT(ManualThresholdClicked()) );
    connect( (QObject*)(m_Controls->m_TensorsToDWIButton), SIGNAL(clicked()), this, SLOT(TensorsToDWI()) );
    connect( (QObject*)(m_Controls->m_TensorsToQbiButton), SIGNAL(clicked()), this, SLOT(TensorsToQbi()) );
  }
}

void QmitkTensorReconstructionView::TeemCheckboxClicked()
{
  m_Controls->groupBox_3->setVisible(m_Controls->
    m_TeemToggle->isChecked());
}

void QmitkTensorReconstructionView::Advanced1CheckboxClicked()
{
  bool check = m_Controls->
    m_Advanced1->isChecked();

  m_Controls->frame->setVisible(check);
}

void QmitkTensorReconstructionView::Advanced2CheckboxClicked()
{
  bool check = m_Controls->
    m_Advanced2->isChecked();

  m_Controls->frame_2->setVisible(check);
}

void QmitkTensorReconstructionView::ManualThresholdClicked()
{
  m_Controls->m_TensorReconstructionThreasholdEdit_2->setEnabled(
    m_Controls->m_TensorEstimationManualThreashold->isChecked());
}

void QmitkTensorReconstructionView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkTensorReconstructionView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkTensorReconstructionView::MethodChoosen(int method)
{
  m_Controls->m_TensorEstimationTeemNumItsLabel_2->setEnabled(method==3);
  m_Controls->m_TensorEstimationTeemNumItsSpin->setEnabled(method==3);
}

void QmitkTensorReconstructionView::ItkReconstruction()
{
  Reconstruct(0);
}

void QmitkTensorReconstructionView::TeemReconstruction()
{
  Reconstruct(1);
}

void QmitkTensorReconstructionView::Reconstruct(int method)
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

    if(method == 0)
    {
      ItkTensorReconstruction(set);
    }

    if(method == 1)
    {
      TeemTensorReconstruction(set);
    }

  }
}

void QmitkTensorReconstructionView::ItkTensorReconstruction
(mitk::DataStorage::SetOfObjects::Pointer inImages)
{
  try
  {
    itk::TimeProbe clock;

    int nrFiles = inImages->size();
    if (!nrFiles) return;

    QString status;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(nrFiles);

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
      ++itemiter;

      // TENSOR RECONSTRUCTION
      clock.Start();
      MBI_INFO << "Tensor reconstruction ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
        "Tensor reconstruction for %s", nodename.c_str()).toAscii());
      typedef itk::DiffusionTensor3DReconstructionImageFilter<
        DiffusionPixelType, DiffusionPixelType, TTensorPixelType > TensorReconstructionImageFilterType;
      TensorReconstructionImageFilterType::Pointer tensorReconstructionFilter =
        TensorReconstructionImageFilterType::New();
      tensorReconstructionFilter->SetGradientImage( vols->GetDirections(), vols->GetVectorImage() );
      tensorReconstructionFilter->SetBValue(vols->GetB_Value());
      tensorReconstructionFilter->SetThreshold( m_Controls->m_TensorReconstructionThreasholdEdit->text().toFloat() );
      tensorReconstructionFilter->Update();
      clock.Stop();
      MBI_DEBUG << "took " << clock.GetMeanTime() << "s.";

      // TENSORS TO DATATREE
      mitk::TensorImage::Pointer image = mitk::TensorImage::New();


      typedef itk::Image<itk::DiffusionTensor3D<TTensorPixelType>, 3> TensorImageType;

      TensorImageType::Pointer tensorImage;
      tensorImage = tensorReconstructionFilter->GetOutput();




      // Check the tensor for negative eigenvalues
      if(m_Controls->m_CheckNegativeEigenvalues->isChecked())
      {
        typedef itk::ImageRegionIterator<TensorImageType> TensorImageIteratorType;
        TensorImageIteratorType tensorIt(tensorImage, tensorImage->GetRequestedRegion());
        tensorIt.GoToBegin();

        while(!tensorIt.IsAtEnd())
        {

          typedef itk::DiffusionTensor3D<TTensorPixelType> TensorType;
          //typedef itk::Tensor<TTensorPixelType, 3> TensorType2;

          TensorType tensor = tensorIt.Get();
         // TensorType2 tensor2;

          /*
          for(int i=0; i<tensor.GetNumberOfComponents(); i++)
          {
            tensor2.SetNthComponent(i, tensor.GetNthComponent(i));
          }

          typedef vnl_symmetric_eigensystem< TTensorPixelType >  SymEigenSystemType;
          SymEigenSystemType eig (tensor2.GetVnlMatrix());
          for(unsigned int i=0; i<eig.D.size(); i++)
          {
            if (eig.D[i] < 0.0 )
            {
              tensor.Fill(0.0);
              tensorIt.Set(tensor);
            }
          }*/


          TensorType::EigenValuesArrayType ev;
          tensor.ComputeEigenValues(ev);
          for(unsigned int i=0; i<ev.Size(); i++)
          {
            if(ev[i] < 0.0)
            {
              tensor.Fill(0.0);
              tensorIt.Set(tensor);
              break;
            }
          }


          ++tensorIt;
        }
      }


      image->InitializeByItk( tensorImage.GetPointer() );
      image->SetVolume( tensorReconstructionFilter->GetOutput()->GetBufferPointer() );
      mitk::DataNode::Pointer node=mitk::DataNode::New();
      node->SetData( image );

      QString newname;
      newname = newname.append(nodename.c_str());
      newname = newname.append("_dti");

      SetDefaultNodeProperties(node, newname.toStdString());
      nodes.push_back(node);

      mitk::ProgressBar::GetInstance()->Progress();

    }

    std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
    for(nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
      GetDefaultDataStorage()->Add(*nodeIt);

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Finished Processing %d Files", nrFiles).toAscii());
    m_MultiWidget->RequestUpdate();

  }
  catch (itk::ExceptionObject &ex)
  {
    MBI_INFO << ex ;
    return ;
  }
}



void QmitkTensorReconstructionView::TeemTensorReconstruction
(mitk::DataStorage::SetOfObjects::Pointer inImages)
{
  try
  {
    itk::TimeProbe clock;

    int nrFiles = inImages->size();
    if (!nrFiles) return;

    QString status;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(nrFiles);

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
      ++itemiter;

      // TENSOR RECONSTRUCTION
      clock.Start();
      MBI_INFO << "Teem Tensor reconstruction ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
        "Teem Tensor reconstruction for %s", nodename.c_str()).toAscii());
      typedef mitk::TeemDiffusionTensor3DReconstructionImageFilter<
        DiffusionPixelType, TTensorPixelType > TensorReconstructionImageFilterType;
      TensorReconstructionImageFilterType::Pointer tensorReconstructionFilter =
        TensorReconstructionImageFilterType::New();
      tensorReconstructionFilter->SetInput( vols );
      if(!m_Controls->m_TensorEstimationTeemSigmaEdit->text().contains(QString("NaN")))
        tensorReconstructionFilter->SetSigma( m_Controls->m_TensorEstimationTeemSigmaEdit->text().toFloat() );
      switch(m_Controls->m_TensorEstimationTeemEstimationMethodCombo->currentIndex())
      {
        //  items << "LLS (Linear Least Squares)"
        //<< "MLE (Maximum Likelihood)"
        //<< "NLS (Nonlinear Least Squares)"
        //<< "WLS (Weighted Least Squares)";
      case 0:
        tensorReconstructionFilter->SetEstimationMethod(mitk::TeemTensorEstimationMethodsLLS);
        break;
      case 1:
        tensorReconstructionFilter->SetEstimationMethod(mitk::TeemTensorEstimationMethodsMLE);
        break;
      case 2:
        tensorReconstructionFilter->SetEstimationMethod(mitk::TeemTensorEstimationMethodsNLS);
        break;
      case 3:
        tensorReconstructionFilter->SetEstimationMethod(mitk::TeemTensorEstimationMethodsWLS);
        break;
      default:
        tensorReconstructionFilter->SetEstimationMethod(mitk::TeemTensorEstimationMethodsLLS);
      }
      tensorReconstructionFilter->SetNumIterations( m_Controls->m_TensorEstimationTeemNumItsSpin->value() );
      if(m_Controls->m_TensorEstimationManualThreashold->isChecked())
        tensorReconstructionFilter->SetConfidenceThreshold( m_Controls->m_TensorReconstructionThreasholdEdit_2->text().toDouble() );
      tensorReconstructionFilter->SetConfidenceFuzzyness( m_Controls->m_TensorEstimationTeemFuzzyEdit->text().toFloat() );
      tensorReconstructionFilter->SetMinPlausibleValue( m_Controls->m_TensorEstimationTeemMinValEdit->text().toDouble() );
      tensorReconstructionFilter->Update();
      clock.Stop();
      MBI_DEBUG << "took " << clock.GetMeanTime() << "s." ;

      // TENSORS TO DATATREE
      mitk::DataNode::Pointer node2=mitk::DataNode::New();
      node2->SetData( tensorReconstructionFilter->GetOutputItk() );

      QString newname;
      newname = newname.append(nodename.c_str());
      newname = newname.append("_dtix");

      SetDefaultNodeProperties(node2, newname.toStdString());
      nodes.push_back(node2);

      mitk::ProgressBar::GetInstance()->Progress();

    }

    std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
    for(nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
      GetDefaultDataStorage()->Add(*nodeIt);

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Finished Processing %d Files", nrFiles).toAscii());
    m_MultiWidget->RequestUpdate();

  }
  catch (itk::ExceptionObject &ex)
  {
    MBI_INFO << ex ;
    return ;
  }
}

void QmitkTensorReconstructionView::SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name)
{
  node->SetProperty( "ShowMaxNumber", mitk::IntProperty::New( 500 ) );
  node->SetProperty( "Scaling", mitk::FloatProperty::New( 1.0 ) );
  node->SetProperty( "Normalization", mitk::OdfNormalizationMethodProperty::New());
  node->SetProperty( "ScaleBy", mitk::OdfScaleByProperty::New());
  node->SetProperty( "IndexParam1", mitk::FloatProperty::New(2));
  node->SetProperty( "IndexParam2", mitk::FloatProperty::New(1));
  node->SetProperty( "visible", mitk::BoolProperty::New( true ) );
  node->SetProperty( "VisibleOdfs", mitk::BoolProperty::New( false ) );
  node->SetProperty ("layer", mitk::IntProperty::New(100));
  node->SetProperty( "DoRefresh", mitk::BoolProperty::New( true ) );
  //node->SetProperty( "opacity", mitk::FloatProperty::New(1.0f) );

  node->SetProperty( "name", mitk::StringProperty::New(name) );
}


//node->SetProperty( "volumerendering", mitk::BoolProperty::New( false ) );
//node->SetProperty( "use color", mitk::BoolProperty::New( true ) );
//node->SetProperty( "texture interpolation", mitk::BoolProperty::New( true ) );
//node->SetProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New() );
//node->SetProperty( "layer", mitk::IntProperty::New(0));
//node->SetProperty( "in plane resample extent by geometry", mitk::BoolProperty::New( false ) );
//node->SetOpacity(1.0f);
//node->SetColor(1.0,1.0,1.0);
//node->SetVisibility(true);
//node->SetProperty( "IsTensorVolume", mitk::BoolProperty::New( true ) );

//mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
//mitk::LevelWindow levelwindow;
////  levelwindow.SetAuto( image );
//levWinProp->SetLevelWindow( levelwindow );
//node->GetPropertyList()->SetProperty( "levelwindow", levWinProp );

//// add a default rainbow lookup table for color mapping
//if(!node->GetProperty("LookupTable"))
//{
//  mitk::LookupTable::Pointer mitkLut = mitk::LookupTable::New();
//  vtkLookupTable* vtkLut = mitkLut->GetVtkLookupTable();
//  vtkLut->SetHueRange(0.6667, 0.0);
//  vtkLut->SetTableRange(0.0, 20.0);
//  vtkLut->Build();
//  mitk::LookupTableProperty::Pointer mitkLutProp = mitk::LookupTableProperty::New();
//  mitkLutProp->SetLookupTable(mitkLut);
//  node->SetProperty( "LookupTable", mitkLutProp );
//}
//if(!node->GetProperty("binary"))
//  node->SetProperty( "binary", mitk::BoolProperty::New( false ) );

//// add a default transfer function
//mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
//node->SetProperty ( "TransferFunction", mitk::TransferFunctionProperty::New ( tf.GetPointer() ) );

//// set foldername as string property
//mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New( name );
//node->SetProperty( "name", nameProp );

void QmitkTensorReconstructionView::TensorsToDWI()
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
        if(QString("TensorImage").compare(node->GetData()->GetNameOfClass())==0)
        {
          set->InsertElement(at++, node);
        }
      }
    }

    DoTensorsToDWI(set);
  }
}

void QmitkTensorReconstructionView::TensorsToQbi()
{
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  for (int i=0; i<nodes.size(); i++)
  {
    mitk::DataNode::Pointer tensorImageNode = nodes.at(i);
    MITK_INFO << "starting Q-Ball estimation";

    typedef float                                       TTensorPixelType;
    typedef itk::DiffusionTensor3D< TTensorPixelType >  TensorPixelType;
    typedef itk::Image< TensorPixelType, 3 >            TensorImageType;

    TensorImageType::Pointer itkvol = TensorImageType::New();
    mitk::CastToItkImage<TensorImageType>(dynamic_cast<mitk::TensorImage*>(tensorImageNode->GetData()), itkvol);

    typedef itk::TensorImageToQBallImageFilter< TTensorPixelType, TTensorPixelType > FilterType;
    FilterType::Pointer filter = FilterType::New();
    filter->SetInput( itkvol );
    filter->Update();

    typedef itk::Vector<TTensorPixelType,QBALL_ODFSIZE>  OutputPixelType;
    typedef itk::Image<OutputPixelType,3>                OutputImageType;

    mitk::QBallImage::Pointer image = mitk::QBallImage::New();
    OutputImageType::Pointer outimg = filter->GetOutput();
    image->InitializeByItk( outimg.GetPointer() );
    image->SetVolume( outimg->GetBufferPointer() );
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData( image );
    QString newname;
    newname = newname.append(tensorImageNode->GetName().c_str());
    newname = newname.append("_qbi");
    node->SetName(newname.toAscii());
    GetDefaultDataStorage()->Add(node);
  }
}

void QmitkTensorReconstructionView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  if ( !this->IsVisible() )
    return;

}

template<int ndirs>
std::vector<itk::Vector<double,3> > QmitkTensorReconstructionView::MakeGradientList()
{
  std::vector<itk::Vector<double,3> > retval;
  vnl_matrix_fixed<double, 3, ndirs>* U =
    itk::PointShell<ndirs, vnl_matrix_fixed<double, 3, ndirs> >::DistributePointShell();

  for(int i=0; i<ndirs;i++)
  {
    itk::Vector<double,3> v;
    v[0] = U->get(0,i); v[1] = U->get(1,i); v[2] = U->get(2,i);
    retval.push_back(v);
  }
  return retval;
}

void QmitkTensorReconstructionView::DoTensorsToDWI
(mitk::DataStorage::SetOfObjects::Pointer inImages)
{
  try
  {
    itk::TimeProbe clock;

    int nrFiles = inImages->size();
    if (!nrFiles) return;

    QString status;
    mitk::ProgressBar::GetInstance()->AddStepsToDo(nrFiles);

    mitk::DataStorage::SetOfObjects::const_iterator itemiter( inImages->begin() );
    mitk::DataStorage::SetOfObjects::const_iterator itemiterend( inImages->end() );

    std::vector<mitk::DataNode::Pointer> nodes;
    while ( itemiter != itemiterend ) // for all items
    {

      std::string nodename;
      (*itemiter)->GetStringProperty("name", nodename);

      mitk::TensorImage* vol =
        static_cast<mitk::TensorImage*>((*itemiter)->GetData());

      ++itemiter;

      typedef float                                       TTensorPixelType;
      typedef itk::DiffusionTensor3D< TTensorPixelType >  TensorPixelType;
      typedef itk::Image< TensorPixelType, 3 >            TensorImageType;


      TensorImageType::Pointer itkvol = TensorImageType::New();
      mitk::CastToItkImage<TensorImageType>(vol, itkvol);

      typedef itk::TensorImageToDiffusionImageFilter<
        TTensorPixelType, DiffusionPixelType > FilterType;

      FilterType::GradientListType gradientList;

      switch(m_Controls->m_TensorsToDWINumDirsSelect->currentIndex())
      {
      case 0:
        gradientList = MakeGradientList<12>();
        break;
      case 1:
        gradientList = MakeGradientList<42>();
        break;
      case 2:
        gradientList = MakeGradientList<92>();
        break;
      case 3:
        gradientList = MakeGradientList<162>();
        break;
      case 4:
        gradientList = MakeGradientList<252>();
        break;
      case 5:
        gradientList = MakeGradientList<362>();
        break;
      case 6:
        gradientList = MakeGradientList<492>();
        break;
      case 7:
        gradientList = MakeGradientList<642>();
        break;
      case 8:
        gradientList = MakeGradientList<812>();
        break;
      case 9:
        gradientList = MakeGradientList<1002>();
        break;
      default:
        gradientList = MakeGradientList<92>();

      }

      double bVal = m_Controls->m_TensorsToDWIBValueEdit->text().toDouble();

      // DWI ESTIMATION
      clock.Start();
      MBI_INFO << "DWI Estimation ";
      mitk::StatusBar::GetInstance()->DisplayText(status.sprintf(
        "DWI Estimation for %s", nodename.c_str()).toAscii());
      FilterType::Pointer filter = FilterType::New();
      filter->SetInput( itkvol );
      filter->SetBValue(bVal);
      filter->SetGradientList(gradientList);
      //filter->SetNumberOfThreads(1);
      filter->Update();
      clock.Stop();
      MBI_DEBUG << "took " << clock.GetMeanTime() << "s.";

      itk::Vector<double,3> v;
      v[0] = 0; v[1] = 0; v[2] = 0;
      gradientList.push_back(v);

      // TENSORS TO DATATREE
      mitk::DiffusionImage<DiffusionPixelType>::Pointer image = mitk::DiffusionImage<DiffusionPixelType>::New();
      image->SetVectorImage( filter->GetOutput() );
      image->SetB_Value(bVal);
      image->SetDirections(gradientList);
      image->SetOriginalDirections(gradientList);
      image->InitializeFromVectorImage();
      mitk::DataNode::Pointer node=mitk::DataNode::New();
      node->SetData( image );

      mitk::DiffusionImageMapper<short>::SetDefaultProperties(node);

      QString newname;
      newname = newname.append(nodename.c_str());
      newname = newname.append("_dwi");
      node->SetName(newname.toAscii());

      nodes.push_back(node);

      mitk::ProgressBar::GetInstance()->Progress();

    }

    std::vector<mitk::DataNode::Pointer>::iterator nodeIt;
    for(nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
      GetDefaultDataStorage()->Add(*nodeIt);

    mitk::StatusBar::GetInstance()->DisplayText(status.sprintf("Finished Processing %d Files", nrFiles).toAscii());
    m_MultiWidget->RequestUpdate();

  }
  catch (itk::ExceptionObject &ex)
  {
    MBI_INFO << ex ;
    return ;
  }
}
