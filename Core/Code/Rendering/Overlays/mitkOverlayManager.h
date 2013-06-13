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

#ifndef OVERLAYMANAGER_H
#define OVERLAYMANAGER_H

#include "MitkExports.h"
#include <itkLightObject.h>
#include <vtkSmartPointer.h>
#include "mitkOverlay.h"
#include "mitkBaseRenderer.h"
#include "mitkBaseLayouter.h"
#include "mitkLocalStorageHandler.h"

// Microservices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

namespace mitk {

class MITK_CORE_EXPORT OverlayManager : public itk::LightObject {
public:
  typedef std::list<BaseRenderer*> BaseRendererList;
  typedef std::list<Overlay::Pointer> OverlayList;

  mitkClassMacro(OverlayManager, itk::LightObject);
  itkNewMacro(OverlayManager);

  void AddOverlay(Overlay::Pointer overlay);
  void RemoveOverlay(Overlay::Pointer overlay);
  void RemoveAllOverlays();
  void UnregisterMicroservice();
  static const std::string PROP_ID;
  static OverlayManager::Pointer GetServiceInstance(int ID = 0);

  void AddBaseRenderer(BaseRenderer* renderer);
  void UpdateOverlays(BaseRenderer *baseRenderer);

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit OverlayManager();

  ~OverlayManager();

private:

  void PrepareLayout(BaseRenderer* renderer);

  OverlayList m_OverlayList;

  BaseRendererList m_BaseRendererList;

  /** \brief copy constructor */
  OverlayManager( const OverlayManager &);

  /** \brief assignment operator */
  OverlayManager &operator=(const OverlayManager &);

  std::string RegisterMicroservice();

  mitk::ServiceRegistration m_Registration;

  std::string m_id;

};

} // namespace mitk

US_DECLARE_SERVICE_INTERFACE(mitk::OverlayManager, "org.mitk.services.OverlayManager")
#endif // OVERLAYMANAGER_H


