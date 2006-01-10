#include "mitkConferenceToken.h"
#include <iostream>

namespace mitk{

bool ConferenceToken::m_Token = false;
int ConferenceToken::m_Ref = 0;

//class ConferenceToken;
ConferenceToken::ConferenceToken() /*:m_Token(false),m_Ref(0)*/{};
ConferenceToken::~ConferenceToken(){};

bool ConferenceToken::HaveToken()
{
  m_Ref++;
   if(!(m_Ref%100))std::cout<<"TokenReferenz:"<<m_Ref<<std::endl;
  return m_Token;
}

bool ConferenceToken::GetToken()
{
  //Networkrequest here
  if(false)//connection
  {
 /*   state s;
    s = requestToken();
 */   std::cout<<"Connection Token: "<<m_Token<<std::endl;
  }
  else
  {
    m_Token=true;
  }

  return m_Token;
}

// state ConferenceToken::requestToken()
// {
//   //Networkrequest here
//   m_Token=true;
//   return succes;
// }


}
