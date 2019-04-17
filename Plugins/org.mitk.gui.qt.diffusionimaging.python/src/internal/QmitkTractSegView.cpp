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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkTractSegView.h"

#include <mitkNodePredicateDataType.h>
#include <mitkIPythonService.h>
#include <boost/lexical_cast.hpp>
#include <QFile>
#include <QMessageBox>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <mitkPeakImage.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkITKImageImport.h>
#include <itkFourDToVectorImageFilter.h>
#include <itkVectorImageToFourDImageFilter.h>
#include <itkSplitVectorImageFilter.h>
#include <itkFlipPeaksFilter.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkLevelWindow.h>
#include <mitkLevelWindowProperty.h>

const std::string QmitkTractSegView::VIEW_ID = "org.mitk.views.tractseg";

QmitkTractSegView::QmitkTractSegView()
  : QmitkAbstractView()
  , m_Controls( 0 )
{

}

// Destructor
QmitkTractSegView::~QmitkTractSegView()
{
}

void QmitkTractSegView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkTractSegViewControls;
    m_Controls->setupUi( parent );
    connect( m_Controls->m_ImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGUI()) );
    connect( m_Controls->m_StartButton, SIGNAL(clicked()), this, SLOT(Start()) );
    connect( m_Controls->m_OutputBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGUI()) );
    connect( m_Controls->m_ModeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGUI()) );
    this->m_Parent = parent;

    m_Controls->m_ImageBox->SetDataStorage(this->GetDataStorage());
    mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("PeakImage");
    m_Controls->m_ImageBox->SetPredicate( isDwi );

    UpdateGUI();
  }
}

void QmitkTractSegView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& )
{
}

void QmitkTractSegView::UpdateGUI()
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
    m_Controls->m_StartButton->setEnabled(true);
  else
    m_Controls->m_StartButton->setEnabled(false);

  m_Controls->m_CollapseBox->setVisible(true);
  m_Controls->m_thresholdFrame->setVisible(true);

  if (m_Controls->m_OutputBox->currentIndex()==3 || m_Controls->m_OutputBox->currentIndex()==1)
    m_Controls->m_thresholdFrame->setVisible(false);

  if (m_Controls->m_OutputBox->currentIndex()>0)
    m_Controls->m_CollapseBox->setVisible(false);
}

void QmitkTractSegView::SetFocus()
{
  UpdateGUI();
  m_Controls->m_StartButton->setFocus();
}

