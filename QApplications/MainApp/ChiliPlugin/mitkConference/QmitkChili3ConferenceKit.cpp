#include <QmitkChili3ConferenceKit.h>
#include <chili/plugin.h>
#include <mitkChiliPlugin.h>


Chili3Conference::Chili3Conference(){};
Chili3Conference::~Chili3Conference(){};


void
Chili3Conference::SendQt(const char* s)
{
  QcPlugin* qp;
  qp = mitk::ChiliPlugin::GetInstance()->GetPluginInstance();

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeString, s,
                         NULL );

  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::QTc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
};


void
Chili3Conference::SendMITK(signed int eventID, const char* sender, int etype, int estate, int ebuttonstate, int ekey, float w1, float w2, float w3, float p1, float p2)
{
  QcPlugin* qp;
  qp = mitk::ChiliPlugin::GetInstance()->GetPluginInstance();

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

  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::MITKc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
};


void
Chili3Conference::MouseMove( const char* sender, float w1, float w2, float w3 )
{ 
  QcPlugin* qp;
  qp = mitk::ChiliPlugin::GetInstance()->GetPluginInstance();

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeString, sender,
                         ipTypeFloat4, &w1,
                         ipTypeFloat4, &w2,
                         ipTypeFloat4, &w3,
                         NULL );

  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::MOUSEMOVEc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
};


void
Chili3Conference::MyTokenPriority(long int tid)
{
  QcPlugin* qp;
  qp = mitk::ChiliPlugin::GetInstance()->GetPluginInstance();

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeInt4, &tid,
                         NULL );

  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::ARRANGEc, list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );

}


void
Chili3Conference::AskForToken(long int tid)
{
  QcPlugin* qp;
  qp = mitk::ChiliPlugin::GetInstance()->GetPluginInstance();

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeInt4, &tid,
                         NULL );

  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::TOKENREQUESTc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
}


void
Chili3Conference::SetToken(long int sender, long int requester)
{
  QcPlugin* qp;
  qp = mitk::ChiliPlugin::GetInstance()->GetPluginInstance();

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeInt4, &sender,
                         ipTypeInt4, &requester,
                         NULL );

  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID + mitk::TOKENSETc , list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
}

