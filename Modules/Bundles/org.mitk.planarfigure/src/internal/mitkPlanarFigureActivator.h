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

#include <berryPlugin.h>

#include <mitkPlanarFigureDll.h>

//#include <QSlider>
//#include <QObject>

namespace mitk
{

/**
 * \ingroup org_mitk_planarfigure_internal
 *
 * \brief The plug-in activator for the planar figure module
 *
 * When the plug-in is started by the framework, it initialzes planar figure specific things.
 */
class ORG_MITK_PLANARFIGURE_EXPORT PlanarFigureActivator : 
  /*public QObject, */public berry::Plugin
{

  //Q_OBJECT

public:

  /**
   * Registers sandbox core object factories.
   */
  void Start(berry::IBundleContext::Pointer context);


private:

};

}
#endif /* MITKPLANARFIGUREACTIVATOR_H_  */
