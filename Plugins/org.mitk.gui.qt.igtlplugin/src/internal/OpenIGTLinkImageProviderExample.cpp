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

#include <chrono>

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkRenderWindow.h"

// Qt
#include <QMessageBox>
#include <QFileDialog>

// mitk
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkIOUtil.h>
#include <mitkImageSource.h>
#include <mitkImageToIGTLMessageFilter.h>
#include <mitkImageWriteAccessor.h>

// vtk
#include <vtkSphereSource.h>
#include <vtkMatrix4x4.h>

//
#include "OpenIGTLinkImageProviderExample.h"

// igtl
#include "igtlStringMessage.h"

class MockImageSource : public mitk::ImageSource
{
 public:
  mitkClassMacro(MockImageSource, ImageSource)

      itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      void GenerateData() override
  {
    this->m_Image = mitk::Image::New();

    mitk::PixelType type =
        mitk::MakePixelType<unsigned char, unsigned char, 1>();
    unsigned int dims[3];
    dims[0] = 256;
    dims[1] = 256;
    dims[2] = 1;
    this->m_Image->Initialize(type, 3, dims);

    // We animate the test-image to basically check if we get a decent-ish
    // framerate.
    auto now = std::chrono::high_resolution_clock::now();
    int t = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();
    t /= 4;

    {
      // Scoped, so that we release writeAccess ASAP.
      mitk::ImageWriteAccessor writeAccess(this->m_Image);
      unsigned char* data = (unsigned char*)writeAccess.GetData();

      for (size_t i = 0; i < 256; ++i)
      {
        for (size_t j = 0; j < 256; ++j)
        {
          data[256 * i + j] = (i + j + t) % 256;
        }
      }
      data[0] = 42;
      this->m_Image->Modified();
    }

    vtkMatrix4x4* matrix = vtkMatrix4x4::New();
    for (size_t i = 0; i < 4; ++i)
    {
      for (size_t j = 0; j < 4; ++j)
      {
        matrix->SetElement(i, j, (i != j) ? 0 : (i == 1 && j == 1) ? -1 : 1);
      }
    }
    this->m_Image->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(matrix);

    this->m_Image->Modified();

    this->SetNumberOfIndexedOutputs(1);
    this->SetNthOutput(0, m_Image.GetPointer());
  }

  MockImageSource()
  {
    this->m_Image = mitk::Image::New();
    this->SetNumberOfIndexedOutputs(1);
    this->SetNthOutput(0, m_Image.GetPointer());
  }

 private:
  mitk::Image::Pointer m_Image;
};

const std::string OpenIGTLinkImageProviderExample::VIEW_ID =
    "org.mitk.views.OpenIGTLinkImageProviderExample";

OpenIGTLinkImageProviderExample::~OpenIGTLinkImageProviderExample()
{
  this->DestroyPipeline();
  if (m_IGTLMessageProvider.IsNotNull())
  {
    m_IGTLMessageProvider->UnRegisterMicroservice();
  }
  if (m_ImageToIGTLMsgFilter.IsNotNull())
  {
    m_ImageToIGTLMsgFilter->UnRegisterMicroservice();
  }
}

void OpenIGTLinkImageProviderExample::SetFocus() {}

void OpenIGTLinkImageProviderExample::CreateQtPartControl(QWidget* parent)
{
  m_Source = MockImageSource::New();
  m_Source->Update();

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);

  // create a new OpenIGTLink Client
  m_IGTLServer = mitk::IGTLServer::New();
  m_IGTLServer->SetName("OIGTL Image Provider Example Device");

  // create a new OpenIGTLink Device source
  m_IGTLMessageProvider = mitk::IGTLMessageProvider::New();

  // set the OpenIGTLink server as the source for the device source
  m_IGTLMessageProvider->SetIGTLDevice(m_IGTLServer);
  m_IGTLMessageProvider->SetFPS(30);

  // register the provider so that it can be configured with the IGTL manager
  // plugin. This could be hardcoded but now I already have the fancy plugin.
  m_IGTLMessageProvider->RegisterAsMicroservice();

  connect(m_Controls.m_startPipeline, SIGNAL(clicked()), this,
          SLOT(OnStartPipeline()));
}

void OpenIGTLinkImageProviderExample::OnStartPipeline()
{
  this->CreatePipeline();
}

void OpenIGTLinkImageProviderExample::CreatePipeline()
{
  // create a filter that converts navigation data into IGTL messages
  m_ImageToIGTLMsgFilter = mitk::ImageToIGTLMessageFilter::New();

  // TODO: Get some image source
  m_ImageToIGTLMsgFilter->ConnectTo(m_Source);

  // connect the filters with each other
  // the navigation data player reads a file with recorded navigation data,
  // passes this data to a filter that converts it into a IGTLMessage.
  // The provider is not connected because it will search for fitting services.
  // Once it found the filter it will automatically connect to it.
  // m_ImageToIGTLMsgFilter->ConnectTo(m_NavDataPlayer);

  // define the operation mode for this filter, we want to send tracking data
  // messages
  // m_NavDataToIGTLMsgFilter->SetOperationMode(
  //  mitk::NavigationDataToIGTLMessageFilter::ModeSendTDataMsg);
  //       mitk::NavigationDataToIGTLMessageFilter::ModeSendTransMsg);

  // set the name of this filter to identify it easier
  m_ImageToIGTLMsgFilter->SetName("Image Source From Example");

  // register this filter as micro service. The message provider looks for
  // provided IGTLMessageSources, once it found this microservice and someone
  // requested this data type then the provider will connect with this filter
  // automatically.
  m_ImageToIGTLMsgFilter->RegisterAsMicroservice();
}

void OpenIGTLinkImageProviderExample::DestroyPipeline() {}
