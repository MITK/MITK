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

#ifndef CHERRYEDITORREGISTRY_H_
#define CHERRYEDITORREGISTRY_H_

#include <org.opencherry.osgi/service/cherryIExtensionPoint.h>

#include "../cherryIEditorRegistry.h"
#include "../cherryIFileEditorMapping.h"
#include "cherryEditorDescriptor.h"
#include "cherryFileEditorMapping.h"

#include <vector>
#include <map>
#include <iostream>

namespace cherry
{

/**
 * Provides access to the collection of defined editors for resource types.
 */
class EditorRegistry : public IEditorRegistry
{

  class RelatedRegistry
  {

    /**
     * Return the objects related to the type.
     * 
     * @param type
     * @return the objects related to the type
     */
    //  public: std::vector<IEditorDescriptor::Pointer> GetRelatedObjects(IContentType type) {     
    //      IEditorDescriptor[] relatedObjects = (IEditorDescriptor[]) contentTypeToEditorMappings.get(type);
    //      if (relatedObjects == null) {
    //        return EMPTY;
    //      }
    //      return (IEditorDescriptor[]) WorkbenchActivityHelper.restrictArray(relatedObjects);
    //    }

    /**
     * Return the objects related to the filename
     * @param fileName
     * @return the objects related to the filename
     */
public:
    std::vector<IEditorDescriptor::Pointer> GetRelatedObjects(
        const std::string& fileName);

  };
  
  
  /**
    * Map of FileEditorMapping (extension to FileEditorMapping) Uses two
    * java.util.HashMap: one keeps the default which are set by the plugins and
    * the other keeps the changes made by the user through the preference page.
    */
 private: class EditorMap
   {
     static std::map<std::string, FileEditorMapping::Pointer> defaultMap;

     static std::map<std::string, FileEditorMapping::Pointer> map;

     /**
      * Put a default mapping into the editor map.
      * 
      * @param key the key to set
      * @param value the value to associate
      */
   public: static void PutDefault(const std::string& key, FileEditorMapping::Pointer value);

     /**
      * Put a mapping into the user editor map.
      * 
      * @param key the key to set
      * @param value the value to associate
      */
   public: void Put(const std::string& key, FileEditorMapping::Pointer value);

     /**
      * Return the mapping associated to the key. First searches user
      * map, and then falls back to the default map if there is no match. May
      * return <code>null</code>
      * 
      * @param key
      *            the key to search for
      * @return the mapping associated to the key or <code>null</code>
      */
   public: FileEditorMapping::Pointer Get(const std::string& key);

     /**
      * Return all mappings. This will return default mappings overlayed with
      * user mappings.
      * 
      * @return the mappings
      */
   public: std::vector<FileEditorMapping::Pointer> AllMappings();

     /**
      * Return all user mappings.
      * 
      * @return the mappings
      */
   public: std::vector<FileEditorMapping::Pointer> UserMappings();
   };

  //private: Map contentTypeToEditorMappings = new HashMap();

  /*
   * Cached images - these include images from registered editors (via
   * plugins) and others hence this table is not one to one with the mappings
   * table. It is in fact a superset of the keys one would find in
   * typeEditorMappings
   */
  //private: Map extensionImages = new HashMap();

  /**
   * Vector of EditorDescriptor - all the editors loaded from plugin files.
   * The list is kept in order to be able to show in the editor selection
   * dialog of the resource associations page.  This list is sorted based on the 
   * human readable label of the editor descriptor.
   * 
   * @see #comparer
   */
private:
  std::vector<EditorDescriptor::Pointer> sortedEditorsFromPlugins;

  // Map of EditorDescriptor - map editor id to editor.
private:
  std::map<std::string, EditorDescriptor::Pointer> mapIDtoEditor;

  // Map of FileEditorMapping (extension to FileEditorMapping)
private:
  EditorMap typeEditorMappings;

  /*
   * Compares the labels from two IEditorDescriptor objects
   */
//private:
//  static final Comparator comparer = new Comparator()
//  {
//  private Collator collator = Collator.getInstance();
//
//  public int compare(Object arg0, Object arg1)
//    {
//      String s1 = ((IEditorDescriptor) arg0).getLabel();
//      String s2 = ((IEditorDescriptor) arg1).getLabel();
//      return collator.compare(s1, s2);
//    }
//  };

private: RelatedRegistry relatedRegistry;

public: static const std::string EMPTY_EDITOR_ID; // = "org.opencherry.ui.internal.emptyEditorTab"; //$NON-NLS-1$

