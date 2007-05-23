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


#ifndef MITKCHILIPLUGINIMPL_H_HEADER_INCLUDED
#define MITKCHILIPLUGINIMPL_H_HEADER_INCLUDED

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <mitkChiliPlugin.h>

class QcPlugin;

namespace mitk {

class ChiliPluginImpl : public ChiliPlugin

{
public:

  virtual StudyInformation GetCurrentStudy();
  virtual SeriesList GetCurrentSeries();

  virtual bool IsPlugin();
  virtual int GetConferenceID();

  virtual QcPlugin* GetPluginInstance();

  mitkClassMacro(ChiliPluginImpl,ChiliPlugin);
  itkNewMacro(ChiliPluginImpl);
  virtual ~ChiliPluginImpl();

protected:

  friend class QcMITKSamplePlugin;

  virtual void SetPluginInstance(QcPlugin* instance);

  void SendStudySelectedEvent();
  //void SendSeriesSelectedEvent();
  //..

  ChiliPluginImpl::ChiliPluginImpl();

  StudyInformation m_CurrentStudy;
  SeriesList m_CurrentSeries;

private:

  static QcPlugin* s_PluginInstance;

};

} // namespace mitk

#endif /* MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD */
