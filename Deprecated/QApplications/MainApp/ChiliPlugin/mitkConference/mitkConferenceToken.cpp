/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include <mitkConferenceToken.h>
#include <mitkConferenceKit.h>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <stdlib.h>

namespace mitk{

bool ConferenceToken::m_Token = true;
long int ConferenceToken::m_Unique = -1;
//int ConferenceToken::m_Ref = 0;
ConferenceToken* ConferenceToken::m_Instance = NULL;

long int
ConferenceToken::Random()
{
  time_t seconds;
  time(&seconds);
  srand((unsigned int) seconds);
  return (long int)rand();
}


//class ConferenceToken;
ConferenceToken::ConferenceToken()
{
  m_Unique = Random(); 
  m_Instance = this;
};
ConferenceToken::~ConferenceToken(){};

bool ConferenceToken::HaveToken()
{
  return m_Token;
}

void
ConferenceToken::GetToken()
{
  mitk::ConferenceKit* ck = mitk::ConferenceKit::GetInstance();
  ck->AskForToken( m_Unique );
}


void
ConferenceToken::GetToken(long int requester)
{
  //std::cout<<"ConferenceToken::GetToken("<<m_Unique<<") => "<<requester<<std::endl;
  if(HaveToken())
  {
  //  std::cout<<"ConferenceToken I am GIVING IT AWAY!"<<std::endl;
    m_Token = false;
    mitk::ConferenceKit* ck = mitk::ConferenceKit::GetInstance();
    ck->SetToken( m_Unique, requester );
  }
}

void
ConferenceToken::SetToken(int long tokenReceiver)
{
  if( tokenReceiver == m_Unique || tokenReceiver == 0)
  {
    //std::cout<<"YUUPPIEEE NOW HAVE CONFERENCE TOKEN"<<std::endl;
    m_Token = true;
  }
}

void
ConferenceToken::ArrangeToken()
{
  mitk::ConferenceKit* ck = mitk::ConferenceKit::GetInstance();
  ck->MyTokenPriority( m_Unique );
}

void
ConferenceToken::ArrangeToken(long int partnerToken)
{
  if(partnerToken == m_Unique )
  {
    m_Unique = Random();
    ArrangeToken();
  }
  else
  {
    //lower id will get the token.
    m_Token= (partnerToken < m_Unique )?false:true;

    if (m_Token)
      std::cout<<"mitkConferencToken.cpp: Get Token: "<<m_Unique<<std::endl;
    else
      std::cout<<"mitkConferencToken.cpp: Loose token "<<m_Unique<<std::endl;
  }
}
// state ConferenceToken::requestToken()
// {
//   //Networkrequest here
//   m_Token=true;
//   return succes;
// }
ConferenceToken*
ConferenceToken::GetInstance()
{
  if ( !m_Instance )
  {
    m_Instance = new ConferenceToken();
    //std::cout<<"NEUE TOKEN INSTANCE"<<std::endl;
  }

  return m_Instance;
}

}