  /**
   * Return an instance of the receiver. Adds listeners into the extension
   * registry for dynamic UI purposes.
   */
public: EditorRegistry();

  /**
   * Add an editor for the given extensions with the specified (possibly null)
   * extended type. The editor is being registered from a plugin
   * 
   * @param editor
   *            The description of the editor (as obtained from the plugin
   *            file and built by the registry reader)
   * @param extensions
   *            Collection of file extensions the editor applies to
   * @param filenames
   *            Collection of filenames the editor applies to
   * @param contentTypeVector 
   * @param bDefault
   *            Indicates whether the editor should be made the default editor
   *            and hence appear first inside a FileEditorMapping
   * 
   * This method is not API and should not be called outside the workbench
   * code.
   */
public: void AddEditorFromPlugin(EditorDescriptor::Pointer editor, const std::vector<std::string>& extensions,
      const std::vector<std::string>& filenames, const std::vector<std::string>& contentTypeVector, bool bDefault);

  /**
   * Add external editors to the editor mapping.
   */
private: void AddExternalEditorsToEditorMap();

  /*
   * (non-Javadoc) Method declared on IEditorRegistry.
   */
  //public: void AddPropertyListener(IPropertyListener l) {
  //        addListenerObject(l);
  //    }

  /*
   * (non-Javadoc) Method declared on IEditorRegistry.
   */
public: IEditorDescriptor::Pointer FindEditor(const std::string& id);

  /**
   * Fires a property changed event to all registered listeners.
   * 
   * @param type the type of event
   * @see IEditorRegistry#PROP_CONTENTS
   */
  //    private: void FirePropertyChange(final int type) {
  //        Object[] array = getListeners();
  //        for (int nX = 0; nX < array.length; nX++) {
  //            final IPropertyListener l = (IPropertyListener) array[nX];
  //            Platform.run(new SafeRunnable() {
  //                public: void run() {
  //                    l.propertyChanged(EditorRegistry.this, type);
  //                }
  //            });
  //        }
  //    }

  /*
   * (non-Javadoc) Method declared on IEditorRegistry.
   * 
   * @deprecated
   */
public: IEditorDescriptor::Pointer GetDefaultEditor();

  /*
   * (non-Javadoc) Method declared on IEditorRegistry.
   */
public: IEditorDescriptor::Pointer GetDefaultEditor(const std::string& filename);

  /**
   * Return the (approximated) content type for a file with the given name.
   * 
   * @param filename the filename
   * @return the content type or <code>null</code> if it could not be determined
   * @since 3.1
   */
  //  private: IContentType::Pointer GuessAtContentType(const std::string& filename) {
  //    return Platform.getContentTypeManager().findContentTypeFor(filename);
  //  }

  /**
   * Returns the default file image descriptor.
   * 
   * @return the image descriptor
   */
  //    private: ImageDescriptor GetDefaultImage() {
  //        // @issue what should be the default image?
  //        return WorkbenchImages.getImageDescriptor(ISharedImages.IMG_OBJ_FILE);
  //    }

  /*
   * (non-Javadoc) Method declared on IEditorRegistry.
   */
public: std::vector<IEditorDescriptor::Pointer> GetEditors(const std::string& filename);

  /*
   * (non-Javadoc) Method declared on IEditorRegistry.
   */
public: std::vector<IFileEditorMapping::Pointer> GetFileEditorMappings();

  /*
   * (non-Javadoc) Method declared on IEditorRegistry.
   */
  //    public: ImageDescriptor GetImageDescriptor(String filename) {
  //    return getImageDescriptor(filename, guessAtContentType(filename));
  //  }

  /**
   * Find the file editor mapping for the file extension. Returns
   * <code>null</code> if not found.
   * 
   * @param ext
   *            the file extension
   * @return the mapping, or <code>null</code>
   */
private: FileEditorMapping::Pointer GetMappingFor(const std::string& ext);

  /**
   * Find the file editor mappings for the given filename.
   * <p>
   * Return an array of two FileEditorMapping items, where the first mapping
   * is for the entire filename, and the second mapping is for the filename's
   * extension only. These items can be null if no mapping exist on the
   * filename and/or filename's extension.</p>
   * 
   * @param filename the filename 
   * @return the mappings
   */
private: std::vector<FileEditorMapping::Pointer> GetMappingForFilename(const std::string& filename);

