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
#include <mitkIOUtil.h>
#include <mitkIPythonService.h>
#include <itkResampleImageFilter.h>
#include <mitkImageCast.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkThresholdImageFilter.h>
#include <itksys/SystemTools.hxx>
#include <itkB0ImageExtractionToSeparateImageFilter.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <boost/algorithm/string.hpp>

#include <BetData.h>

#define _USE_MATH_DEFINES
#include <math.h>

typedef itksys::SystemTools ist;

const std::string QmitkBrainExtractionView::VIEW_ID = "org.mitk.views.brainextraction";

QmitkBrainExtractionView::QmitkBrainExtractionView()
  : QmitkAbstractView()
  , m_Controls( nullptr )
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
    m_Controls->m_ImageBox->SetPredicate(isImagePredicate);

    UpdateGUI();
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

std::string QmitkBrainExtractionView::GetPythonFile(std::string filename)
{
  std::string out = "";
  std::string exec_dir = QCoreApplication::applicationDirPath().toStdString();
  for (auto dir : mitk::bet::relative_search_dirs)
  {
    if ( ist::FileExists( ist::GetCurrentWorkingDirectory() + dir + filename) )
    {
      out = ist::GetCurrentWorkingDirectory() + dir + filename;
      return out;
    }
    if ( ist::FileExists( exec_dir + dir + filename) )
    {
      out = exec_dir + dir + filename;
      return out;
    }
  }
  for (auto dir : mitk::bet::absolute_search_dirs)
  {
    if ( ist::FileExists( dir + filename) )
    {
      out = dir + filename;
      return out;
    }
  }

  return out;
}

void QmitkBrainExtractionView::StartBrainExtraction()
{
  mitk::DataNode::Pointer node = m_Controls->m_ImageBox->GetSelectedNode();
  mitk::Image::Pointer mitk_image = dynamic_cast<mitk::Image*>(node->GetData());

  bool missing_file = false;
  std::string missing_file_string = "";
  if ( GetPythonFile("run_mitk.py").empty() )
  {
    missing_file_string += "Brain extraction script file missing: run_mitk.py\n\n";
    missing_file = true;
  }

  if ( GetPythonFile("model_final.model").empty() )
  {
    missing_file_string += "Brain extraction model file missing: model_final.model\n\n";
    missing_file = true;
  }

  if ( GetPythonFile("basic_config_just_like_braintumor.py").empty() )
  {
    missing_file_string += "Config file missing: basic_config_just_like_braintumor.py\n\n";
    missing_file = true;
  }

  if (missing_file)
  {
    QMessageBox::warning(nullptr, "Error", (missing_file_string).c_str(), QMessageBox::Ok);
    return;
  }

  try
  {
    us::ModuleContext* context = us::GetModuleContext();
    us::ServiceReference<mitk::IPythonService> m_PythonServiceRef = context->GetServiceReference<mitk::IPythonService>();
    mitk::IPythonService* m_PythonService = dynamic_cast<mitk::IPythonService*> ( context->GetService<mitk::IPythonService>(m_PythonServiceRef) );
    mitk::IPythonService::ForceLoadModule();

    // extend python search path
    m_PythonService->AddAbsoluteSearchDirs(mitk::bet::absolute_search_dirs);
    m_PythonService->AddRelativeSearchDirs(mitk::bet::relative_search_dirs);

    // legacy (can be romved when BetData is updated accordingly)
    m_PythonService->Execute("paths=list()");

    // set input files (model and config)
    m_PythonService->Execute("model_file=\""+GetPythonFile("model_final.model")+"\"");
    m_PythonService->Execute("config_file=\""+GetPythonFile("basic_config_just_like_braintumor.py")+"\"");

    // copy input  image to python
    m_PythonService->CopyToPythonAsSimpleItkImage( mitk_image, "in_image");

    // run segmentation script
    m_PythonService->ExecuteScript( GetPythonFile("run_mitk.py") );

    // clean up after running script (better way than deleting individual variables?)
    if(m_PythonService->DoesVariableExist("in_image"))
      m_PythonService->Execute("del in_image");

    // check for errors
    if(!m_PythonService->GetVariable("error_string").empty())
    {
      QMessageBox::warning(nullptr, "Error", QString(m_PythonService->GetVariable("error_string").c_str()), QMessageBox::Ok);
      return;
    }

    // get output images and add to datastorage
    std::string output_variables = m_PythonService->GetVariable("output_variables");
    std::vector<std::string> outputs;
    boost::split(outputs, output_variables, boost::is_any_of(","));

    std::string output_types = m_PythonService->GetVariable("output_types");
    std::vector<std::string> types;
    boost::split(types, output_types, boost::is_any_of(","));

    for (unsigned int i=0; i<outputs.size(); ++i)
    {
      if (m_PythonService->DoesVariableExist(outputs.at(i)))
      {
        mitk::Image::Pointer image = m_PythonService->CopySimpleItkImageFromPython(outputs.at(i));

        if(types.at(i)=="input" && mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(mitk_image))
        {
          mitk::DiffusionPropertyHelper::CopyProperties(mitk_image, image, true);
          mitk::DiffusionPropertyHelper::InitializeImage(image);
        }

        mitk::DataNode::Pointer corrected_node = mitk::DataNode::New();
        corrected_node->SetData( image );
        std::string name = node->GetName();
        name += "_";
        name += outputs.at(i);
        corrected_node->SetName(name);
        GetDataStorage()->Add(corrected_node, node);
        m_PythonService->Execute("del " + outputs.at(i));

        mitk::RenderingManager::GetInstance()->InitializeViews( corrected_node->GetData()->GetTimeGeometry(),
                                                                mitk::RenderingManager::REQUEST_UPDATE_ALL,
                                                                true);
      }
    }
  }
  catch(...)
  {
    QMessageBox::warning(nullptr, "Error", "File could not be processed.\nIs pytorch installed on your system?\nDoes your script use the correct input and output variable names (in: in_image & model, out: brain_mask & brain_extracted)?", QMessageBox::Ok);
  }
}
