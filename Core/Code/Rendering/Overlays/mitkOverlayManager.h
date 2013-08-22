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
#include "mitkBaseLayouter.h"
#include "mitkLocalStorageHandler.h"

namespace mitk {

class BaseRenderer;

/** \brief The OverlayManager updates and manages Overlays and the respective Layouters. */
/** An Instance of the OverlayManager can be registered to several BaseRenderer instances in order to
 * call the update method of each Overlay during the rendering phase of the renderer.
*/
class MITK_CORE_EXPORT OverlayManager : public itk::LightObject {
public:
  typedef std::list<BaseRenderer*> BaseRendererList;
  typedef std::list<Overlay::Pointer> OverlayList;
  typedef std::map<const std::string,BaseLayouter::Pointer > LayouterMap;
  typedef std::map<const BaseRenderer*,LayouterMap > LayouterRendererMap;

  mitkClassMacro(OverlayManager, itk::LightObject);
  itkNewMacro(OverlayManager);

  void AddOverlay(Overlay* overlay);
  void RemoveOverlay(Overlay* overlay);

  /** \brief Clears the manager of all Overlays.*/
  void RemoveAllOverlays();

  /** \brief Adds the overlay to the layouter specified by identifier and renderer*/
  void SetLayouter(Overlay* overlay, const char* identifier, BaseRenderer* renderer);

  /** \brief Calls all layouters to update the position and size of the registered Overlays*/
  void UpdateLayouts(BaseRenderer* renderer);

  /** \brief Returns the Layouter specified by renderer and the identifier*/
  BaseLayouter::Pointer GetLayouter(BaseRenderer* renderer, const std::string identifier);

  /** \brief Add a layouter to provide it with the use of the SetLayouter method*/
  void AddLayouter(BaseLayouter* layouter);

  void AddBaseRenderer(BaseRenderer* renderer);

  /** \brief The layout of each Overlay will be prepared and the properties of each Overlay is updated.*/
  void UpdateOverlays(BaseRenderer *baseRenderer);

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit OverlayManager();

  ~OverlayManager();

private:

  OverlayList m_OverlayList;

  BaseRendererList m_BaseRendererList;

  LayouterRendererMap m_LayouterMap;

  /** \brief copy constructor */
  OverlayManager( const OverlayManager &);

  /** \brief assignment operator */
  OverlayManager &operator=(const OverlayManager &);

};

} // namespace mitk

#endif // OVERLAYMANAGER_H


