#include <QmitkChili3ConferenceKit.h>
#include <chili/plugin.h>
#include <mitkChiliPluginImpl.h>

Chili3Conference::Chili3Conference(){};
Chili3Conference::~Chili3Conference(){};

void
Chili3Conference::SendQt(const char* s)
{
  mitk::PACSPlugin::Pointer pluginInstance = mitk::PACSPlugin::GetInstance();
  mitk::ChiliPlugin::Pointer realPluginInstance = dynamic_cast<mitk::ChiliPlugin*>( pluginInstance.GetPointer() );

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeString, s,
                         NULL );

  realPluginInstance->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::QTc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
};


void
Chili3Conference::SendMITK(signed int eventID, const char* sender, int etype, int estate, int ebuttonstate, int ekey, float w1, float w2, float w3, float p1, float p2)
{
  mitk::PACSPlugin::Pointer pluginInstance = mitk::PACSPlugin::GetInstance();
  mitk::ChiliPlugin::Pointer realPluginInstance = dynamic_cast<mitk::ChiliPlugin*>( pluginInstance.GetPointer() );

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeInt4, &eventID,
                         ipTypeString, sender,
                         ipTypeInt4, &etype,
                         ipTypeInt4, &estate,
                         ipTypeInt4, &ebuttonstate,
                         ipTypeInt4, &ekey,
                         ipTypeFloat4, &w1,
                         ipTypeFloat4, &w2,
                         ipTypeFloat4, &w3,
                         ipTypeFloat4, &p1,
                         ipTypeFloat4, &p2,
                         NULL );

  realPluginInstance->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::QTc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
};


void
Chili3Conference::MouseMove( const char* sender, float w1, float w2, float w3 )
{
  mitk::PACSPlugin::Pointer pluginInstance = mitk::PACSPlugin::GetInstance();
  mitk::ChiliPlugin::Pointer realPluginInstance = dynamic_cast<mitk::ChiliPlugin*>( pluginInstance.GetPointer() );

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeString, sender,
                         ipTypeFloat4, &w1,
                         ipTypeFloat4, &w2,
                         ipTypeFloat4, &w3,
                         NULL );

  realPluginInstance->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::QTc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
};


void
Chili3Conference::MyTokenPriority(long int tid)
{
  mitk::PACSPlugin::Pointer pluginInstance = mitk::PACSPlugin::GetInstance();
  mitk::ChiliPlugin::Pointer realPluginInstance = dynamic_cast<mitk::ChiliPlugin*>( pluginInstance.GetPointer() );

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeInt4, &tid,
                         NULL );

  realPluginInstance->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::QTc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );

}


void
Chili3Conference::AskForToken(long int tid)
{
  mitk::PACSPlugin::Pointer pluginInstance = mitk::PACSPlugin::GetInstance();
  mitk::ChiliPlugin::Pointer realPluginInstance = dynamic_cast<mitk::ChiliPlugin*>( pluginInstance.GetPointer() );

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeInt4, &tid,
                         NULL );

  realPluginInstance->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::QTc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
}


void
Chili3Conference::SetToken(long int sender, long int requester)
{
  mitk::PACSPlugin::Pointer pluginInstance = mitk::PACSPlugin::GetInstance();
  mitk::ChiliPlugin::Pointer realPluginInstance = dynamic_cast<mitk::ChiliPlugin*>( pluginInstance.GetPointer() );

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeInt4, &sender,
                         ipTypeInt4, &requester,
                         NULL );

  realPluginInstance->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::QTc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
}

