/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYMESSAGEDIALOGTWEAKLET_H_
#define BERRYMESSAGEDIALOGTWEAKLET_H_

#include <org_blueberry_ui_Export.h>
#include "berryShell.h"

#include "dialogs/berryIDialog.h"
#include "internal/berryTweaklets.h"

#include <berryMacros.h>

#include <vector>
#include <string>

namespace berry {

/**
 * A dialog for showing messages to the user.
 */
struct BERRY_UI MessageDialogTweaklet
{

  static Tweaklets::TweakKey<MessageDialogTweaklet> KEY;

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
    virtual bool OpenConfirm(Shell::Pointer parent, const std::string& title, const std::string& message) = 0;

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
    virtual void OpenError(Shell::Pointer parent, const std::string& title, const std::string& message) = 0;

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
    virtual void OpenInformation(Shell::Pointer parent, const std::string& title,
            const std::string& message) = 0;

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
    virtual bool OpenQuestion(Shell::Pointer parent, const std::string& title,
            const std::string& message) = 0;

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
    virtual void OpenWarning(Shell::Pointer parent, const std::string& title, const std::string& message) = 0;

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
     *            <li><code>IDialog::ERR</code> for a dialog with an
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
    virtual IDialog::Pointer MessageDialog(Shell::Pointer parentShell, const std::string& dialogTitle,
            void* dialogTitleImage, const std::string& dialogMessage, int dialogImageType,
            const std::vector<std::string>& dialogButtonLabels, int defaultIndex) = 0;

};

}

Q_DECLARE_INTERFACE(berry::MessageDialogTweaklet, "org.blueberry.MessageDialogTweaklet")

#endif /* BERRYMESSAGEDIALOGTWEAKLET_H_ */
