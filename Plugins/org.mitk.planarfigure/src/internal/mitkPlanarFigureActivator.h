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


#ifndef MITKPLANARFIGUREACTIVATOR_H_
#define MITKPLANARFIGUREACTIVATOR_H_

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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "org_mitk_planarfigure")
#endif
  Q_INTERFACES(ctkPluginActivator)

public:

  /**
   * Registers sandbox core object factories.
   */
  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

};

}
#endif /* MITKPLANARFIGUREACTIVATOR_H_  */
