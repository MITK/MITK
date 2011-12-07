#include "mitkClippingPlaneDeformationTool.h"

#include "mitkClippingPlaneDeformationTool.xpm"

#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkGlobalInteraction.h"

namespace mitk{
  MITK_TOOL_MACRO(ClippingTools_EXPORT, ClippingPlaneDeformationTool, "Clipping Plane Deformation");
}

mitk::ClippingPlaneDeformationTool::ClippingPlaneDeformationTool()
: Tool( "global" )
{
}

mitk::ClippingPlaneDeformationTool::~ClippingPlaneDeformationTool()
{
}

const char** mitk::ClippingPlaneDeformationTool::GetXPM() const
{
  return mitkClippingPlaneDeformationTool_xpm;
}

const char* mitk::ClippingPlaneDeformationTool::GetName() const
{
  return "Deformation";
}

const char* mitk::ClippingPlaneDeformationTool::GetGroup() const
{
  return "ClippingPlane";
}

void mitk::ClippingPlaneDeformationTool::Activated()
{
  Superclass::Activated();

  //check if the Clipping plane is changed.
  if(m_ClippingPlaneNode != m_ToolManager->GetWorkingData( 0 ))
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_SurfaceInteractor );
    this->ClippingPlaneChanged();
  }

  mitk::GlobalInteraction::GetInstance()->AddInteractor( m_SurfaceInteractor );
}

void mitk::ClippingPlaneDeformationTool::Deactivated()
{
  Superclass::Deactivated();

  mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_SurfaceInteractor );
}

//Checks the working data node, if it has an interactor. Otherwise initial a new one.
void mitk::ClippingPlaneDeformationTool::ClippingPlaneChanged()
{
  m_ClippingPlaneNode = m_ToolManager->GetWorkingData(0);
  m_SurfaceInteractor = dynamic_cast<mitk::SurfaceDeformationInteractor3D*>(m_ClippingPlaneNode->GetInteractor());

  if (m_SurfaceInteractor.IsNull())
  {
    m_SurfaceInteractor = mitk::SurfaceDeformationInteractor3D::New( 
      "AffineInteractor3D", m_ClippingPlaneNode );
  }
}



