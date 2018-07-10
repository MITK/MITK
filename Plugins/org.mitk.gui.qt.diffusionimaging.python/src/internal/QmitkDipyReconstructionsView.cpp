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
#include "QmitkDipyReconstructionsView.h"

#include <mitkNodePredicateIsDWI.h>
#include <mitkIPythonService.h>
#include <boost/lexical_cast.hpp>
#include <QFile>
#include <QMessageBox>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkOdfImage.h>
#include <mitkImageCast.h>

const std::string QmitkDipyReconstructionsView::VIEW_ID = "org.mitk.views.dipyreconstruction";

QmitkDipyReconstructionsView::QmitkDipyReconstructionsView()
  : QmitkAbstractView()
  , m_Controls( 0 )
{

}

// Destructor
QmitkDipyReconstructionsView::~QmitkDipyReconstructionsView()
{
}

void QmitkDipyReconstructionsView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkDipyReconstructionsViewControls;
    m_Controls->setupUi( parent );
    connect( m_Controls->m_ImageBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGUI()) );
    connect( m_Controls->m_StartButton, SIGNAL(clicked()), this, SLOT(StartFit()) );
    this->m_Parent = parent;

    m_Controls->m_ImageBox->SetDataStorage(this->GetDataStorage());
    mitk::NodePredicateIsDWI::Pointer isDwi = mitk::NodePredicateIsDWI::New();
    m_Controls->m_ImageBox->SetPredicate( isDwi );

    UpdateGUI();
  }
}

void QmitkDipyReconstructionsView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& )
{
}

void QmitkDipyReconstructionsView::UpdateGUI()
{
  if (m_Controls->m_ImageBox->GetSelectedNode().IsNotNull())
    m_Controls->m_StartButton->setEnabled(true);
  else
    m_Controls->m_StartButton->setEnabled(false);
}

void QmitkDipyReconstructionsView::SetFocus()
{
  UpdateGUI();
  m_Controls->m_StartButton->setFocus();
}

void QmitkDipyReconstructionsView::StartFit()
{
  mitk::DataNode::Pointer node = m_Controls->m_ImageBox->GetSelectedNode();
  mitk::Image::Pointer input_image = dynamic_cast<mitk::Image*>(node->GetData());

  try
  {
    // get python script as string
    QString data;
    QString fileName(":/QmitkDiffusionImaging/dipy_reconstructions.py");
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

    // get bvals and bvecs
    std::locale::global(std::locale::classic());
    std::string bvals = "[";
    std::string bvecs = "[";
    auto gcont = mitk::DiffusionPropertyHelper::GetGradientContainer(input_image);
    auto bvaluevector = mitk::DiffusionPropertyHelper::GetBValueVector(input_image);
    int b0_count = 0;
    for (unsigned int i=0; i<gcont->Size(); ++i)
    {
      bvals += boost::lexical_cast<std::string>(bvaluevector.at(i));
      if (bvaluevector.at(i)==0)
        b0_count++;
      auto g = gcont->GetElement(i);

      if (g.two_norm()>0.000001)
        g /= g.two_norm();
      bvecs += "[" + boost::lexical_cast<std::string>(g[0]) + "," + boost::lexical_cast<std::string>(g[1]) + "," + boost::lexical_cast<std::string>(g[2]) + "]";

      if (i<gcont->Size()-1)
      {
        bvals += ", ";
        bvecs += ", ";
      }
    }
    bvals += "]";
    bvecs += "]";
    if (b0_count==0)
    {
      QMessageBox::warning(nullptr, "Error", "No b=0 volume found. Do your b-values need rounding? Use the Preprocessing View for rounding b-values,", QMessageBox::Ok);
      return;
    }


    us::ModuleContext* context = us::GetModuleContext();
    us::ServiceReference<mitk::IPythonService> m_PythonServiceRef = context->GetServiceReference<mitk::IPythonService>();
    mitk::IPythonService* m_PythonService = dynamic_cast<mitk::IPythonService*> ( context->GetService<mitk::IPythonService>(m_PythonServiceRef) );
    mitk::IPythonService::ForceLoadModule();

    m_PythonService->Execute("import SimpleITK as sitk");
    m_PythonService->Execute("import SimpleITK._SimpleITK as _SimpleITK");
    m_PythonService->Execute("import numpy");
    m_PythonService->CopyToPythonAsSimpleItkImage( input_image, "in_image");
    std::string model = "3D-SHORE";
    switch(m_Controls->m_ModelBox->currentIndex())
    {
    case 0:
      model = "3D-SHORE";
      break;
    case 1:
      model = "SFM";
      break;
    case 2:
      model = "CSD";
      break;
    case 3:
      model = "CSA-QBALL";
      break;
    }
    m_PythonService->Execute("model_type='"+model+"'");
    m_PythonService->Execute("calculate_peak_image=False");
    m_PythonService->Execute("data=False");
    m_PythonService->Execute("bvals=" + bvals);
    m_PythonService->Execute("bvecs=" + bvecs);
    m_PythonService->Execute(data.toStdString(), mitk::IPythonService::MULTI_LINE_COMMAND);

    {
      mitk::OdfImage::ItkOdfImageType::Pointer itkImg = mitk::OdfImage::ItkOdfImageType::New();

      mitk::Image::Pointer out_image = m_PythonService->CopySimpleItkImageFromPython("odf_image");
      mitk::CastToItkImage(out_image, itkImg);

      mitk::OdfImage::Pointer image = mitk::OdfImage::New();
      image->InitializeByItk( itkImg.GetPointer() );
      image->SetVolume( itkImg->GetBufferPointer() );

      mitk::DataNode::Pointer odfs = mitk::DataNode::New();
      odfs->SetData( image );
      QString name(node->GetName().c_str());
      odfs->SetName(name.toStdString() + "_" + model);
      GetDataStorage()->Add(odfs, node);
    }
  }
  catch(...)
  {
    QMessageBox::warning(nullptr, "Error", "File could not be processed.\nIs pytorch installed on your system?\nDoes your script use the correct input and output variable names (in: in_image & model, out: brain_mask & brain_extracted)?", QMessageBox::Ok);
  }
}
