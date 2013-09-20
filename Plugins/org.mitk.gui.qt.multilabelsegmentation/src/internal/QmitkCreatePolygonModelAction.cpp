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
#include <QmitkStdMultiWidget.h>

#include <mitkIRenderWindowPart.h>
#include <mitkIRenderingManager.h>

// Blueberry
#include <berryIPreferencesService.h>
#include <berryPlatformUI.h>
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
    return;

  try
  {
    if (!m_IsSmoothed)
    {
      ShowSegmentationAsSurface::Pointer surfaceFilter = ShowSegmentationAsSurface::New();

      itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::Pointer successCommand = itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::New();
      successCommand->SetCallbackFunction(this, &QmitkCreatePolygonModelAction::OnSurfaceCalculationDone);
      surfaceFilter->AddObserver(ResultAvailable(), successCommand);

      itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::Pointer errorCommand = itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::New();
      errorCommand->SetCallbackFunction(this, &QmitkCreatePolygonModelAction::OnSurfaceCalculationDone);
      surfaceFilter->AddObserver(ProcessingError(), errorCommand);

      surfaceFilter->SetDataStorage(*m_DataStorage);
      surfaceFilter->SetPointerParameter("Input", image);
      surfaceFilter->SetPointerParameter("Group node", selectedNode);
      surfaceFilter->SetParameter("Show result", true);
      surfaceFilter->SetParameter("Sync visibility", false);
      surfaceFilter->SetParameter("Smooth", false);
      surfaceFilter->SetParameter("Apply median", false);
      surfaceFilter->SetParameter("Median kernel size", 3u);
      surfaceFilter->SetParameter("Gaussian SD", 1.5f);
      surfaceFilter->SetParameter("Decimate mesh", m_IsDecimated);
      surfaceFilter->SetParameter("Decimation rate", 0.8f);

      StatusBar::GetInstance()->DisplayText("Surface creation started in background...");

      surfaceFilter->StartAlgorithm();
    }
    else
    {
      ShowSegmentationAsSmoothedSurface::Pointer surfaceFilter = ShowSegmentationAsSmoothedSurface::New();

      itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::Pointer successCommand = itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::New();
      successCommand->SetCallbackFunction(this, &QmitkCreatePolygonModelAction::OnSurfaceCalculationDone);
      surfaceFilter->AddObserver(mitk::ResultAvailable(), successCommand);

      itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::Pointer errorCommand = itk::SimpleMemberCommand<QmitkCreatePolygonModelAction>::New();
      errorCommand->SetCallbackFunction(this, &QmitkCreatePolygonModelAction::OnSurfaceCalculationDone);
      surfaceFilter->AddObserver(mitk::ProcessingError(), errorCommand);

      surfaceFilter->SetDataStorage(*m_DataStorage);
      surfaceFilter->SetPointerParameter("Input", image);
      surfaceFilter->SetPointerParameter("Group node", selectedNode);

      berry::IWorkbenchPart::Pointer activePart =
          berry::PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow()->GetActivePage()->GetActivePart();
      mitk::IRenderWindowPart* renderPart = dynamic_cast<mitk::IRenderWindowPart*>(activePart.GetPointer());
      mitk::SliceNavigationController* timeNavController = 0;
      if (renderPart != 0)
      {
        timeNavController = renderPart->GetRenderingManager()->GetTimeNavigationController();
      }

      int timeNr = timeNavController != 0 ? timeNavController->GetTime()->GetPos() : 0;
      surfaceFilter->SetParameter("TimeNr", timeNr);

      IPreferencesService::Pointer prefService = Platform::GetServiceRegistry().GetServiceById<IPreferencesService>(IPreferencesService::ID);
      IPreferences::Pointer segPref = prefService->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

      bool smoothingHint = segPref->GetBool("smoothing hint", true);
      float smoothing = (float)segPref->GetDouble("smoothing value", 1.0);
      float decimation = (float)segPref->GetDouble("decimation rate", 0.5);
      float closing = (float)segPref->GetDouble("closing ratio", 0.0);

      if (smoothingHint)
      {
        smoothing = 0.0;
        Vector3D spacing = image->GetGeometry()->GetSpacing();

        for (Vector3D::Iterator iter = spacing.Begin(); iter != spacing.End(); ++iter)
          smoothing = max(smoothing, *iter);
      }

      surfaceFilter->SetParameter("Smoothing", smoothing);
      surfaceFilter->SetParameter("Decimation", decimation);
      surfaceFilter->SetParameter("Closing", closing);

      ProgressBar::GetInstance()->AddStepsToDo(8);
      StatusBar::GetInstance()->DisplayText("Smoothed surface creation started in background...");

      try {
        surfaceFilter->StartAlgorithm();
      } catch (...)
      {
        MITK_ERROR<<"Error creating smoothed polygon model: Not enough memory!";
      }
    }
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