void QmitkTractSegView::Start()
{
  std::locale::global(std::locale::classic());
  mitk::DataNode::Pointer node = m_Controls->m_ImageBox->GetSelectedNode();
  mitk::Image::Pointer input_image = dynamic_cast<mitk::Image*>(node->GetData());

  // get python script as string
  QString data;
  QString fileName(":/QmitkDiffusionImaging/tractseg.py");
  QFile file(fileName);
  if(!file.open(QIODevice::ReadOnly)) {
    qDebug()<<"filenot opened"<<endl;
  }
  else
  {
    qDebug()<<"file opened"<<endl;
    data = file.readAll();
  }
  file.close();

  us::ModuleContext* context = us::GetModuleContext();
  us::ServiceReference<mitk::IPythonService> m_PythonServiceRef = context->GetServiceReference<mitk::IPythonService>();
  mitk::IPythonService* m_PythonService = dynamic_cast<mitk::IPythonService*> ( context->GetService<mitk::IPythonService>(m_PythonServiceRef) );
  mitk::IPythonService::ForceLoadModule();

  typedef mitk::ImageToItk< mitk::PeakImage::ItkPeakImageType > CasterType;
  CasterType::Pointer caster = CasterType::New();
  caster->SetInput(input_image);
  caster->Update();
  mitk::PeakImage::ItkPeakImageType::Pointer itk_peaks = caster->GetOutput();
  if (itk_peaks->GetLargestPossibleRegion().GetSize()[3]!=9)
  {
    QMessageBox::warning(nullptr, "Error", "TractSeg only works with 3-peak images!", QMessageBox::Ok);
    return;
  }

  itk::FlipPeaksFilter< float >::Pointer flipper = itk::FlipPeaksFilter< float >::New();
  flipper->SetInput(itk_peaks);
  flipper->SetFlipX(false);
  flipper->SetFlipY(false);
  flipper->SetFlipZ(true);
  flipper->Update();

  itk::FourDToVectorImageFilter< float >::Pointer converter = itk::FourDToVectorImageFilter< float >::New();
  converter->SetInputImage(flipper->GetOutput());
  converter->GenerateData();
  mitk::Image::Pointer mitk_vec_img = mitk::GrabItkImageMemory( converter->GetOutputImage().GetPointer() );

  m_PythonService->CopyToPythonAsSimpleItkImage( mitk_vec_img, "in_image");

  m_PythonService->Execute("sx=" + boost::lexical_cast<std::string>(itk_peaks->GetLargestPossibleRegion().GetSize()[0]));
  m_PythonService->Execute("sy=" + boost::lexical_cast<std::string>(itk_peaks->GetLargestPossibleRegion().GetSize()[1]));
  m_PythonService->Execute("sz=" + boost::lexical_cast<std::string>(itk_peaks->GetLargestPossibleRegion().GetSize()[2]));

  if (m_PythonService->DoesVariableExist("segmentation"))
    m_PythonService->Execute("del segmentation");

  if(m_Controls->m_ModeBox->currentIndex()==0)
    m_PythonService->Execute("output_type=\"tract_segmentation\"");
  else if(m_Controls->m_ModeBox->currentIndex()==1)
    m_PythonService->Execute("output_type=\"endings_segmentation\"");

  if(m_Controls->m_OutputBox->currentIndex()==3)
    m_PythonService->Execute("output_type=\"TOM\"");


  if (m_Controls->m_CollapseBox->isChecked() && m_Controls->m_OutputBox->currentIndex()==0)
    m_PythonService->Execute("collapse=True");
  else
    m_PythonService->Execute("collapse=False");

  m_PythonService->Execute("get_probs=False");
  m_PythonService->Execute("dropout_sampling=False");

  if (m_Controls->m_OutputBox->currentIndex()==1)
    m_PythonService->Execute("get_probs=True");
  else if (m_Controls->m_OutputBox->currentIndex()==2)
    m_PythonService->Execute("dropout_sampling=True");

  m_PythonService->Execute("threshold=" + boost::lexical_cast<std::string>(m_Controls->m_SegThresholdBox->value()));

  m_PythonService->Execute("verbose=False");
  m_PythonService->Execute(data.toStdString(), mitk::IPythonService::MULTI_LINE_COMMAND);

  // clean up after running script (better way than deleting individual variables?)
  if(m_PythonService->DoesVariableExist("in_image"))
    m_PythonService->Execute("del in_image");
  if(m_PythonService->DoesVariableExist("collapse"))
    m_PythonService->Execute("del collapse");

  // check for errors
  if(!m_PythonService->GetVariable("error_string").empty())
  {
    QMessageBox::warning(nullptr, "Error", QString(m_PythonService->GetVariable("error_string").c_str()), QMessageBox::Ok);
    return;
  }

  std::vector<std::string> small_name_list = {"AF_left", "AF_right", "CA", "CST_left", "CST_right", "CG_left", "CG_right",
                     "ICP_left", "ICP_right", "MCP", "SCP_left", "SCP_right", "ILF_left", "ILF_right",
                     "IFO_left", "IFO_right", "OR_left", "OR_right", "UF_left", "UF_right"};

  std::vector<std::string> large_name_list = {"AF_left", "AF_right", "ATR_left", "ATR_right", "CA", "CC_1", "CC_2", "CC_3", "CC_4", "CC_5",
                                              "CC_6", "CC_7","CG_left","CG_right","CST_left","CST_right" ,"MLF_left","MLF_right","FPT_left"
                                              ,"FPT_right","FX_left","FX_right","ICP_left","ICP_right","IFO_left","IFO_right"
                                              ,"ILF_left" ,"ILF_right","MCP","OR_left","OR_right","POPT_left","POPT_right"
                                              ,"SCP_left","SCP_right","SLF_I_left","SLF_I_right","SLF_II_left","SLF_II_right"
                                              ,"SLF_III_left","SLF_III_right","STR_left","STR_right","UF_left","UF_right"
                                              ,"CC","T_PREF_left","T_PREF_right","T_PREM_left","T_PREM_right","T_PREC_left"
                                              ,"T_PREC_right","T_POSTC_left","T_POSTC_right","T_PAR_left","T_PAR_right","T_OCC_left"
                                              ,"T_OCC_right","ST_FO_left","ST_FO_right","ST_PREF_left","ST_PREF_right","ST_PREM_left"
                                              ,"ST_PREM_right","ST_PREC_left","ST_PREC_right","ST_POSTC_left","ST_POSTC_right"
                                              ,"ST_PAR_left","ST_PAR_right","ST_OCC_left","ST_OCC_right"};

  if (m_PythonService->DoesVariableExist("segmentation"))
  {
    mitk::Image::Pointer out_image = m_PythonService->CopySimpleItkImageFromPython("segmentation");

    if (!m_Controls->m_CollapseBox->isChecked() || m_Controls->m_OutputBox->currentIndex()>0)
    {
      if (m_Controls->m_OutputBox->currentIndex()>0)
      {
        itk::VectorImage<float>::Pointer vectorImage = itk::VectorImage<float>::New();
        mitk::CastToItkImage(out_image, vectorImage);
        itk::SplitVectorImageFilter<float>::Pointer splitter = itk::SplitVectorImageFilter<float>::New();
        splitter->SetInputImage(vectorImage);
        splitter->GenerateData();
        int c = 0;
        for (auto itk_seg : splitter->GetOutputImages())
        {
          mitk::DataNode::Pointer seg = mitk::DataNode::New();
          seg->SetData( mitk::GrabItkImageMemory(itk_seg) );
          seg->SetName(large_name_list.at(c));

          mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
          lut->SetType( mitk::LookupTable::JET_TRANSPARENT );
          mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
          lut_prop->SetLookupTable( lut );
          seg->SetProperty("LookupTable", lut_prop );
          mitk::LevelWindow lw; lw.SetRangeMinMax(0.0,1.0);
          seg->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( lw ) );

          GetDataStorage()->Add(seg, node);
          ++c;
        }
      }
      else
      {
        itk::VectorImage<unsigned char>::Pointer vectorImage = itk::VectorImage<unsigned char>::New();
        mitk::CastToItkImage(out_image, vectorImage);
        itk::SplitVectorImageFilter<unsigned char>::Pointer splitter = itk::SplitVectorImageFilter<unsigned char>::New();
        splitter->SetInputImage(vectorImage);
        splitter->GenerateData();
        int c = 0;
        for (auto itk_seg : splitter->GetOutputImages())
        {
          mitk::DataNode::Pointer seg = mitk::DataNode::New();
          seg->SetData( mitk::GrabItkImageMemory(itk_seg) );
          if (m_Controls->m_ModeBox->currentIndex()==0)
          {
            seg->SetName(large_name_list.at(c));
            seg->SetBoolProperty("binary", true);
          }
          else
          {
            mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
            lut->SetType( mitk::LookupTable::MULTILABEL );
            mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
            lut_prop->SetLookupTable( lut );
            seg->SetProperty("LookupTable", lut_prop );
            mitk::LevelWindow lw; lw.SetRangeMinMax(0,2);
            seg->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( lw ) );

            seg->SetName(large_name_list.at(c));
          }
          GetDataStorage()->Add(seg, node);
          ++c;
        }
      }
    }
    else
    {
      mitk::DataNode::Pointer seg = mitk::DataNode::New();
      seg->SetData(out_image);

      mitk::LevelWindow lw;
      if (m_Controls->m_ModeBox->currentIndex()==0)
      {
        seg->SetName("TractLabels");
        lw.SetRangeMinMax(0, 72);
      }
      else
      {
        seg->SetName("TractEndpointRegionLabels");
        lw.SetRangeMinMax(0, 144);
      }

      if (m_Controls->m_OutputBox->currentIndex()==0)
      {
        mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
        lut->SetType( mitk::LookupTable::MULTILABEL );
        mitk::LookupTableProperty::Pointer lut_prop = mitk::LookupTableProperty::New();
        lut_prop->SetLookupTable( lut );
        seg->SetProperty("LookupTable", lut_prop );
        seg->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( lw ) );
      }

      GetDataStorage()->Add(seg, node);
    }

    m_PythonService->Execute("del segmentation");
  }
  else
  {
    for (int i=0; i<72; ++i)
      if (m_PythonService->DoesVariableExist("tom" + boost::lexical_cast<std::string>(i)))
      {
        mitk::Image::Pointer out_image = m_PythonService->CopySimpleItkImageFromPython("tom" + boost::lexical_cast<std::string>(i));
        itk::VectorImage<float>::Pointer vectorImage = itk::VectorImage<float>::New();
        mitk::CastToItkImage(out_image, vectorImage);

        itk::VectorImageToFourDImageFilter< float >::Pointer converter = itk::VectorImageToFourDImageFilter< float >::New();
        converter->SetInputImage(vectorImage);
        converter->GenerateData();
        mitk::PeakImage::ItkPeakImageType::Pointer itk_peaks = converter->GetOutputImage();

        itk::FlipPeaksFilter< float >::Pointer flipper = itk::FlipPeaksFilter< float >::New();
        flipper->SetInput(itk_peaks);
        flipper->SetFlipX(false);
        flipper->SetFlipY(false);
        flipper->SetFlipZ(true);
        flipper->Update();
        itk_peaks = flipper->GetOutput();

        mitk::Image::Pointer mitk_peaks = dynamic_cast<mitk::Image*>(mitk::PeakImage::New().GetPointer());
        mitk::CastToMitkImage(itk_peaks, mitk_peaks);
        mitk_peaks->SetVolume(itk_peaks->GetBufferPointer());

        mitk::DataNode::Pointer seg = mitk::DataNode::New();
        seg->SetData( mitk_peaks );
        if (!m_Controls->m_CollapseBox->isChecked())
          seg->SetName(large_name_list.at(i) + "_TOM");
        else
          seg->SetName("Collapsed_TOM");
        GetDataStorage()->Add(seg, node);
        m_PythonService->Execute("del tom" + boost::lexical_cast<std::string>(i));
      }
  }
}
