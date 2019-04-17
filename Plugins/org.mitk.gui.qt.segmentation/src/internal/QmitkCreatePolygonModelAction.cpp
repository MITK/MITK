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
#include "QmitkCreatePolygonModelAction.h"

// MITK
#include <mitkShowSegmentationAsSmoothedSurface.h>
#include <mitkShowSegmentationAsSurface.h>
#include <mitkProgressBar.h>
#include <mitkStatusBar.h>

#include <mitkIRenderWindowPart.h>
#include <mitkIRenderingManager.h>

// Blueberry
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryPlatform.h>
#include <berryIWorkbenchPage.h>

using namespace berry;
using namespace mitk;
using namespace std;

QmitkCreatePolygonModelAction::QmitkCreatePolygonModelAction()
{
}

QmitkCreatePolygonModelAction::~QmitkCreatePolygonModelAction()
{
}

void QmitkCreatePolygonModelAction::Run(const QList<DataNode::Pointer> &selectedNodes)
{
  DataNode::Pointer selectedNode = selectedNodes[0];
  Image::Pointer image = dynamic_cast<mitk::Image *>(selectedNode->GetData());

  if (image.IsNull())
  {
    return;
  }

  try
  {
    // Get preference properties for smoothing and decimation
    IPreferencesService* prefService = Platform::GetPreferencesService();
    IPreferences::Pointer segPref = prefService->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

    bool smoothingHint = segPref->GetBool("smoothing hint", true);
    ScalarType smoothing = segPref->GetDouble("smoothing value", 1.0);
    ScalarType decimation = segPref->GetDouble("decimation rate", 0.5);

    if (smoothingHint)
    {
      smoothing = 0.0;
      Vector3D spacing = image->GetGeometry()->GetSpacing();

      for (Vector3D::Iterator iter = spacing.Begin(); iter != spacing.End(); ++iter)
        smoothing = max(smoothing, *iter);
    }

    ShowSegmentationAsSurface::Pointer surfaceFilter = ShowSegmentationAsSurface::New();

    // Activate callback functions
    itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::Pointer successCommand = itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::New();
    successCommand->SetCallbackFunction(this, &QmitkCreatePolygonModelAction::OnSurfaceCalculationDone);
    surfaceFilter->AddObserver(ResultAvailable(), successCommand);

    itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::Pointer errorCommand = itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::New();
    errorCommand->SetCallbackFunction(this, &QmitkCreatePolygonModelAction::OnSurfaceCalculationDone);
    surfaceFilter->AddObserver(ProcessingError(), errorCommand);

    // set filter parameter
    surfaceFilter->SetDataStorage(*m_DataStorage);
    surfaceFilter->SetPointerParameter("Input", image);
    surfaceFilter->SetPointerParameter("Group node", selectedNode);
    surfaceFilter->SetParameter("Show result", true);
    surfaceFilter->SetParameter("Sync visibility", false);
    surfaceFilter->SetParameter("Median kernel size", 3u);
    surfaceFilter->SetParameter("Decimate mesh", m_IsDecimated);
    surfaceFilter->SetParameter("Decimation rate", decimation);

    if (m_IsSmoothed)
    {
      surfaceFilter->SetParameter("Apply median", true);
      surfaceFilter->SetParameter("Smooth", true);
      surfaceFilter->SetParameter("Gaussian SD", sqrt(smoothing)); // use sqrt to account for setting of variance in preferences
      StatusBar::GetInstance()->DisplayText("Smoothed surface creation started in background...");
    }
    else
    {
      surfaceFilter->SetParameter("Apply median", false);
      surfaceFilter->SetParameter("Smooth", false);
      StatusBar::GetInstance()->DisplayText("Surface creation started in background...");
    }

    surfaceFilter->StartAlgorithm();
  }
  catch(...)
  {
    MITK_ERROR << "Surface creation failed!";
  }
}

void QmitkCreatePolygonModelAction::OnSurfaceCalculationDone()
{
  StatusBar::GetInstance()->Clear();
}

void QmitkCreatePolygonModelAction::SetDataStorage(DataStorage *dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkCreatePolygonModelAction::SetSmoothed(bool smoothed)
{
  m_IsSmoothed = smoothed;
}

void QmitkCreatePolygonModelAction::SetDecimated(bool decimated)
{
  m_IsDecimated = decimated;
}

void QmitkCreatePolygonModelAction::SetFunctionality(QtViewPart *)
{
}