  /**
   * Return the editor descriptors pulled from the OS.
   * <p>
   * WARNING! The image described by each editor descriptor is *not* known by
   * the workbench's graphic registry. Therefore clients must take care to
   * ensure that if they access any of the images held by these editors that
   * they also dispose them
   * </p>
   * @return the editor descriptors
   */
public: std::vector<IEditorDescriptor::Pointer> GetSortedEditorsFromOS();

  /**
   * Return the editors loaded from plugins.
   * 
   * @return the sorted array of editors declared in plugins
   * @see #comparer
   */
public: std::vector<IEditorDescriptor::Pointer> GetSortedEditorsFromPlugins();

  /**
   * Answer an intial id to editor map. This will create a new map and
   * populate it with the default system editors.
   * 
   * @param initialSize
   *            the initial size of the map
   * @return the new map
   */
private: void InitialIdToEditorMap(const std::map<std::string, EditorDescriptor::Pointer>& map);

  /**
   * Add the system editors to the provided map. This will always add an
   * editor with an id of {@link #SYSTEM_EXTERNAL_EDITOR_ID} and may also add
   * an editor with id of {@link #SYSTEM_INPLACE_EDITOR_ID} if the system
   * configuration supports it.
   * 
   * @param map the map to augment
   */
private: void AddSystemEditors(std::map<std::string, EditorDescriptor::Pointer>& map);

  /**
   * Initialize the registry state from plugin declarations and preference
   * overrides.
   */
private: void InitializeFromStorage();

  /**
   * Set the default editors according to the preference store which can be
   * overwritten in the file properties.ini.  In the form: 
   * <p>
   * <code>ext1:id1;ext2:id2;...</code>
   * </p>
   * 
   * @param defaultEditors the default editors to set
   */
private: void SetProductDefaults(const std::string& defaultEditors);

  /**
   * Read the editors defined in the preferences store.
   * 
   * @param editorTable
   *            Editor table to store the editor definitions.
   * @return true if the table is built succesfully.
   */
private: bool ReadEditors(std::map<std::string, EditorDescriptor::Pointer>& editorTable);

  /**
   * Read the file types and associate them to their defined editor(s).
   * 
   * @param editorTable
   *            The editor table containing the defined editors.
   * @param reader
   *            Reader containing the preferences content for the resources.
   * 
   * @throws WorkbenchException
   */
public: void ReadResources(std::map<std::string, EditorDescriptor::Pointer>& editorTable, std::ostream& reader);

  /**
   * Determine if the editors list contains the editor descriptor.
   * 
   * @param editorsArray
   *      The list of editors
   * @param editorDescriptor
   *      The editor descriptor
   * @return <code>true</code> if the editors list contains the editor descriptor
   */
private: bool Contains(const std::vector<IEditorDescriptor::Pointer>& editorsArray,
      IEditorDescriptor::Pointer editorDescriptor);

  /**
   * Creates the reader for the resources preferences defined in the
   * preference store.
   * 
   * @param editorTable
   *            The editor table containing the defined editors.
   * @return true if the resources are read succesfully.
   */
private: bool ReadResources(std::map<std::string, EditorDescriptor::Pointer>& editorTable);

  /**
   * Load the serialized resource associations Return true if the operation
   * was successful, false otherwise
   */
private: bool LoadAssociations();

  /**
   * Return a friendly version of the given key suitable for use in the editor
   * map.
   */
private: std::string MappingKeyFor(const std::string& type);

  /**
   * Return a key that combines the file's name and extension of the given
   * mapping
   * 
   * @param mapping the mapping to generate a key for
   */
private: std::string MappingKeyFor(FileEditorMapping::Pointer mapping);

  /**
   * Rebuild the editor map
   */
private: void RebuildEditorMap();

  /**
   * Rebuild the internal editor mapping.
   */
private: void RebuildInternalEditorMap();

  /*
   * (non-Javadoc) Method declared on IEditorRegistry.
   */
  //    public: void RemovePropertyListener(IPropertyListener l) {
  //        removeListenerObject(l);
  //    }

  /**
   * Save the registry to the filesystem by serializing the current resource
   * associations.
   */
public: void SaveAssociations();

  /**
   * Set the collection of FileEditorMappings. The given collection is
   * converted into the internal hash table for faster lookup Each mapping
   * goes from an extension to the collection of editors that work on it. This
   * operation will rebuild the internal editor mappings.
   * 
   * @param newResourceTypes
   *            te new file editor mappings.
   */
public: void SetFileEditorMappings(const std::vector<FileEditorMapping::Pointer>& newResourceTypes);

  /*
   * (non-Javadoc) Method declared on IEditorRegistry.
   */
public: void SetDefaultEditor(const std::string& fileName, const std::string& editorId);

  /**
   * Alphabetically sort the internal editors.
   * 
   * @see #comparer
   */
private: std::vector<IEditorDescriptor::Pointer> SortEditors(const std::vector<IEditorDescriptor>& unsortedList);

  /**
   * Alphabetically sort the internal editors.
   * 
   * @see #comparer
   */
private: void SortInternalEditors();

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IEditorRegistry#isSystemInPlaceEditorAvailable(String)
   */
public: bool IsSystemInPlaceEditorAvailable(const std::string& filename);

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IEditorRegistry#isSystemExternalEditorAvailable(String)
   */
public: bool IsSystemExternalEditorAvailable(const std::string& filename);

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IEditorRegistry#getSystemExternalEditorImageDescriptor(java.lang.String)
   */
  //    public: ImageDescriptor GetSystemExternalEditorImageDescriptor(
  //            const std::string& filename) {
  //        Program externalProgram = null;
  //        int extensionIndex = filename.lastIndexOf('.');
  //        if (extensionIndex >= 0) {
  //            externalProgram = Program.findProgram(filename
  //                    .substring(extensionIndex));
  //        }
  //        if (externalProgram == null) {
  //            return null;
  //        } 
  //        
  //        return new ExternalProgramImageDescriptor(externalProgram);
  //    }

  /**
   * Removes the entry with the value of the editor descriptor from the given
   * map. If the descriptor is the last descriptor in a given
   * FileEditorMapping then the mapping is removed from the map.
   * 
   * @param map
   *            the map to search
   * @param desc
   *            the descriptor value to remove
   */
private: void RemoveEditorFromMapping(std::map<std::string, FileEditorMapping::Pointer>& map, IEditorDescriptor::Pointer desc);

private: IExtensionPoint* GetExtensionPointFilter();

  /* (non-Javadoc)
   * @see org.eclipse.ui.IEditorRegistry#getDefaultEditor(java.lang.String, org.eclipse.core.runtime.content.IContentType)
   */
  //  public: IEditorDescriptor::Pointer GetDefaultEditor(const std::string& fileName, IContentType contentType) {
  //        return getEditorForContentType(fileName, contentType);
  //  }

  /**
   * Return the editor for a file with a given content type.
   * 
   * @param filename the file name
   * @param contentType the content type
   * @return the editor for a file with a given content type
   * @since 3.1
   */
  //  private: IEditorDescriptor::Pointer GetEditorForContentType(const std::string& filename,
  //      IContentType contentType) {
  //    IEditorDescriptor desc = null;
  //    Object[] contentTypeResults = findRelatedObjects(contentType, filename, relatedRegistry);
  //    if (contentTypeResults != null && contentTypeResults.length > 0) {
  //      desc = (IEditorDescriptor) contentTypeResults[0];
  //    }
  //    return desc;
  //  }

  /* (non-Javadoc)
   * @see org.eclipse.ui.IEditorRegistry#getEditors(java.lang.String, org.eclipse.core.runtime.content.IContentType)
   */
  //  public: std::vector<IEditorDescriptor::Pointer> GetEditors(const std::string& fileName, IContentType contentType) {
  //    return findRelatedObjects(contentType, fileName, relatedRegistry);
  //  }

  /* (non-Javadoc)
   * @see org.eclipse.ui.IEditorRegistry#getImageDescriptor(java.lang.String, org.eclipse.core.runtime.content.IContentType)
   */
  //  public: ImageDescriptor GetImageDescriptor(const std::string filename, IContentType contentType) {
  //        if (filename == null) {
  //      return getDefaultImage();
  //    }
  //
  //    if (contentType != null) {
  //      IEditorDescriptor desc = getEditorForContentType(filename, contentType);
  //      if (desc != null) {
  //        ImageDescriptor anImage = (ImageDescriptor) extensionImages.get(desc);  
  //        if (anImage != null) {
  //          return anImage;
  //        }
  //        anImage = desc.getImageDescriptor();
  //        extensionImages.put(desc, anImage);
  //        return anImage;       
  //      }
  //    }
  //        // Lookup in the cache first...
  //        String key = mappingKeyFor(filename);
  //        ImageDescriptor anImage = (ImageDescriptor) extensionImages.get(key);
  //        if (anImage != null) {
  //      return anImage;
  //    }
  //
  //        // See if we have a mapping for the filename or extension
  //        FileEditorMapping[] mapping = getMappingForFilename(filename);
  //        for (int i = 0; i < 2; i++) {
  //            if (mapping[i] != null) {
  //                // Lookup in the cache first...
  //                String mappingKey = mappingKeyFor(mapping[i]);
  //                ImageDescriptor mappingImage = (ImageDescriptor) extensionImages
  //                        .get(key);
  //                if (mappingImage != null) {
  //          return mappingImage;
  //        }
  //                // Create it and cache it
  //                IEditorDescriptor editor = mapping[i].getDefaultEditor();
  //                if (editor != null) {
  //                    mappingImage = editor.getImageDescriptor();
  //                    extensionImages.put(mappingKey, mappingImage);
  //                    return mappingImage;
  //                }
  //            }
  //        }
  //
  //        // Nothing - time to look externally for the icon
  //        anImage = getSystemExternalEditorImageDescriptor(filename);
  //        if (anImage == null) {
  //      anImage = getDefaultImage();
  //    }
  //        //  for dynamic UI - comment out the next line
  //        //extensionImages.put(key, anImage);
  //        return anImage;
  //
  //  }

  /**
   * Find objects related to the content type.
   * 
   * This method is temporary and exists only to back us off of the
   * soon-to-be-removed IContentTypeManager.IRelatedRegistry API.
   * 
   * @param type
   * @param fileName
   * @param registry
   * @return the related objects
   */
  //  private: std::vector<IEditorDescriptor::Pointer> FindRelatedObjects(IContentType type, const std::string& fileName,
  //      RelatedRegistry& registry) {
  //    List allRelated = new ArrayList();
  //    List nonDefaultFileEditors = new ArrayList();
  //    IEditorDescriptor [] related;
  //    
  //    if (fileName != null) {
  //      FileEditorMapping mapping = getMappingFor(fileName);
  //      if (mapping != null) {
  //        // backwards compatibility - add editors flagged as "default"
  //        related = mapping.getDeclaredDefaultEditors();
  //        for (int i = 0; i < related.length; i++) {
  //          // we don't want to return duplicates
  //          if (!allRelated.contains(related[i])) {
  //            // if it's not filtered, add it to the list
  //            if (!WorkbenchActivityHelper.filterItem(related[i])) {
  //              allRelated.add(related[i]);
  //            }
  //          }
  //        }
  //        
  //        // add all filename editors to the nonDefaultList
  //        // we'll later try to add them all after content types are resolved
  //        // duplicates (ie: default editors) will be ignored
  //        nonDefaultFileEditors.addAll(Arrays.asList(mapping.getEditors()));
  //      }
  //      
  //      int index = fileName.lastIndexOf('.');
  //      if (index > -1) {
  //        String extension = "*" + fileName.substring(index); //$NON-NLS-1$
  //        mapping = getMappingFor(extension);
  //        if (mapping != null) {
  //          related = mapping.getDeclaredDefaultEditors();
  //          for (int i = 0; i < related.length; i++) {
  //            // we don't want to return duplicates
  //            if (!allRelated.contains(related[i])) {
  //              // if it's not filtered, add it to the list
  //              if (!WorkbenchActivityHelper.filterItem(related[i])) {
  //                allRelated.add(related[i]);
  //              }
  //            }
  //          }
  //          nonDefaultFileEditors.addAll(Arrays.asList(mapping.getEditors()));
  //        }
  //      }
  //    }
  //    
  //    if (type != null) {
  //      // now add any objects directly related to the content type
  //      related = registry.getRelatedObjects(type);
  //      for (int i = 0; i < related.length; i++) {
  //        // we don't want to return duplicates
  //        if (!allRelated.contains(related[i])) {
  //          // if it's not filtered, add it to the list
  //          if (!WorkbenchActivityHelper.filterItem(related[i])) {
  //            allRelated.add(related[i]);
  //          }
  //        }
  //      }
  //
  //    }
  //
  //    if (type != null) {
  //      // now add any indirectly related objects, walking up the content type hierarchy 
  //      while ((type = type.getBaseType()) != null) {
  //        related = registry.getRelatedObjects(type);
  //        for (int i = 0; i < related.length; i++) {
  //          // we don't want to return duplicates
  //          if (!allRelated.contains(related[i])) {
  //            // if it's not filtered, add it to the list
  //            if (!WorkbenchActivityHelper.filterItem(related[i])) {
  //              allRelated.add(related[i]);
  //            }
  //          }
  //        }
  //      }
  //    }
  //      
  //    // add all non-default editors to the list
  //    for (Iterator i = nonDefaultFileEditors.iterator(); i.hasNext();) {
  //      IEditorDescriptor editor = (IEditorDescriptor) i.next();
  //      if (!allRelated.contains(editor) && !WorkbenchActivityHelper.filterItem(editor)) {
  //        allRelated.add(editor);
  //      }
  //    }
  //    
  //    return (IEditorDescriptor []) allRelated.toArray(new IEditorDescriptor [allRelated
  //        .size()]);
  //  }

