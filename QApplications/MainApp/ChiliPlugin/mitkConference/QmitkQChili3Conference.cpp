#include "QmitkQChili3Conference.h"
#include <iostream>
#include <chili/plugin.h>
//#include <chili/task.h>
//#include <chili/plugin.xpm>
// #include <ipMsg/ipMsg.h>
// #include <ipMsg/ipMsgTypes.h>
#include <mitkChiliPlugin.h>


const ipInt4_t qmitk_chili_plugin_type = 5001;

QChili3Conference::QChili3Conference(){};
QChili3Conference::~QChili3Conference(){};

void QChili3Conference::SendQt(const char* s)
{
  std::cout<<"QChili3Conference__"<<s<<std::endl;

  QcPlugin* qp;
  qp = mitk::ChiliPlugin::GetPluginInstance();

  ipMsgParaList_t *list = NULL;
  ipInt4_t arg0 = 999999;
  ipInt4_t arg1 = 654321;
  //char arg2[5] = {'m','I','t','K','\0'};
  char arg2[] = "mItK";

  list = ipMsgVarToList( list,
                         ipTypeInt4, &arg0,
                         ipTypeStringPtr, &arg2,
                         NULL );


  qp->sendMessage( qmitk_chili_plugin_type, list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );

};

void QChili3Conference::SendMITK(int eventID, short int p0, short int p1, short int p2)
{
  QcPlugin* qp;
  qp = mitk::ChiliPlugin::GetPluginInstance();

  ipMsgParaList_t *list = NULL;
  ipInt4_t eid = eventID;
  ipInt4_t x = p0;
  ipInt4_t y = p1;
  ipInt4_t z = p2;

  list = ipMsgVarToList( list,
                         ipTypeInt4, &eid,
                         ipTypeInt4, &x,
                         ipTypeInt4, &y,
                         ipTypeInt4, &z,
                         NULL );


  qp->sendMessage( qmitk_chili_plugin_type + 1, list );

  if( list )
    ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );

};


// void
// MessageLogger::sendPressed()
// {
//   ipMsgParaList_t *list = NULL;
//   ipInt4_t arg0 = 1234;
//   ipInt4_t arg1 = 7654321;
//
//   list = ipMsgVarToList( list,
//                          ipTypeInt4, &arg0,
//                          ipTypeInt4, &arg1,
//                          NULL );
//
//   sendMessage( custom_type, list );
//
//   if( list )
//     ipMsgRemoveList( list, _MSG_REMOVE_LIST_ONLY );
// }
