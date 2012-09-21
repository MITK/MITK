///*===================================================================
//
//The Medical Imaging Interaction Toolkit (MITK)
//
//Copyright (c) German Cancer Research Center,
//Division of Medical and Biological Informatics.
//All rights reserved.
//
//This software is distributed WITHOUT ANY WARRANTY; without
//even the implied warranty of MERCHANTABILITY or FITNESS FOR
//A PARTICULAR PURPOSE.
//
//See LICENSE.txt or http://www.mitk.org for details.
//
//===================================================================*/

#ifndef EXTENSIONPOINTCONTRIBUTION_H_
#define EXTENSIONPOINTCONTRIBUTION_H_

// qt Includes
#include "IChangeText.h"


class ExtensionPointContribution : public IChangeText
{
  
public:
  
  ExtensionPointContribution();
  ~ExtensionPointContribution();

  void ChangeExtensionLabelText(QString s);

};

#endif /*EXTENSIONPOINTCONTRIBUTION_H_*/
