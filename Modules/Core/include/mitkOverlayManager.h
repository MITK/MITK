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

#include "MitkCoreExports.h"
#include <itkLightObject.h>
#include <vtkSmartPointer.h>
#include "mitkOverlay.h"
#include "mitkAbstractOverlayLayouter.h"
#include "mitkLocalStorageHandler.h"

namespace mitk {

class BaseRenderer;

/** \brief The OverlayManager updates and manages Overlays and the respective Layouters. */
/** An Instance of the OverlayManager can be registered to several BaseRenderer instances in order to
 * call the update method of each Overlay during the rendering phase of the renderer.
 * See \ref OverlaysPage for more info.
*/
class MITKCORE_EXPORT OverlayManager : public itk::LightObject {
public:
  typedef std::set<BaseRenderer*> BaseRendererSet;
  typedef std::set<Overlay::Pointer> OverlaySet;
  typedef std::map<const std::string,AbstractOverlayLayouter::Pointer > LayouterMap;
  typedef std::map<const BaseRenderer*,LayouterMap > LayouterRendererMap;
  typedef std::map<const BaseRenderer*,vtkSmartPointer<vtkRenderer> > ForegroundRendererMap;

  mitkClassMacroItkParent(OverlayManager, itk::LightObject);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void AddOverlay(const Overlay::Pointer& overlay, bool ForceInForeground = true);
  void AddOverlay(const Overlay::Pointer& overlay, BaseRenderer* renderer, bool ForceInForeground = true);
  void RemoveOverlay(const Overlay::Pointer& overlay);

  /** \brief Clears the manager of all Overlays.*/
  void RemoveAllOverlays();

  /** \brief Adds the overlay to the layouter specified by identifier and renderer*/
  void SetLayouter(Overlay* overlay, const std::string& identifier, BaseRenderer* renderer);

  /** \brief Calls all layouters to update the position and size of the registered Overlays*/
  void UpdateLayouts(BaseRenderer* renderer);

  /** \brief Returns the Layouter specified by renderer and the identifier*/
  AbstractOverlayLayouter::Pointer GetLayouter(BaseRenderer* renderer, const std::string& identifier);

  /** \brief Add a layouter to provide it with the use of the SetLayouter method*/
  void AddLayouter(const AbstractOverlayLayouter::Pointer& layouter);

  void AddBaseRenderer(BaseRenderer* renderer);

  /** \brief The layout of each Overlay will be prepared and the properties of each Overlay is updated.*/
  void UpdateOverlays(BaseRenderer *baseRenderer);

  void RemoveBaseRenderer(mitk::BaseRenderer *renderer);

  void RemoveAllBaseRenderers();

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit OverlayManager();

  ~OverlayManager();

private:

  OverlaySet m_OverlaySet;

  BaseRendererSet m_BaseRendererSet;

  LayouterRendererMap m_LayouterMap;

  ForegroundRendererMap m_ForegroundRenderer;

  /** \brief copy constructor */
  OverlayManager( const OverlayManager &);

  /** \brief assignment operator */
  OverlayManager &operator=(const OverlayManager &);

};

} // namespace mitk

#endif // OVERLAYMANAGER_H


