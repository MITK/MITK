
#include "QmitkQChili3Conference.h"
#include <iostream>
#include <chili/plugin.h>
#include <mitkPACSPlugin.h>


QChili3Conference::QChili3Conference(){};
QChili3Conference::~QChili3Conference(){};

void QChili3Conference::SendQt(const char* s)
{
  std::cout<<"QChili3Conference__"<<s<<std::endl;

  QcPlugin* qp;
  qp = mitk::PACSPlugin::GetInstance()->GetPluginInstance();

  ipMsgParaList_t *list = NULL;
  ipInt4_t arg0 = 999999;
  ipInt4_t arg1 = 654321;

  list = ipMsgVarToList( list,
                         ipTypeInt4, &arg0,
                         ipTypeString, s,
                         NULL );


  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID, list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );

};

void QChili3Conference::SendMITK(signed int eventID, const char* sender, float w1, float w2, float w3, float p1, float p2)
{
  QcPlugin* qp;
  qp = mitk::PACSPlugin::GetInstance()->GetPluginInstance();

  ipMsgParaList_t *list = NULL;

  list = ipMsgVarToList( list,
                         ipTypeInt4, &eventID,
                         ipTypeString, sender,
                         ipTypeFloat4, &w1,
                         ipTypeFloat4, &w2,
                         ipTypeFloat4, &w3,
                         ipTypeFloat4, &p1,
                         ipTypeFloat4, &p2,
                         NULL );

  qp->sendMessage( mitk::m_QmitkChiliPluginConferenceID + 1, list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
};


