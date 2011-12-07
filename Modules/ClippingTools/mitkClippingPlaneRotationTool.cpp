#include "mitkClippingPlaneRotationTool.h"

#include "mitkClippingPlaneRotationTool.xpm"

#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkGlobalInteraction.h"

namespace mitk{
  MITK_TOOL_MACRO(ClippingTools_EXPORT, ClippingPlaneRotationTool, "Clipping Plane Rotation");
}

mitk::ClippingPlaneRotationTool::ClippingPlaneRotationTool()
: Tool( "global" )
{
}

mitk::ClippingPlaneRotationTool::~ClippingPlaneRotationTool()
{
}

const char** mitk::ClippingPlaneRotationTool::GetXPM() const
{
  return mitkClippingPlaneRotationTool_xpm;
}

const char* mitk::ClippingPlaneRotationTool::GetName() const
{
  return "Rotation";
}

const char* mitk::ClippingPlaneRotationTool::GetGroup() const
{
  return "ClippingPlane";
}

void mitk::ClippingPlaneRotationTool::Activated()
{
  Superclass::Activated();

  //check if the Clipping plane is changed.
  if(m_ClippingPlaneNode != m_ToolManager->GetWorkingData( 0 ))
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_AffineInteractor );
    this->ClippingPlaneChanged();
  }

  m_AffineInteractor->SetInteractionModeToRotation();
  mitk::GlobalInteraction::GetInstance()->AddInteractor( m_AffineInteractor );
}

void mitk::ClippingPlaneRotationTool::Deactivated()
{
  Superclass::Deactivated();

  mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_AffineInteractor );
}

//Checks the working data node, if it has an interactor. Otherwise initial a new one.
void mitk::ClippingPlaneRotationTool::ClippingPlaneChanged()
{
  m_ClippingPlaneNode = m_ToolManager->GetWorkingData(0);
  m_AffineInteractor = dynamic_cast<mitk::AffineInteractor3D*>(m_ClippingPlaneNode->GetInteractor());

  if (m_AffineInteractor.IsNull())
  {
    m_AffineInteractor = mitk::AffineInteractor3D::New( 
      "AffineInteractor3D", m_ClippingPlaneNode );
  }
}


