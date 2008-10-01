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


#ifndef CHERRYMESSAGEDIALOG_H_
#define CHERRYMESSAGEDIALOG_H_

#include "cherryIDialog.h"

#include "../cherryShell.h"

#include <string>
#include <vector>

namespace cherry {

struct MessageDialog
{

 /**
  * Convenience method to open a simple confirm (OK/Cancel) dialog.
  *
  * @param parent
  *            the parent shell of the dialog, or <code>null</code> if none
  * @param title
  *            the dialog's title, or <code>null</code> if none
  * @param message
  *            the message
  * @return <code>true</code> if the user presses the OK button,
  *         <code>false</code> otherwise
  */
 static bool OpenConfirm(Shell::Pointer parent, const std::string& title, const std::string& message);

 /**
  * Convenience method to open a standard error dialog.
  *
  * @param parent
  *            the parent shell of the dialog, or <code>null</code> if none
  * @param title
  *            the dialog's title, or <code>null</code> if none
  * @param message
  *            the message
  */
 static void OpenError(Shell::Pointer parent, const std::string& title, const std::string& message);

 /**
  * Convenience method to open a standard information dialog.
  *
  * @param parent
  *            the parent shell of the dialog, or <code>null</code> if none
  * @param title
  *            the dialog's title, or <code>null</code> if none
  * @param message
  *            the message
  */
 static void OpenInformation(Shell::Pointer parent, const std::string& title,
         const std::string& message);

 /**
  * Convenience method to open a simple Yes/No question dialog.
  *
  * @param parent
  *            the parent shell of the dialog, or <code>null</code> if none
  * @param title
  *            the dialog's title, or <code>null</code> if none
  * @param message
  *            the message
  * @return <code>true</code> if the user presses the OK button,
  *         <code>false</code> otherwise
  */
 static bool OpenQuestion(Shell::Pointer parent, const std::string& title,
         const std::string& message);

 /**
  * Convenience method to open a standard warning dialog.
  *
  * @param parent
  *            the parent shell of the dialog, or <code>null</code> if none
  * @param title
  *            the dialog's title, or <code>null</code> if none
  * @param message
  *            the message
  */
 static void OpenWarning(Shell::Pointer parent, const std::string& title, const std::string& message);

 /**
  * Create a message dialog. Note that the dialog will have no visual
  * representation (no widgets) until it is told to open.
  * <p>
  * The labels of the buttons to appear in the button bar are supplied in
  * this constructor as an array. The <code>open</code> method will return
  * the index of the label in this array corresponding to the button that was
  * pressed to close the dialog. If the dialog was dismissed without pressing
  * a button (ESC, etc.) then -1 is returned. Note that the <code>open</code>
  * method blocks.
  * </p>
  *
  * @param parentShell
  *            the parent shell
  * @param dialogTitle
  *            the dialog title, or <code>null</code> if none
  * @param dialogTitleImage
  *            the dialog title image, or <code>null</code> if none
  * @param dialogMessage
  *            the dialog message
  * @param dialogImageType
  *            one of the following values:
  *            <ul>
  *            <li><code>IDialog::NONE</code> for a dialog with no
  *            image</li>
  *            <li><code>IDialog::ERROR</code> for a dialog with an
  *            error image</li>
  *            <li><code>IDialog::INFORMATION</code> for a dialog
  *            with an information image</li>
  *            <li><code>IDialog::QUESTION </code> for a dialog with a
  *            question image</li>
  *            <li><code>IDialog::WARNING</code> for a dialog with a
  *            warning image</li>
  *            </ul>
  * @param dialogButtonLabels
  *            an array of labels for the buttons in the button bar
  * @param defaultIndex
  *            the index in the button label array of the default button
  */
 static IDialog::Pointer CreateMessageDialog(Shell::Pointer parentShell, const std::string& dialogTitle,
         void* dialogTitleImage, const std::string& dialogMessage, int dialogImageType,
         const std::vector<std::string>& dialogButtonLabels, int defaultIndex);

};

}


#endif /* CHERRYMESSAGEDIALOG_H_ */
