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

#ifndef BERRYEDITORDESCRIPTOR_H_
#define BERRYEDITORDESCRIPTOR_H_

#include "berryIEditorDescriptor.h"
#include "berryIMemento.h"

#include <berryIConfigurationElement.h>

namespace berry
{

struct IEditorPart;

/**
 * \ingroup org_blueberry_ui_internal
 *
 * @see IEditorDescriptor
 */
class BERRY_UI EditorDescriptor : public IEditorDescriptor
{ //, Serializable, IPluginContribution {

public:
  berryObjectMacro(EditorDescriptor);

  // @issue the following constants need not be public; see bug 47600
  /**
   * Open internal constant.  Value <code>0x01</code>.
   */
  static const int OPEN_INTERNAL; // = 0x01;

  /**
   * Open in place constant.  Value <code>0x02</code>.
   */
  static const int OPEN_INPLACE; // = 0x02;

  /**
   * Open external constant.  Value <code>0x04</code>.
   */
  static const int OPEN_EXTERNAL; // = 0x04;


private:

  std::string editorName;

  std::string imageFilename;

  mutable SmartPointer<ImageDescriptor> imageDesc;

  mutable bool testImage;

  std::string className;

  std::string launcherName;

  std::string fileName;

  std::string id;

  bool matchingStrategyChecked;
  IEditorMatchingStrategy::Pointer matchingStrategy;

  //Program program;

  //The id of the plugin which contributed this editor, null for external editors
  std::string pluginIdentifier;

  int openMode;

  IConfigurationElement::Pointer configurationElement;

  /**
   * Create a new instance of an editor descriptor. Limited
   * to internal framework calls.
   * @param element
   * @param id2
   */
  /* package */
public: EditorDescriptor(const std::string& id2, IConfigurationElement::Pointer element);

  /**
   * Create a new instance of an editor descriptor. Limited
   * to internal framework calls.
   */
  /* package */
public: EditorDescriptor();

  /**
   * Creates a descriptor for an external program.
   *
   * @param filename the external editor full path and filename
   * @return the editor descriptor
   */
  //public: static EditorDescriptor::Pointer CreateForProgram(const std::string& filename) {
  //        if (filename == null) {
  //            throw new IllegalArgumentException();
  //        }
  //        EditorDescriptor editor = new EditorDescriptor();
  //
  //        editor.setFileName(filename);
  //        editor.setID(filename);
  //        editor.setOpenMode(OPEN_EXTERNAL);
  //
  //        //Isolate the program name (no directory or extension)
  //        int start = filename.lastIndexOf(File.separator);
  //        String name;
  //        if (start != -1) {
  //            name = filename.substring(start + 1);
  //        } else {
  //            name = filename;
  //        }
  //        int end = name.lastIndexOf('.');
  //        if (end != -1) {
  //            name = name.substring(0, end);
  //        }
  //        editor.setName(name);
  //
  //        // get the program icon without storing it in the registry
  //        ImageDescriptor imageDescriptor = new ProgramImageDescriptor(filename,
  //                0);
  //        editor.setImageDescriptor(imageDescriptor);
  //
  //        return editor;
  //    }

  /**
   * Return the program called programName. Return null if it is not found.
   * @return org.blueberry.swt.program.Program
   */
  //private: static Program FindProgram(const std::string& programName) {
  //
  //        Program[] programs = Program.getPrograms();
  //        for (int i = 0; i < programs.length; i++) {
  //            if (programs[i].getName().equals(programName)) {
  //        return programs[i];
  //      }
  //        }
  //
  //        return null;
  //    }

  /**
   * Create the editor action bar contributor for editors of this type.
   *
   * @return the action bar contributor, or <code>null</code>
   */
//public: IEditorActionBarContributor::Pointer CreateActionBarContributor();

  /**
   * Return the editor class name.
   *
   * @return the class name
   */
public: std::string GetEditorClassName() const;

  /**
   * Return the configuration element used to define this editor, or <code>null</code>.
   *
   * @return the element or null
   */
public: IConfigurationElement::Pointer GetConfigurationElement() const;

  /**
   * Create an editor part based on this descriptor.
   *
   * @return the editor part
   * @throws CoreException thrown if there is an issue creating the editor
   */
public: SmartPointer<IEditorPart> CreateEditor();

