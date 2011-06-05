/*=========================================================================

 Program:   MITK Platform
 Language:  C++
 Date:      $Date: 2009-05-19 15:45:39 +0200 (Di, 19 Mai 2009) $
 Version:   $Revision: 17020 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


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
