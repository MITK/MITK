/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD
#define MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD


#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>

#include "mitkChiliPluginEvents.h"

class QcPlugin;
class QcMITKSamplePlugin;

namespace mitk {

//TODO wird aktuell so genutzt mitk::m_QmitkChiliPluginConferenceID (QmitkChili3ConferenceKit), nicht über die GetConferenceID-Funktion
const int m_QmitkChiliPluginConferenceID = 5001;

typedef enum
{
  MITKc = 0,
  QTc,
  LAUNCHc,
  ARRANGEc,
  TOKENREQUESTc,
  TOKENSETc,
  TEXTc,
  MOUSEMOVEc ,
} ConfMsgType;

class ChiliPlugin : public itk::Object
{
public:

  virtual bool IsPlugin();
  virtual int GetConferenceID();
  virtual QcPlugin* GetPluginInstance();
  static mitk::ChiliPlugin* GetInstance();

  mitkClassMacro(ChiliPlugin,itk::Object);
  itkNewMacro(ChiliPlugin);
  virtual ~ChiliPlugin();

  virtual std::string GetCurrentStudyID();

protected:

  friend class QcMITKSamplePlugin;

  virtual void SetPluginInstance(QcPlugin* instance);

  virtual void SetCurrentStudyID( const std::string& );

  ChiliPlugin() {}

  std::string m_CurrentStudyID;

};

} // namespace mitk

#endif /* MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD */
