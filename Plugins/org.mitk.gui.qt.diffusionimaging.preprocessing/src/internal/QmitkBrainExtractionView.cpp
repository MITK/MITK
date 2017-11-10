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

//misc
#define _USE_MATH_DEFINES
#include <math.h>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkBrainExtractionView.h"

// MITK
#include <mitkNodePredicateDataType.h>

// Qt
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QTimer>

#include <mitkNodePredicateDimension.h>
#include <mitkNodePredicateAnd.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <mitkIOUtil.h>
#include <mitkIPythonService.h>
#include <itkResampleImageFilter.h>
#include <mitkImageCast.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkThresholdImageFilter.h>
#include <itksys/SystemTools.hxx>

#define _USE_MATH_DEFINES
#include <math.h>


FileDownloader::FileDownloader(QObject *parent) : QObject(parent)
{

}

void FileDownloader::download(QUrl url)
{
  connect(&m_WebCtrl, SIGNAL (finished(QNetworkReply*)), SLOT (Downloaded(QNetworkReply*)));

  QNetworkRequest request(url);
  m_WebCtrl.get(request);
}

void FileDownloader::Downloaded(QNetworkReply *reply)
{
  MITK_INFO << "FileDownloader::Downloaded TESTTEST";
  QFile localFile("/home/neher/test_download.tar.gz");
  if (!localFile.open(QIODevice::WriteOnly))
    return;
  localFile.write(reply->readAll());
  localFile.close();
  delete reply;

  QMessageBox::information(nullptr, "FILE DOWNLOADED", "BLABL");
}

FileDownloader::~FileDownloader()
{

}

const std::string QmitkBrainExtractionView::VIEW_ID = "org.mitk.views.brainextraction";

QmitkBrainExtractionView::QmitkBrainExtractionView()
  : QmitkAbstractView()
  , m_Controls( 0 )
  , m_DiffusionImage( nullptr )
{

}

// Destructor
QmitkBrainExtractionView::~QmitkBrainExtractionView()
{
}

void QmitkBrainExtractionView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkBrainExtractionViewControls;
    m_Controls->setupUi( parent );
    connect( m_Controls->m_ImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGUI()) );
    connect( m_Controls->m_StartButton, SIGNAL(clicked()), this, SLOT(StartBrainExtraction()) );
    this->m_Parent = parent;

    m_Controls->m_ImageBox->SetDataStorage(this->GetDataStorage());
    mitk::NodePredicateDimension::Pointer dimPred = mitk::NodePredicateDimension::New(3);
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImagePredicate = mitk::TNodePredicateDataType<mitk::Image>::New();
    m_Controls->m_ImageBox->SetPredicate(mitk::NodePredicateAnd::New(isImagePredicate,dimPred));

    UpdateGUI();

    std::string module_library_full_path = us::GetModuleContext()->GetModule()->GetLocation();
    std::string library_file;
    itksys::SystemTools::SplitProgramPath(module_library_full_path, m_ModulePath, library_file);
  }
}

void QmitkBrainExtractionView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& )
{
}

void QmitkBrainExtractionView::UpdateGUI()
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
    m_Controls->m_StartButton->setEnabled(true);
  else
    m_Controls->m_StartButton->setEnabled(false);
}

void QmitkBrainExtractionView::SetFocus()
{
  UpdateGUI();
  m_Controls->m_StartButton->setFocus();
}

void QmitkBrainExtractionView::StartBrainExtraction()
{
//  FileDownloader dl;
//  dl.download(QUrl("http://mitk.org/download/releases/MITK-2012.06/MITK-2012.06.0-src.tar.gz"));

  mitk::DataNode::Pointer node = m_Controls->m_ImageBox->GetSelectedNode();
  mitk::Image::Pointer mitk_image = dynamic_cast<mitk::Image*>(node->GetData());

  us::ModuleContext* context = us::GetModuleContext();
  us::ServiceReference<mitk::IPythonService> m_PythonServiceRef = context->GetServiceReference<mitk::IPythonService>();
  mitk::IPythonService* m_PythonService = dynamic_cast<mitk::IPythonService*> ( context->GetService<mitk::IPythonService>(m_PythonServiceRef) );
  mitk::IPythonService::ForceLoadModule();

  m_PythonService->Execute("import SimpleITK as sitk");
  m_PythonService->Execute("import SimpleITK._SimpleITK as _SimpleITK");
  m_PythonService->Execute("import numpy");
  m_PythonService->CopyToPythonAsSimpleItkImage( mitk_image, "in_image");
  m_PythonService->Execute("model=\""+m_ModulePath+"/brain_extraction_model.model\"");
  m_PythonService->ExecuteScript(m_ModulePath + "/brain_extraction_script.py");

  {
    mitk::Image::Pointer image = m_PythonService->CopySimpleItkImageFromPython("brain_mask");
    mitk::DataNode::Pointer corrected_node = mitk::DataNode::New();
    corrected_node->SetData( image );
    QString name(node->GetName().c_str());
    name += "_BrainMask";
    corrected_node->SetName(name.toStdString());
    GetDataStorage()->Add(corrected_node, node);
  }

  {
    mitk::Image::Pointer image = m_PythonService->CopySimpleItkImageFromPython("brain_extracted");
    mitk::DataNode::Pointer corrected_node = mitk::DataNode::New();
    corrected_node->SetData( image );
    QString name(node->GetName().c_str());
    name += "_SkullStripped";
    corrected_node->SetName(name.toStdString());
    GetDataStorage()->Add(corrected_node, node);
  }
}
