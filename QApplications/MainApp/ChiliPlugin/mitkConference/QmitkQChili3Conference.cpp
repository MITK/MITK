#include "QmitkQChili3Conference.h"
#include <iostream>
#include <chili/plugin.h>
//#include <chili/task.h>
//#include <chili/plugin.xpm>
// #include <ipMsg/ipMsg.h>
// #include <ipMsg/ipMsgTypes.h>
#include <mitkChiliPlugin.h>


const ipInt4_t qmitk_chili_plugin_type = 5000;

//mitk::ConferenceKit::Pointer QChili3Conference::m_Instance;

// mitk::ConferenceKit::Pointer
// QChili3Conference::GetInstance()
// {
//   if( !QChili3Conference::m_Instance )
//   {
//      ConferenceKit::Pointer qcc = QChili3Conference::New();
//      QChili3Conference::m_Instance = qcc;
//   }
//   return QChili3Conference::m_Instance;
// };


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

  list = ipMsgVarToList( list,
                         ipTypeInt4, &arg0,
                         ipTypeInt4, &arg1,
                         NULL );


  qp->sendMessage( qmitk_chili_plugin_type, list );

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
