/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkPlanarFigureActivator_h
#define mitkPlanarFigureActivator_h

#include <ctkPluginActivator.h>


namespace mitk
{

/**
 * \ingroup org_mitk_planarfigure_internal
 *
 * \brief The plug-in activator for the planar figure module
 *
 * When the plug-in is started by the framework, it initialzes planar figure specific things.
 */
class PlanarFigureActivator :
  public QObject, public ctkPluginActivator
{

  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_mitk_planarfigure")
  Q_INTERFACES(ctkPluginActivator)

public:

  /**
   * Registers sandbox core object factories.
   */
  void start(ctkPluginContext* context) override;
  void stop(ctkPluginContext* context) override;

};

}
#endif