  /**
   * Return the editors bound to this content type, either directly or indirectly.
   * 
   * @param type the content type to check
   * @return the editors
   * @since 3.1
   *
   * TODO: this should be rolled in with the above findRelatedObjects code
   */
  //  public: std::vector<IEditorDescriptor> GetEditorsForContentType(IContentType type) {
  //    ArrayList allRelated = new ArrayList();
  //    if (type == null) {
  //      return new IEditorDescriptor [0];
  //    }
  //    
  //    Object [] related = relatedRegistry.getRelatedObjects(type);
  //    for (int i = 0; i < related.length; i++) {  
  //      // we don't want to return duplicates
  //      if (!allRelated.contains(related[i])) {
  //        // if it's not filtered, add it to the list
  //        if (!WorkbenchActivityHelper.filterItem(related[i])) {
  //          allRelated.add(related[i]);
  //        }
  //        
  //      }
  //    }
  //    
  //    // now add any indirectly related objects, walking up the content type hierarchy 
  //    while ((type = type.getBaseType()) != null) {
  //      related = relatedRegistry.getRelatedObjects(type);
  //      for (int i = 0; i < related.length; i++) {
  //        // we don't want to return duplicates
  //        if (!allRelated.contains(related[i])) {
  //          // if it's not filtered, add it to the list
  //          if (!WorkbenchActivityHelper.filterItem(related[i])) {
  //            allRelated.add(related[i]);
  //          }
  //        }
  //      }
  //    }
  //    
  //    return (IEditorDescriptor[]) allRelated.toArray(new IEditorDescriptor[allRelated.size()]);
  //  }

  /**
   * Get filemappings for all defined filetypes, including those defined by content type.
   * 
   * @return the filetypes
   * @since 3.1
   */
public: std::vector<IFileEditorMapping::Pointer> GetUnifiedMappings();

};

class MockMapping : public IFileEditorMapping
{

  //private: IContentType contentType;
private:
  std::string extension;
private:
  std::string filename;

  MockMapping(/*IContentType type,*/const std::string& name,
      const std::string& ext);

public:
  IEditorDescriptor::Pointer GetDefaultEditor();

public:
  std::vector<IEditorDescriptor::Pointer> GetEditors();

public:
  std::vector<IEditorDescriptor::Pointer> GetDeletedEditors();

public:
  std::string GetExtension();

  //  public: ImageDescriptor GetImageDescriptor() {
  //    IEditorDescriptor editor = getDefaultEditor();
  //    if (editor == null) {
  //      return WorkbenchImages
  //          .getImageDescriptor(ISharedImages.IMG_OBJ_FILE);
  //    }
  //
  //    return editor.getImageDescriptor();
  //  }

public:
  std::string GetLabel();

public:
  std::string GetName();

  /* (non-Javadoc)
   * @see java.lang.Object#equals(java.lang.Object)
   */
public:
  bool operator==(const MockMapping* obj);
};

}

#endif /*CHERRYEDITORREGISTRY_H_*/
