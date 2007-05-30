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

#include <mitkChiliPlugin.h>

class QcPlugin;

namespace mitk {

  /**Documentation
  @brief interface between Chili and MITK
  @ingroup Process
  @ingroup Chili
  */

class ChiliPluginImpl : public ChiliPlugin
{
  public:

    /** return the StudyInformation of the current selected study */
    virtual StudyInformation GetCurrentStudy();
    /** return the list of the series to the current selected study */
    virtual SeriesList GetCurrentSeries();
    /** return the number of Lightboxes in chili */
    virtual unsigned int GetLightBoxCount();

    /** return if the application run as standalone or as chiliplugin*/
    virtual bool IsPlugin();
    /** return the conferenceid */
    virtual int GetConferenceID();

    /** Set the properties from the list to the datatreenode. The description of the properties get the prefix "Chili: ". */
    virtual void SetPropertyToNode( const mitk::PropertyList::Pointer property, mitk::DataTreeNode* );

    /** return the PluginInstance */
    virtual QcPlugin* GetPluginInstance();

    mitkClassMacro(ChiliPluginImpl,ChiliPlugin);
    itkNewMacro(ChiliPluginImpl);
    virtual ~ChiliPluginImpl();

  protected:

    /** someone have to set the study- and seriesinformation and the instance */
    friend class QcMITKSamplePlugin;

    virtual void SetPluginInstance(QcPlugin* instance);

    /** Invoke an event if a study selected and the information about the study and series changed */
    void SendStudySelectedEvent();
    void SendLightBoxCountChangedEvent();
    //void SendSeriesSelectedEvent();
    //..

    ChiliPluginImpl::ChiliPluginImpl();

    StudyInformation m_CurrentStudy;
    SeriesList m_CurrentSeries;
    unsigned int m_LightBoxCount;

  private:

    /** make the plugin a singleton */
    static QcPlugin* s_PluginInstance;

};

} // namespace mitk

#endif /* MITKCHILIPLUGIN_H_HEADER_INCLUDED_C1EBD0AD */
