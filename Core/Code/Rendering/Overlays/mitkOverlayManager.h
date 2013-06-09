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
#include "mitkLocalStorageHandler.h"

// Microservices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

namespace mitk {

class MITK_CORE_EXPORT OverlayManager : public itk::LightObject {
public:

  mitkClassMacro(OverlayManager, itk::LightObject);
  itkNewMacro(OverlayManager);

  void AddOverlay(Overlay::Pointer overlay);
  void UpdateOverlays(BaseRenderer *baseRenderer);
  void UnregisterMicroservice();
  static const std::string PROP_ID;
  static OverlayManager::Pointer GetServiceInstance(int ID = 0);

  /** \brief Base class for mapper specific rendering ressources.
   */
  class MITK_CORE_EXPORT LocalStorage
  {
  public:

    /** \brief Timestamp of last update of stored data. */
    itk::TimeStamp m_LastUpdateTime;

    /** \brief Default constructor of the local storage. */
    LocalStorage();
    /** \brief Default deconstructor of the local storage. */
    ~LocalStorage();

//    bool IsGenerateDataRequired(mitk::BaseRenderer *renderer,mitk::OverlayManager *overlaymanager);

    inline void UpdateGenerateDataTime()
    {
      m_LastGenerateDataTime.Modified();
    }

    inline itk::TimeStamp & GetLastGenerateDataTime() { return m_LastGenerateDataTime; }

  protected:

    /** \brief timestamp of last update of stored data */
    itk::TimeStamp m_LastGenerateDataTime;

  };

  /** \brief The LocalStorageHandler holds all (three) LocalStorages for the three 2D render windows. */
  mitk::LocalStorageHandler<LocalStorage> m_LSH;

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit OverlayManager();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~OverlayManager();

private:

  std::vector<Overlay::Pointer> m_OverlayList;

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


