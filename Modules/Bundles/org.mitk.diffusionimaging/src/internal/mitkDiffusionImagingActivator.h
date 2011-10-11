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


#ifndef MITKDIFFUSIONIMAGINGACTIVATOR_H_ 
#define MITKDIFFUSIONIMAGINGACTIVATOR_H_ 

#include <ctkPluginActivator.h>
#include <mitkExportMacros.h>

namespace mitk
{

/**
 * \ingroup org_mitk_diffusionimaging_internal
 *
 * \brief The plug-in activator for the diffusion imaging module
 *
 * When the plug-in is started by the framework, it initialzes diffusion imaging
 * specific things.
 */
class MITK_LOCAL DiffusionImagingActivator : 
  public QObject, public ctkPluginActivator
{

  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:

  /**
   * Registers diffusion imaging object factories.
   */
  void start(ctkPluginContext* context);
  
  void stop(ctkPluginContext* context);

};

}
#endif /* MITKDIFFUSIONIMAGINGACTIVATOR_H_ */
