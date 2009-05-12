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

#ifndef MITK_CONFERENC_TOKEN
#define MITK_CONFERENC_TOKEN

namespace mitk
{

  class ConferenceToken 
  {
    public:
      ConferenceToken();
      ~ConferenceToken();

      /* Token at hand.
       * returns the token state.
       */
      bool HaveToken();

      /* Instance call of ConferenceToken.
       * Every Application holds only one instance of the token.
       */
      static ConferenceToken* GetInstance();

      /* Request for token used by Application.
       *   will call the conference factory call to hand on the request to all participants
       */
      void GetToken();


      /* Request for token used by conference factory to pass the token request.
       *  if availible the token will be removed by the token holder and sended to the requesting conference partner. 
       *  if the token is not availible nothing take place
       */
      void GetToken(int long tokenRequester);


      /* Give token to the requester .
       * if the id is equal to the application own the conference apllication will get the token.
       */
      void SetToken(int long tokenReceiver);


      /* initial call to create exactly one global token for the conference.
       * th function will call the conference factory to pass the own random created application id
       */   
      void ArrangeToken();


      /* factory call from all other participants.
       * the token goes to the participant with the lowest id.
       */
      void ArrangeToken(long int);


    private:
      //Use only one global token
      static bool m_Token;
      // static int m_Ref;
      static long int m_Unique;
      static ConferenceToken* m_Instance;


      /* Create Random Integer.
       *
       */
      static long int Random();

  };

}
#endif
