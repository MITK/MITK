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

#ifndef CHERRYEDITORDESCRIPTOR_H_
#define CHERRYEDITORDESCRIPTOR_H_

#include "../cherryIEditorDescriptor.h"
#include "../cherryIMemento.h"

#include <cherryIConfigurationElement.h>

namespace cherry
{

struct IEditorPart;

/**
 * \ingroup org_opencherry_ui_internal
 * 
 * @see IEditorDescriptor
 */
class CHERRY_UI EditorDescriptor : public IEditorDescriptor
{ //, Serializable, IPluginContribution {

public:
  cherryObjectMacro(EditorDescriptor)

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

  // ImageDescriptor imageDesc;

  //bool testImage = true;

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
   * @return org.opencherry.swt.program.Program
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
public: std::string GetClassName();

  /**
   * Return the configuration element used to define this editor, or <code>null</code>.
   * 
   * @return the element or null
   */
public: IConfigurationElement::Pointer GetConfigurationElement();

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
public: std::string GetFileName();

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
  //public: ImageDescriptor GetImageDescriptor() {
  //      if (testImage) {
  //        testImage = false;
  //      if (imageDesc == null) {
  //        String imageFileName = getImageFilename();
  //        String command = getFileName();
  //        if (imageFileName != null && configurationElement != null) {
  //          imageDesc = AbstractUIPlugin.imageDescriptorFromPlugin(
  //              configurationElement.getNamespace(), imageFileName);
  //        } else if (command != null) {
  //          imageDesc = WorkbenchImages.getImageDescriptorFromProgram(
  //              command, 0);
  //        }
  //      }
  //      verifyImage();        
  //      }
  //      
  //        return imageDesc;
  //    }

  /**
   * Verifies that the image descriptor generates an image.  If not, the 
   * descriptor is replaced with the default image.
   * 
   * @since 3.1
   */
  //private: void VerifyImage() {
  //    if (imageDesc == null) {
  //      imageDesc = WorkbenchImages
  //            .getImageDescriptor(ISharedImages.IMG_OBJ_FILE);
  //    }
  //    else {
  //      Image img = imageDesc.createImage(false);
  //      if (img == null) {
  //          // @issue what should be the default image?
  //          imageDesc = WorkbenchImages
  //                  .getImageDescriptor(ISharedImages.IMG_OBJ_FILE);
  //      } else {
  //          img.dispose();
  //      }
  //    }
  //  }

  /**
   * The name of the image describing this editor.
   * 
   * @return the image file name
   */
  //public: std::string GetImageFilename() {
  //      if (configurationElement == null) {
  //      return imageFilename;
  //    }
  //      return configurationElement.getAttribute(IWorkbenchRegistryConstants.ATT_ICON);
  //    }

  /**
   * Return the user printable label for this editor.
   * 
   * @return the label
   */
public: std::string GetLabel();

  /**
   * Returns the class name of the launcher.
   * 
   * @return the launcher class name
   */
public: std::string GetLauncher();

  /**
   * Return the contributing plugin id.
   * 
   * @return the contributing plugin id
   */
public: std::string GetPluginID();

  /**
   * Get the program for the receiver if there is one.
   * @return Program
   */
  //public: Program GetProgram() {
  //        return this.program;
  //    }

  /* (non-Javadoc)
   * @see org.opencherry.ui.IEditorDescriptor#isInternal
   */
public: bool IsInternal();

  /* (non-Javadoc)
   * @see org.opencherry.ui.IEditorDescriptor#isOpenInPlace
   */
public: bool IsOpenInPlace();

  /* (non-Javadoc)
   * @see org.opencherry.ui.IEditorDescriptor#isOpenExternal
   */
public: bool IsOpenExternal();

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
private: int GetOpenMode();

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
public: std::string ToString();

  /* (non-Javadoc)
   * @see org.opencherry.ui.activities.support.IPluginContribution#getLocalId()
   */
public: std::string GetLocalId();

  /* (non-Javadoc)
   * @see org.opencherry.ui.activities.support.IPluginContribution#getPluginId()
   */
public: std::string GetPluginId();

  /* (non-Javadoc)
   * @see org.opencherry.ui.IEditorDescriptor#getEditorManagementPolicy()
   */
public: IEditorMatchingStrategy::Pointer GetEditorMatchingStrategy();

};

}

#endif /*CHERRYEDITORDESCRIPTOR_H_*/
