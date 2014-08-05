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


#ifndef BERRYPERSPECTIVEREGISTRY_H_
#define BERRYPERSPECTIVEREGISTRY_H_

#include "berryIPerspectiveRegistry.h"

#include "berryPerspectiveDescriptor.h"

#include <list>

namespace berry {

/**
 * Perspective registry.
 */
class PerspectiveRegistry : public IPerspectiveRegistry {
    // IExtensionChangeHandler {

  friend class PerspectiveDescriptor;

private:

  std::string defaultPerspID;

  static const std::string EXT; // = "_persp.xml";

  static const std::string ID_DEF_PERSP; // = "PerspectiveRegistry.DEFAULT_PERSP";

  static const std::string PERSP; // = "_persp";

  static const char SPACE_DELIMITER; // = ' ';

  std::list<PerspectiveDescriptor::Pointer> perspectives;

  // keep track of the perspectives the user has selected to remove or revert
  std::list<std::string> perspToRemove;

  //IPropertyChangeListener::Pointer preferenceListener;

public:

  /**
   * Construct a new registry.
   */
  PerspectiveRegistry();

  /**
     * Adds a perspective. This is typically used by the reader.
     *
     * @param desc
     */
    void AddPerspective(PerspectiveDescriptor::Pointer desc);

    /**
       * Create a new perspective.
       *
       * @param label
       *            the name of the new descriptor
       * @param originalDescriptor
       *            the descriptor on which to base the new descriptor
       * @return a new perspective descriptor or <code>null</code> if the
       *         creation failed.
       */
      IPerspectiveDescriptor::Pointer CreatePerspective(const std::string& label,
          IPerspectiveDescriptor::Pointer originalDescriptor);

      /**
       * Reverts a list of perspectives back to the plugin definition
       *
       * @param perspToRevert
       */
      void RevertPerspectives(const std::list<PerspectiveDescriptor::Pointer>& perspToRevert);

      /**
       * Deletes a list of perspectives
       *
       * @param perspToDelete
       */
      void DeletePerspectives(const std::list<PerspectiveDescriptor::Pointer>& perspToDelete);

      /**
       * Delete a perspective. Has no effect if the perspective is defined in an
       * extension.
       *
       * @param in
       */
      void DeletePerspective(IPerspectiveDescriptor::Pointer in);

      /*
         * (non-Javadoc)
         *
         * @see org.blueberry.ui.IPerspectiveRegistry#findPerspectiveWithId(java.lang.std::string)
         */
         IPerspectiveDescriptor::Pointer FindPerspectiveWithId(const std::string& id);

        /*
         * (non-Javadoc)
         *
         * @see org.blueberry.ui.IPerspectiveRegistry#findPerspectiveWithLabel(java.lang.std::string)
         */
         IPerspectiveDescriptor::Pointer FindPerspectiveWithLabel(const std::string& label);

        /**
         * @see IPerspectiveRegistry#getDefaultPerspective()
         */
         std::string GetDefaultPerspective();

        /*
         * (non-Javadoc)
         *
         * @see org.blueberry.ui.IPerspectiveRegistry#getPerspectives()
         */
         std::vector<IPerspectiveDescriptor::Pointer> GetPerspectives();

        /**
         * Loads the registry.
         */
        void Load();

        /**
           * Saves a custom perspective definition to the preference store.
           *
           * @param desc
           *            the perspective
           * @param memento
           *            the memento to save to
           * @throws IOException
           */
        //  void SaveCustomPersp(PerspectiveDescriptor::Pointer desc, XMLMemento::Pointer memento);

          /**
           * Gets the Custom perspective definition from the preference store.
           *
           * @param id
           *            the id of the perspective to find
           * @return IMemento a memento containing the perspective description
           *
           * @throws WorkbenchException
           * @throws IOException
           */
          IMemento::Pointer GetCustomPersp(const std::string& id);

          /**
             * @see IPerspectiveRegistry#setDefaultPerspective(std::string)
             */
            void SetDefaultPerspective(const std::string& id);

            /**
             * Return <code>true</code> if a label is valid. This checks only the
             * given label in isolation. It does not check whether the given label is
             * used by any existing perspectives.
             *
             * @param label
             *            the label to test
             * @return whether the label is valid
             */
            bool ValidateLabel(const std::string& label);

