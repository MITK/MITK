/*=========================================================================
 
 Program:   openCherry Platform
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

#ifndef CHERRYQTMESSAGEDIALOGTWEAKLET_H_
#define CHERRYQTMESSAGEDIALOGTWEAKLET_H_

#include <cherryMessageDialogTweaklet.h>

namespace cherry
{

class QtMessageDialogTweaklet: public MessageDialogTweaklet
{

  bool OpenConfirm(Shell::Pointer parent, const std::string& title,
      const std::string& message);
  void OpenError(Shell::Pointer parent, const std::string& title,
      const std::string& message);
  void OpenInformation(Shell::Pointer parent, const std::string& title,
      const std::string& message);
  bool OpenQuestion(Shell::Pointer parent, const std::string& title,
      const std::string& message);
  void OpenWarning(Shell::Pointer parent, const std::string& title,
      const std::string& message);

  IDialog::Pointer MessageDialog(Shell::Pointer parentShell,
      const std::string& dialogTitle, void* dialogTitleImage,
      const std::string& dialogMessage, int dialogImageType, const std::vector<
          std::string>& dialogButtonLabels, int defaultIndex);

};

}

#endif /* CHERRYQTMESSAGEDIALOGTWEAKLET_H_ */