  /**
   * Return the file name of the command to execute for this editor.
   *
   * @return the file name to execute
   */
public: std::string GetFileName() const;

  /**
   * Return the id for this editor.
   *
   * @return the id
   */
public: std::string GetId() const;

  /**
   * Return the image descriptor describing this editor.
   *
   * @return the image descriptor
   */
public: SmartPointer<ImageDescriptor> GetImageDescriptor() const;

  /**
   * Verifies that the image descriptor generates an image.  If not, the
   * descriptor is replaced with the default image.
   *
   * @since 3.1
   */
private: void VerifyImage() const;

  /**
   * The name of the image describing this editor.
   *
   * @return the image file name
   */
public: std::string GetImageFilename() const;

  /**
   * Return the user printable label for this editor.
   *
   * @return the label
   */
public: std::string GetLabel() const;

  /**
   * Returns the class name of the launcher.
   *
   * @return the launcher class name
   */
public: std::string GetLauncher() const;

  /**
   * Return the contributing plugin id.
   *
   * @return the contributing plugin id
   */
public: std::string GetPluginID() const;

  /**
   * Get the program for the receiver if there is one.
   * @return Program
   */
  //public: Program GetProgram() {
  //        return this.program;
  //    }

  /* (non-Javadoc)
   * @see org.blueberry.ui.IEditorDescriptor#isInternal
   */
public: bool IsInternal() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IEditorDescriptor#isOpenInPlace
   */
public: bool IsOpenInPlace() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IEditorDescriptor#isOpenExternal
   */
public: bool IsOpenExternal() const;

  /**
   * Load the object properties from a memento.
   *
   * @return <code>true</code> if the values are valid, <code>false</code> otherwise
   */
protected: bool LoadValues(IMemento::Pointer memento);

  /**
   * Save the object values in a IMemento
   */
protected: void SaveValues(IMemento::Pointer memento);

  /**
   * Return the open mode of this editor.
   *
   * @return the open mode of this editor
   * @since 3.1
   */
private: int GetOpenMode() const;

  /**
   * Set the class name of an internal editor.
   */
  /* package */public: void SetClassName(const std::string& newClassName);

  /**
   * Set the configuration element which contributed this editor.
   */
  /* package */public: void SetConfigurationElement(
      IConfigurationElement::Pointer newConfigurationElement);

  /**
   * Set the filename of an external editor.
   */
  /* package */public: void SetFileName(const std::string& aFileName);

  /**
   * Set the id of the editor.
   * For internal editors this is the id as provided in the extension point
   * For external editors it is path and filename of the editor
   */
  /* package */public: void SetID(const std::string& anID);

  /**
   * The Image to use to repesent this editor
   */
  /* package */
  //    public : void SetImageDescriptor(ImageDescriptor desc) {
  //        imageDesc = desc;
  //        testImage = true;
  //    }

  /**
   * The name of the image to use for this editor.
   */
  /* package */
  //    public: void SetImageFilename(const std::string& aFileName) {
  //        imageFilename = aFileName;
  //    }

  /**
   * Sets the new launcher class name
   *
   * @param newLauncher the new launcher
   */
  /* package */public: void SetLauncher(const std::string& newLauncher);

  /**
   * The label to show for this editor.
   */
  /* package */public: void SetName(const std::string& newName);

  /**
   * Sets the open mode of this editor descriptor.
   *
   * @param mode the open mode
   *
   * @issue this method is public as a temporary fix for bug 47600
   */
public: void SetOpenMode(int mode);

  /**
   * The id of the plugin which contributed this editor, null for external editors.
   */
  /* package */public: void SetPluginIdentifier(const std::string& anID);

  /**
   * Set the receivers program.
   * @param newProgram
   */
  /* package */
  //    public: void SetProgram(Program newProgram) {
  //
  //        this.program = newProgram;
  //        if (editorName == null) {
  //      setName(newProgram.getName());
  //    }
  //    }

  /**
   * For debugging purposes only.
   */
public: std::string ToString() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.activities.support.IPluginContribution#getLocalId()
   */
public: std::string GetLocalId() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.activities.support.IPluginContribution#getPluginId()
   */
public: std::string GetPluginId() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IEditorDescriptor#getEditorManagementPolicy()
   */
public: IEditorMatchingStrategy::Pointer GetEditorMatchingStrategy();

};

}

#endif /*BERRYEDITORDESCRIPTOR_H_*/