            /*
               * (non-Javadoc)
               *
               * @see org.blueberry.ui.IPerspectiveRegistry#clonePerspective(java.lang.std::string,
               *      java.lang.std::string, org.blueberry.ui.IPerspectiveDescriptor)
               */
              IPerspectiveDescriptor::Pointer ClonePerspective(const std::string& id, const std::string& label,
                  IPerspectiveDescriptor::Pointer originalDescriptor);

              /*
               * (non-Javadoc)
               *
               * @see org.blueberry.ui.IPerspectiveRegistry#revertPerspective(org.blueberry.ui.IPerspectiveDescriptor)
               */
              void RevertPerspective(IPerspectiveDescriptor::Pointer perspToRevert);

              /**
               * Dispose the receiver.
               */
              ~PerspectiveRegistry();

              /*
               * (non-Javadoc)
               *
               * @see org.blueberry.core.runtime.dynamicHelpers.IExtensionChangeHandler#removeExtension(org.blueberry.core.runtime.IExtension,
               *      java.lang.Object[])
               */
//              void removeExtension(IExtension source, Object[] objects) {
//                for (int i = 0; i < objects.length; i++) {
//                  if (objects[i] instanceof PerspectiveDescriptor) {
//                    // close the perspective in all windows
//                    IWorkbenchWindow[] windows = PlatformUI.getWorkbench()
//                        .getWorkbenchWindows();
//                    PerspectiveDescriptor desc = (PerspectiveDescriptor) objects[i];
//                    for (int w = 0; w < windows.length; ++w) {
//                      IWorkbenchWindow window = windows[w];
//                      IWorkbenchPage[] pages = window.getPages();
//                      for (int p = 0; p < pages.length; ++p) {
//                        WorkbenchPage page = (WorkbenchPage) pages[p];
//                        ClosePerspectiveHandler.closePerspective(page, page
//                            .findPerspective(desc));
//                      }
//                    }
//
//                    // ((Workbench)PlatformUI.getWorkbench()).getPerspectiveHistory().removeItem(desc);
//
//                    internalDeletePerspective(desc);
//                  }
//
//                }
//              }

              /*
               * (non-Javadoc)
               *
               * @see org.blueberry.core.runtime.dynamicHelpers.IExtensionChangeHandler#addExtension(org.blueberry.core.runtime.dynamicHelpers.IExtensionTracker,
               *      org.blueberry.core.runtime.IExtension)
               */
//              void addExtension(IExtensionTracker tracker,
//                  IExtension addedExtension) {
//                IConfigurationElement[] addedElements = addedExtension
//                    .getConfigurationElements();
//                for (int i = 0; i < addedElements.length; i++) {
//                  PerspectiveRegistryReader reader = new PerspectiveRegistryReader(
//                      this);
//                  reader.readElement(addedElements[i]);
//                }
//              }

protected:

  /**
     * Removes the custom definition of a perspective from the preference store
     *
     * @param desc
     */
    /* package */
    void DeleteCustomDefinition(PerspectiveDescriptor::Pointer desc);

    /**
     * Method hasCustomDefinition.
     *
     * @param desc
     */
    /* package */
    bool HasCustomDefinition(PerspectiveDescriptor::ConstPointer desc) const;

private:

  /**
   * Initialize the preference change listener.
   */
  void InitializePreferenceChangeListener();

  /**
   * @param desc
   */
  void Add(PerspectiveDescriptor::Pointer desc);

  /**
   * Delete a perspective. This will remove perspectives defined in
   * extensions.
   *
   * @param desc
   *            the perspective to delete
   * @since 3.1
   */
  void InternalDeletePerspective(PerspectiveDescriptor::Pointer desc);

  /**
   * Read children from the file system.
   */
  void LoadCustom();

  /**
   * @param status
   */
  void UnableToLoadPerspective(const std::string& status);

  /**
   * Read children from the plugin registry.
   */
  void LoadPredefined();

  /**
   * Verifies the id of the default perspective. If the default perspective is
   * invalid use the workbench default.
   */
  void VerifyDefaultPerspective();

};

}

#endif /* BERRYPERSPECTIVEREGISTRY_H_ */
