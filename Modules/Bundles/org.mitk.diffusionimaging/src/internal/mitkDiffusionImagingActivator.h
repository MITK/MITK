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

#include <cherryPlugin.h>

#include <mitkDiffusionImagingDll.h>

//#include <QSlider>
//#include <QObject>

namespace mitk
{

/**
 * \ingroup org_mitk_diffusionimaging_internal
 *
 * \brief The plug-in activator for the diffusion imaging module
 *
 * When the plug-in is started by the framework, it initialzes sandbox
 * specific things.
 */
class ORG_MITK_DIFFUSIONIMAGING_EXPORT DiffusionImagingActivator : 
  /*public QObject, */public cherry::Plugin
{

  //Q_OBJECT

public:

  /**
   * Registers sandbox core object factories.
   */
  void Start(cherry::IBundleContext::Pointer context);

  //void OpactiyChanged(int value);
  //void OpactiyActionChanged();

  //void CreateQtPartControl(QWidget *){}

private:

  //QSlider* m_OpacitySlider;

};

}
#endif /* MITKDIFFUSIONIMAGINGACTIVATOR_H_ */
