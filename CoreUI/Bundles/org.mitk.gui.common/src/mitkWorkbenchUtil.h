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


#ifndef MITKWORKBENCHUTIL_H
#define MITKWORKBENCHUTIL_H

#include <org_mitk_gui_common_Export.h>

#include "mitkDataStorageEditorInput.h"

#include <berryIWorkbenchPage.h>
#include <berryIEditorDescriptor.h>

namespace mitk {

/**
 * @ingroup org_mitk_gui_common
 *
 * @brief Utility class for loading data and opening editors in a MITK Workbench.
 *
 * @note Infering the content type is not yet supported (ignore the comments about it
 *       in the method documentation). 
 */
struct MITK_GUI_COMMON_PLUGIN WorkbenchUtil
{

  /**
   * Loads the set of given files into the active data storage of the given Workbench window.
   *
   * If the window already has an editor open on the active datastorage then that editor
   * is activated; otherwise the default editor for the "mitk" extension is activated.
   *
   * @param fileNames
   *            A list of file names with absolute path.
   * @param wnd
   *            The Workbench window in which the data will be loaded.
   *
   * @see mitk::IDataNodeReader
   */
  static void LoadFiles(const QStringList& fileNames, berry::IWorkbenchWindow::Pointer wnd);

  /**
     * Opens an editor on the given object.
     * <p>
     * If the page already has an editor open on the target object then that
     * editor is brought to front; otherwise, a new editor is opened. If
     * <code>activate == true</code> the editor will be activated.
     * <p>
     *
     * @param page
     *            the page in which the editor will be opened
     * @param input
     *            the editor input
     * @param editorId
     *            the id of the editor extension to use
     * @param activate
     *            if <code>true</code> the editor will be activated
     * @return an open editor or <code>null</code> if an external editor was
     *         opened
     * @exception PartInitException
     *                if the editor could not be initialized
     * @see IWorkbenchPage#OpenEditor(IEditorInput::Pointer, std::string, bool)
     */
  static berry::IEditorPart::Pointer OpenEditor(berry::IWorkbenchPage::Pointer page,
                                                berry::IEditorInput::Pointer input,
                                                const QString& editorId, bool activate = false);

  /**
     * Opens an editor on the given file resource. This method will attempt to
     * resolve the editor based on content-type bindings as well as traditional
     * name/extension bindings if <code>determineContentType</code> is
     * <code>true</code>.
     * <p>
     * If the page already has an editor open on the target object then that
     * editor is brought to front; otherwise, a new editor is opened. If
     * <code>activate == true</code> the editor will be activated.
     * <p>
     *
     * @param page
     *            the page in which the editor will be opened
     * @param input
     *            the editor input
     * @param activate
     *            if <code>true</code> the editor will be activated
     * @param determineContentType
     *            attempt to resolve the content type for this file
     * @return an open editor or <code>null</code> if an external editor was
     *         opened
     * @exception PartInitException
     *                if the editor could not be initialized
     * @see IWorkbenchPage#OpenEditor(IEditorInput::Pointer,std::string,bool)
     */
  static berry::IEditorPart::Pointer OpenEditor(berry::IWorkbenchPage::Pointer page,
                                                mitk::DataStorageEditorInput::Pointer input,
                                                bool activate = false,
                                                bool determineContentType = false);

  /**
     * Returns an editor descriptor appropriate for opening a file resource with
     * the given name.
     * <p>
     * The editor descriptor is determined using a multi-step process. This
     * method will attempt to infer the content type of the file if
     * <code>inferContentType</code> is <code>true</code>.
     * </p>
     * <ol>
     * <li>The file is consulted for a persistent property named
     * <code>IDE.EDITOR_KEY</code> containing the preferred editor id to be
     * used.</li>
     * <li>The workbench editor registry is consulted to determine if an editor
     * extension has been registered for the file type. If so, an instance of
     * the editor extension is opened on the file. See
     * <code>IEditorRegistry#GetDefaultEditor(std::string)</code>.</li>
     * <li>The operating system is consulted to determine if an in-place
     * component editor is available (e.g. OLE editor on Win32 platforms).</li>
     * <li>The operating system is consulted to determine if an external editor
     * is available.</li>
     * </ol>
     * </p>
     *
     * @param name
     *            the file name
     * @param inferContentType
     *            attempt to infer the content type from the file name if this
     *            is <code>true</code>
     * @return an editor descriptor, appropriate for opening the file
     * @throws PartInitException
     *             if no editor can be found
     */
  static berry::IEditorDescriptor::Pointer GetEditorDescriptor(
      const QString& name, bool inferContentType = true);

  /**
     * Returns the default editor for a given file. This method will attempt to
     * resolve the editor based on content-type bindings as well as traditional
     * name/extension bindings if <code>determineContentType</code> is
     * <code>true</code>.
     * <p>
     * A default editor id may be registered for a specific file using
     * <code>setDefaultEditor</code>. If the given file has a registered
     * default editor id the default editor will derived from it. If not, the
     * default editor is determined by taking the file name for the file and
     * obtaining the default editor for that name.
     * </p>
     *
     * @param file
     *            the file
     * @param determineContentType
     *            determine the content type for the given file
     * @return the descriptor of the default editor, or <code>null</code> if
     *         not found
     */
  static berry::IEditorDescriptor::Pointer GetDefaultEditor(const QString& file,
                                                            bool determineContentType);

};

}

#endif // MITKWORKBENCHUTIL_H
