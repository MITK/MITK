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

#include "cherryEditorRegistry.h"

namespace cherry
{

std::vector<IEditorDescriptor::Pointer> EditorRegistry::RelatedRegistry::GetRelatedObjects(
    const std::string& fileName)
{
  IFileEditorMapping::Pointer mapping = getMappingFor(fileName);
  if (mapping == null)
  {
    return EMPTY;
  }

  return (IEditorDescriptor[]) WorkbenchActivityHelper.restrictArray(mapping.getEditors());
}

static const std::string EditorRegistry::EMPTY_EDITOR_ID =
    "org.opencherry.ui.internal.emptyEditorTab"; //$NON-NLS-1$

EditorRegistry::EditorRegistry()
{
  this->InitialIdToEditorMap(mapIDtoEditor);
  initializeFromStorage();
  IExtensionTracker tracker = PlatformUI.getWorkbench().getExtensionTracker();
  tracker.registerHandler(this,
      ExtensionTracker.createExtensionPointFilter(getExtensionPointFilter()));
  relatedRegistry = new RelatedRegistry();
}

void EditorRegistry::AddEditorFromPlugin(EditorDescriptor::Pointer editor,
    const std::vector<std::string>& extensions,
    const std::vector<std::string>& filenames,
    const std::vector<std::string>& contentTypeVector, bool bDefault)
{

  PlatformUI.getWorkbench().getExtensionTracker().registerObject(editor.getConfigurationElement().getDeclaringExtension(), editor, IExtensionTracker.REF_WEAK);
  // record it in our quick reference list
  sortedEditorsFromPlugins.add(editor);

  // add it to the table of mappings
  Iterator itr = extensions.iterator();
  while (itr.hasNext())
  {
    String fileExtension = (String) itr.next();

    if (fileExtension != null && fileExtension.length() > 0)
    {
      FileEditorMapping mapping = getMappingFor("*." + fileExtension); //$NON-NLS-1$
      if (mapping == null)
      { // no mapping for that extension
        mapping = new FileEditorMapping(fileExtension);
        typeEditorMappings.putDefault(mappingKeyFor(mapping), mapping);
      }
      mapping.addEditor(editor);
      if (bDefault)
      {
        mapping.setDefaultEditor(editor);
      }
    }
  }

  // add it to the table of mappings
  itr = filenames.iterator();
  while (itr.hasNext())
  {
    String filename = (String) itr.next();

    if (filename != null && filename.length() > 0)
    {
      FileEditorMapping mapping = getMappingFor(filename);
      if (mapping == null)
      { // no mapping for that extension
        String name;
        String extension;
        int index = filename.indexOf('.');
        if (index < 0)
        {
          name = filename;
          extension = ""; //$NON-NLS-1$
        }
        else
        {
          name = filename.substring(0, index);
          extension = filename.substring(index + 1);
        }
        mapping = new FileEditorMapping(name, extension);
        typeEditorMappings.putDefault(mappingKeyFor(mapping), mapping);
      }
      mapping.addEditor(editor);
      if (bDefault)
      {
        mapping.setDefaultEditor(editor);
      }
    }
  }

  itr = contentTypeVector.iterator();
  while (itr.hasNext())
  {
    String contentTypeId = (String) itr.next();
    if (contentTypeId != null && contentTypeId.length() > 0)
    {
      IContentType contentType = Platform.getContentTypeManager().getContentType(contentTypeId);
      if (contentType != null)
      {
        IEditorDescriptor [] editorArray = (IEditorDescriptor[]) contentTypeToEditorMappings.get(contentType);
        if (editorArray == null)
        {
          editorArray = new IEditorDescriptor[]
          { editor};
          contentTypeToEditorMappings.put(contentType, editorArray);
        }
        else
        {
          IEditorDescriptor [] newArray = new IEditorDescriptor[editorArray.length + 1];
          if (bDefault)
          { // default editors go to the front of the line
            newArray[0] = editor;
            System.arraycopy(editorArray, 0, newArray, 1, editorArray.length);
          }
          else
          {
            newArray[editorArray.length] = editor;
            System.arraycopy(editorArray, 0, newArray, 0, editorArray.length);
          }
          contentTypeToEditorMappings.put(contentType, newArray);
        }
      }
    }
  }

  // Update editor map.
  mapIDtoEditor.put(editor.getId(), editor);
}

void EditorRegistry::AddExternalEditorsToEditorMap()
{
  IEditorDescriptor desc = null;

  // Add registered editors (may include external editors).
  FileEditorMapping maps[] = typeEditorMappings.allMappings();
  for (int i = 0; i < maps.length; i++)
  {
    FileEditorMapping map = maps[i];
    IEditorDescriptor[] descArray = map.getEditors();
    for (int n = 0; n < descArray.length; n++)
    {
      desc = descArray[n];
      mapIDtoEditor.put(desc.getId(), desc);
    }
  }
}

IEditorDescriptor::Pointer EditorRegistry::FindEditor(const std::string& id)
{
  Object desc = mapIDtoEditor.get(id);
  if (WorkbenchActivityHelper.restrictUseOf(desc))
  {
    return null;
  }
  return (IEditorDescriptor) desc;
}

IEditorDescriptor::Pointer EditorRegistry::GetDefaultEditor()
{
  // the default editor will always be the system external editor
  // this should never return null
  return findEditor(IEditorRegistry.SYSTEM_EXTERNAL_EDITOR_ID);
}

IEditorDescriptor::Pointer EditorRegistry::GetDefaultEditor(
    const std::string& filename)
{
  return getDefaultEditor(filename, guessAtContentType(filename));
}

std::vector<IEditorDescriptor::Pointer> EditorRegistry::GetEditors(
    const std::string& filename)
{
  return getEditors(filename, guessAtContentType(filename));
}

std::vector<IFileEditorMapping::Pointer> EditorRegistry::GetFileEditorMappings()
{
  FileEditorMapping[] array = typeEditorMappings.allMappings();
  final Collator collator = Collator.getInstance();
  Arrays.sort(array, new Comparator()
      {

        /* (non-Javadoc)
         * @see java.util.Comparator#compare(java.lang.Object, java.lang.Object)
         */
      public: int compare(Object o1, Object o2)
        {
          String s1 = ((FileEditorMapping) o1).getLabel();
          String s2 = ((FileEditorMapping) o2).getLabel();
          return collator.compare(s1, s2);
        }
      }
      );
      return array;
    }

    FileEditorMapping::Pointer EditorRegistry::GetMappingFor(
        const std::string& ext)
    {
      if (ext == null)
      {
        return null;
      }
      String key = mappingKeyFor(ext);
      return typeEditorMappings.get(key);
    }

    std::vector<FileEditorMapping::Pointer> EditorRegistry::GetMappingForFilename(
        const std::string& filename)
    {
      FileEditorMapping[] mapping = new FileEditorMapping[2];

      // Lookup on entire filename
      mapping[0] = getMappingFor(filename);

      // Lookup on filename's extension
      int index = filename.lastIndexOf('.');
      if (index > -1)
      {
        String extension = filename.substring(index);
        mapping[1] = getMappingFor("*" + extension); //$NON-NLS-1$
      }

      return mapping;
    }

    std::vector<IEditorDescriptor::Pointer> EditorRegistry::GetSortedEditorsFromOS()
    {
      List externalEditors = new ArrayList();
      Program[] programs = Program.getPrograms();

      for (int i = 0; i < programs.length; i++)
      {
        //1FPLRL2: ITPUI:WINNT - NOTEPAD editor cannot be launched
        //Some entries start with %SystemRoot%
        //For such cases just use the file name as they are generally
        //in directories which are on the path
        /*
         * if (fileName.charAt(0) == '%') { fileName = name + ".exe"; }
         */

        EditorDescriptor editor = new EditorDescriptor();
        editor.setOpenMode(EditorDescriptor.OPEN_EXTERNAL);
        editor.setProgram(programs[i]);

        // determine the program icon this editor would need (do not let it
        // be cached in the workbench registry)
        ImageDescriptor desc = new ExternalProgramImageDescriptor(
            programs[i]);
        editor.setImageDescriptor(desc);
        externalEditors.add(editor);
      }

      Object[] tempArray = sortEditors(externalEditors);
      IEditorDescriptor[] array = new IEditorDescriptor[externalEditors
      .size()];
      for (int i = 0; i < tempArray.length; i++)
      {
        array[i] = (IEditorDescriptor) tempArray[i];
      }
      return array;
    }

    std::vector<IEditorDescriptor::Pointer> EditorRegistry::GetSortedEditorsFromPlugins()
    {
      Collection descs = WorkbenchActivityHelper
      .restrictCollection(sortedEditorsFromPlugins, new ArrayList());
      return (IEditorDescriptor[]) descs.toArray(new IEditorDescriptor[descs
      .size()]);
    }

    void EditorRegistry::InitialIdToEditorMap(std::map<std::string, EditorDescriptor::Pointer>& map)
    {
      this->AddSystemEditors(map);
    }

    void EditorRegistry::AddSystemEditors(
        std::map<std::string, EditorDescriptor::Pointer>& map)
    {
      // there will always be a system external editor descriptor
      EditorDescriptor editor = new EditorDescriptor();
      editor.setID(IEditorRegistry.SYSTEM_EXTERNAL_EDITOR_ID);
      editor.setName(WorkbenchMessages.SystemEditorDescription_name);
      editor.setOpenMode(EditorDescriptor.OPEN_EXTERNAL);
      // @issue we need a real icon for this editor?
      map.put(IEditorRegistry.SYSTEM_EXTERNAL_EDITOR_ID, editor);

      // there may be a system in-place editor if supported by platform
      if (ComponentSupport.inPlaceEditorSupported())
      {
        editor = new EditorDescriptor();
        editor.setID(IEditorRegistry.SYSTEM_INPLACE_EDITOR_ID);
        editor.setName(WorkbenchMessages.SystemInPlaceDescription_name);
        editor.setOpenMode(EditorDescriptor.OPEN_INPLACE);
        // @issue we need a real icon for this editor?
        map.put(IEditorRegistry.SYSTEM_INPLACE_EDITOR_ID, editor);
      }

      EditorDescriptor emptyEditorDescriptor = new EditorDescriptor();
      emptyEditorDescriptor.setID(EMPTY_EDITOR_ID);
      emptyEditorDescriptor.setName("(Empty)"); //$NON-NLS-1$
      emptyEditorDescriptor
      .setImageDescriptor(WorkbenchImages
      .getImageDescriptor(IWorkbenchGraphicConstants.IMG_OBJ_ELEMENT));
      map.put(EMPTY_EDITOR_ID, emptyEditorDescriptor);
    }

    void EditorRegistry::InitializeFromStorage()
    {
      typeEditorMappings = new EditorMap();
      extensionImages = new HashMap();

      //Get editors from the registry
      EditorRegistryReader registryReader = new EditorRegistryReader();
      registryReader.addEditors(this);
      sortInternalEditors();
      rebuildInternalEditorMap();

      IPreferenceStore store = PlatformUI.getPreferenceStore();
      String defaultEditors = store
      .getString(IPreferenceConstants.DEFAULT_EDITORS);
      String chachedDefaultEditors = store
      .getString(IPreferenceConstants.DEFAULT_EDITORS_CACHE);

      //If defaults has changed load it afterwards so it overrides the users
      // associations.
      if (defaultEditors == null
          || defaultEditors.equals(chachedDefaultEditors))
      {
        setProductDefaults(defaultEditors);
        loadAssociations(); //get saved earlier state
      }
      else
      {
        loadAssociations(); //get saved earlier state
        setProductDefaults(defaultEditors);
        store.putValue(IPreferenceConstants.DEFAULT_EDITORS_CACHE,
            defaultEditors);
      }
      addExternalEditorsToEditorMap();
    }

    void EditorRegistry::SetProductDefaults(const std::string& defaultEditors)
    {
      if (defaultEditors == null || defaultEditors.length() == 0)
      {
        return;
      }

      StringTokenizer extEditors = new StringTokenizer(defaultEditors,
          new Character(IPreferenceConstants.SEPARATOR).toString());
      while (extEditors.hasMoreTokens())
      {
        String extEditor = extEditors.nextToken().trim();
        int index = extEditor.indexOf(':');
        if (extEditor.length() < 3 || index <= 0 || index
            >= (extEditor.length() - 1))
        {
          //Extension and id must have at least one char.
          WorkbenchPlugin
          .log("Error setting default editor. Could not parse '" + extEditor
              + "'. Default editors should be specified as '*.ext1:editorId1;*.ext2:editorId2'"); //$NON-NLS-1$ //$NON-NLS-2$
          return;
        }
        String ext = extEditor.substring(0, index).trim();
        String editorId = extEditor.substring(index + 1).trim();
        FileEditorMapping mapping = getMappingFor(ext);
        if (mapping == null)
        {
          WorkbenchPlugin
          .log("Error setting default editor. Could not find mapping for '"
              + ext + "'."); //$NON-NLS-1$ //$NON-NLS-2$
          continue;
        }
        EditorDescriptor editor = (EditorDescriptor) findEditor(editorId);
        if (editor == null)
        {
          WorkbenchPlugin
          .log("Error setting default editor. Could not find editor: '"
              + editorId + "'."); //$NON-NLS-1$ //$NON-NLS-2$
          continue;
        }
        mapping.setDefaultEditor(editor);
      }
    }

    bool EditorRegistry::ReadEditors(
        std::map<std::string, EditorDescriptor::Pointer>& editorTable)
    {
      //Get the workbench plugin's working directory
      IPath workbenchStatePath = WorkbenchPlugin.getDefault().getDataLocation();
      if (workbenchStatePath == null)
      {
        return false;
      }
      IPreferenceStore store = WorkbenchPlugin.getDefault()
      .getPreferenceStore();
      Reader reader = null;
      try
      {
        // Get the editors defined in the preferences store
        String xmlString = store.getString(IPreferenceConstants.EDITORS);
        if (xmlString == null || xmlString.length() == 0)
        {
          FileInputStream stream = new FileInputStream(workbenchStatePath
              .append(IWorkbenchConstants.EDITOR_FILE_NAME)
              .toOSString());
          reader = new BufferedReader(new InputStreamReader(stream,
                  "utf-8")); //$NON-NLS-1$
        }
        else
        {
          reader = new StringReader(xmlString);
        }
        XMLMemento memento = XMLMemento.createReadRoot(reader);
        EditorDescriptor editor;
        IMemento[] edMementos = memento
        .getChildren(IWorkbenchConstants.TAG_DESCRIPTOR);
        // Get the editors and validate each one
        for (int i = 0; i < edMementos.length; i++)
        {
          editor = new EditorDescriptor();
          boolean valid = editor.loadValues(edMementos[i]);
          if (!valid)
          {
            continue;
          }
          if (editor.getPluginID() != null)
          {
            //If the editor is from a plugin we use its ID to look it
            // up in the mapping of editors we
            //have obtained from plugins. This allows us to verify that
            // the editor is still valid
            //and allows us to get the editor description from the
            // mapping table which has
            //a valid config element field.
            EditorDescriptor validEditorDescritor = (EditorDescriptor) mapIDtoEditor
            .get(editor.getId());
            if (validEditorDescritor != null)
            {
              editorTable.put(validEditorDescritor.getId(),
                  validEditorDescritor);
            }
          }
          else
          { //This is either from a program or a user defined
            // editor
            ImageDescriptor descriptor;
            if (editor.getProgram() == null)
            {
              descriptor = new ProgramImageDescriptor(editor
                  .getFileName(), 0);
            }
            else
            {
              descriptor = new ExternalProgramImageDescriptor(editor
                  .getProgram());
            }
            editor.setImageDescriptor(descriptor);
            editorTable.put(editor.getId(), editor);
          }
        }
      }
      catch (IOException e)
      {
        try
        {
          if (reader != null)
          {
            reader.close();
          }
        }
        catch (IOException ex)
        {
          e.printStackTrace();
        }
        //Ignore this as the workbench may not yet have saved any state
        return false;
      }
      catch (WorkbenchException e)
      {
        ErrorDialog.openError((Shell) null, WorkbenchMessages.EditorRegistry_errorTitle,
            WorkbenchMessages.EditorRegistry_errorMessage,
            e.getStatus());
        return false;
      }

      return true;
    }

    void EditorRegistry::ReadResources(
        std::map<std::string, EditorDescriptor::Pointer>& editorTable,
        std::ostream& reader)
    {
      XMLMemento memento = XMLMemento.createReadRoot(reader);
      String versionString = memento.getString(IWorkbenchConstants.TAG_VERSION);
      boolean versionIs31 = "3.1".equals(versionString); //$NON-NLS-1$

      IMemento[] extMementos = memento
      .getChildren(IWorkbenchConstants.TAG_INFO);
      for (int i = 0; i < extMementos.length; i++)
      {
        String name = extMementos[i]
        .getString(IWorkbenchConstants.TAG_NAME);
        if (name == null)
        {
          name = "*"; //$NON-NLS-1$
        }
        String extension = extMementos[i]
        .getString(IWorkbenchConstants.TAG_EXTENSION);
        IMemento[] idMementos = extMementos[i]
        .getChildren(IWorkbenchConstants.TAG_EDITOR);
        String[] editorIDs = new String[idMementos.length];
        for (int j = 0; j < idMementos.length; j++)
        {
          editorIDs[j] = idMementos[j]
          .getString(IWorkbenchConstants.TAG_ID);
        }
        idMementos = extMementos[i]
        .getChildren(IWorkbenchConstants.TAG_DELETED_EDITOR);
        String[] deletedEditorIDs = new String[idMementos.length];
        for (int j = 0; j < idMementos.length; j++)
        {
          deletedEditorIDs[j] = idMementos[j]
          .getString(IWorkbenchConstants.TAG_ID);
        }
        FileEditorMapping mapping = getMappingFor(name + "." + extension); //$NON-NLS-1$
        if (mapping == null)
        {
          mapping = new FileEditorMapping(name, extension);
        }
        List editors = new ArrayList();
        for (int j = 0; j < editorIDs.length; j++)
        {
          if (editorIDs[j] != null)
          {
            EditorDescriptor editor = (EditorDescriptor) editorTable
            .get(editorIDs[j]);
            if (editor != null)
            {
              editors.add(editor);
            }
          }
        }
        List deletedEditors = new ArrayList();
        for (int j = 0; j < deletedEditorIDs.length; j++)
        {
          if (deletedEditorIDs[j] != null)
          {
            EditorDescriptor editor = (EditorDescriptor) editorTable
            .get(deletedEditorIDs[j]);
            if (editor != null)
            {
              deletedEditors.add(editor);
            }
          }
        }

        List defaultEditors = new ArrayList();

        if (versionIs31)
        { // parse the new format
          idMementos = extMementos[i]
          .getChildren(IWorkbenchConstants.TAG_DEFAULT_EDITOR);
          String[] defaultEditorIds = new String[idMementos.length];
          for (int j = 0; j < idMementos.length; j++)
          {
            defaultEditorIds[j] = idMementos[j]
            .getString(IWorkbenchConstants.TAG_ID);
          }
          for (int j = 0; j < defaultEditorIds.length; j++)
          {
            if (defaultEditorIds[j] != null)
            {
              EditorDescriptor editor = (EditorDescriptor) editorTable
              .get(defaultEditorIds[j]);
              if (editor != null)
              {
                defaultEditors.add(editor);
              }
            }
          }
        }
        else
        { // guess at pre 3.1 format defaults
          if (!editors.isEmpty())
          {
            EditorDescriptor editor = (EditorDescriptor) editors.get(0);
            if (editor != null)
            {
              defaultEditors.add(editor);
            }
          }
          defaultEditors.addAll(Arrays.asList(mapping.getDeclaredDefaultEditors()));
        }

        // Add any new editors that have already been read from the registry
        // which were not deleted.
        IEditorDescriptor[] editorsArray = mapping.getEditors();
        for (int j = 0; j < editorsArray.length; j++)
        {
          if (!contains(editors, editorsArray[j])
              && !deletedEditors.contains(editorsArray[j]))
          {
            editors.add(editorsArray[j]);
          }
        }
        // Map the editor(s) to the file type
        mapping.setEditorsList(editors);
        mapping.setDeletedEditorsList(deletedEditors);
        mapping.setDefaultEditors(defaultEditors);
        typeEditorMappings.put(mappingKeyFor(mapping), mapping);
      }
    }

    bool EditorRegistry::Contains(
        const std::vector<IEditorDescriptor::Pointer>& editorsArray,
        IEditorDescriptor::Pointer editorDescriptor)
    {
      IEditorDescriptor currentEditorDescriptor = null;
      Iterator i = editorsArray.iterator();
      while (i.hasNext())
      {
        currentEditorDescriptor = (IEditorDescriptor) i.next();
        if (currentEditorDescriptor.getId()
        .equals(editorDescriptor.getId()))
        {
          return true;
        }
      }
      return false;

    }

    bool EditorRegistry::ReadResources(
        std::map<std::string, EditorDescriptor::Pointer>& editorTable)
    {
      //Get the workbench plugin's working directory
      IPath workbenchStatePath = WorkbenchPlugin.getDefault().getDataLocation();
      // XXX: nobody cares about this return value
      if (workbenchStatePath == null)
      {
        return false;
      }
      IPreferenceStore store = WorkbenchPlugin.getDefault()
      .getPreferenceStore();
      Reader reader = null;
      try
      {
        // Get the resource types
        String xmlString = store.getString(IPreferenceConstants.RESOURCES);
        if (xmlString == null || xmlString.length() == 0)
        {
          FileInputStream stream = new FileInputStream(workbenchStatePath
              .append(IWorkbenchConstants.RESOURCE_TYPE_FILE_NAME)
              .toOSString());
          reader = new BufferedReader(new InputStreamReader(stream,
                  "utf-8")); //$NON-NLS-1$
        }
        else
        {
          reader = new StringReader(xmlString);
        }
        // Read the defined resources into the table
        readResources(editorTable, reader);
      }
      catch (IOException e)
      {
        try
        {
          if (reader != null)
          {
            reader.close();
          }
        }
        catch (IOException ex)
        {
          ex.printStackTrace();
        }
        MessageDialog.openError((Shell) null, WorkbenchMessages.EditorRegistry_errorTitle,
            WorkbenchMessages.EditorRegistry_errorMessage);
        return false;
      }
      catch (WorkbenchException e)
      {
        ErrorDialog.openError((Shell) null, WorkbenchMessages.EditorRegistry_errorTitle,
            WorkbenchMessages.EditorRegistry_errorMessage,
            e.getStatus());
        return false;
      }
      return true;

    }

    bool EditorRegistry::LoadAssociations()
    {
      Map editorTable = new HashMap();
      if (!readEditors(editorTable))
      {
        return false;
      }
      return readResources(editorTable);
    }

    std::string EditorRegistry::MappingKeyFor(const std::string& type)
    {
      // keep everyting lower case for case-sensitive platforms
      return type.toLowerCase();
    }

    std::string EditorRegistry::MappingKeyFor(FileEditorMapping::Pointer mapping)
    {
      return mappingKeyFor(mapping.getName() + (mapping.getExtension().length() == 0 ? "" : "." + mapping.getExtension())); //$NON-NLS-1$ //$NON-NLS-2$
    }

    void EditorRegistry::RebuildEditorMap()
    {
      rebuildInternalEditorMap();
      addExternalEditorsToEditorMap();
    }

    void EditorRegistry::RebuildInternalEditorMap()
    {
      Iterator itr = null;
      IEditorDescriptor desc = null;

      // Allocate a new map.
      mapIDtoEditor = initialIdToEditorMap(mapIDtoEditor.size());

      // Add plugin editors.
      itr = sortedEditorsFromPlugins.iterator();
      while (itr.hasNext())
      {
        desc = (IEditorDescriptor) itr.next();
        mapIDtoEditor.put(desc.getId(), desc);
      }
    }

    void EditorRegistry::SaveAssociations()
    {
      //Save the resource type descriptions
      List editors = new ArrayList();
      IPreferenceStore store = WorkbenchPlugin.getDefault()
      .getPreferenceStore();

      XMLMemento memento = XMLMemento
      .createWriteRoot(IWorkbenchConstants.TAG_EDITORS);
      memento.putString(IWorkbenchConstants.TAG_VERSION, "3.1"); //$NON-NLS-1$
      FileEditorMapping maps[] = typeEditorMappings.userMappings();
      for (int mapsIndex = 0; mapsIndex < maps.length; mapsIndex++)
      {
        FileEditorMapping type = maps[mapsIndex];
        IMemento editorMemento = memento
        .createChild(IWorkbenchConstants.TAG_INFO);
        editorMemento.putString(IWorkbenchConstants.TAG_NAME, type
        .getName());
        editorMemento.putString(IWorkbenchConstants.TAG_EXTENSION, type
        .getExtension());
        IEditorDescriptor[] editorArray = type.getEditors();
        for (int i = 0; i < editorArray.length; i++)
        {
          EditorDescriptor editor = (EditorDescriptor) editorArray[i];
          if (!editors.contains(editor))
          {
            editors.add(editor);
          }
          IMemento idMemento = editorMemento
          .createChild(IWorkbenchConstants.TAG_EDITOR);
          idMemento.putString(IWorkbenchConstants.TAG_ID, editorArray[i]
          .getId());
        }
        editorArray = type.getDeletedEditors();
        for (int i = 0; i < editorArray.length; i++)
        {
          EditorDescriptor editor = (EditorDescriptor) editorArray[i];
          if (!editors.contains(editor))
          {
            editors.add(editor);
          }
          IMemento idMemento = editorMemento
          .createChild(IWorkbenchConstants.TAG_DELETED_EDITOR);
          idMemento.putString(IWorkbenchConstants.TAG_ID, editorArray[i]
          .getId());
        }
        editorArray = type.getDeclaredDefaultEditors();
        for (int i = 0; i < editorArray.length; i++)
        {
          EditorDescriptor editor = (EditorDescriptor) editorArray[i];
          if (!editors.contains(editor))
          {
            editors.add(editor);
          }
          IMemento idMemento = editorMemento
          .createChild(IWorkbenchConstants.TAG_DEFAULT_EDITOR);
          idMemento.putString(IWorkbenchConstants.TAG_ID, editorArray[i]
          .getId());
        }
      }
      Writer writer = null;
      try
      {
        writer = new StringWriter();
        memento.save(writer);
        writer.close();
        store.setValue(IPreferenceConstants.RESOURCES, writer.toString());
      }
      catch (IOException e)
      {
        try
        {
          if (writer != null)
          {
            writer.close();
          }
        }
        catch (IOException ex)
        {
          ex.printStackTrace();
        }
        MessageDialog.openError((Shell) null, "Saving Problems", //$NON-NLS-1$
            "Unable to save resource associations."); //$NON-NLS-1$
        return;
      }

      memento = XMLMemento.createWriteRoot(IWorkbenchConstants.TAG_EDITORS);
      Iterator itr = editors.iterator();
      while (itr.hasNext())
      {
        EditorDescriptor editor = (EditorDescriptor) itr.next();
        IMemento editorMemento = memento
        .createChild(IWorkbenchConstants.TAG_DESCRIPTOR);
        editor.saveValues(editorMemento);
      }
      writer = null;
      try
      {
        writer = new StringWriter();
        memento.save(writer);
        writer.close();
        store.setValue(IPreferenceConstants.EDITORS, writer.toString());
      }
      catch (IOException e)
      {
        try
        {
          if (writer != null)
          {
            writer.close();
          }
        }
        catch (IOException ex)
        {
          ex.printStackTrace();
        }
        MessageDialog.openError((Shell) null,
            "Error", "Unable to save resource associations."); //$NON-NLS-1$ //$NON-NLS-2$
        return;
      }
    }

    void EditorRegistry::SetFileEditorMappings(
        const std::vector<FileEditorMapping::Pointer>& newResourceTypes)
    {
      typeEditorMappings = new EditorMap();
      for (int i = 0; i < newResourceTypes.length; i++)
      {
        FileEditorMapping mapping = newResourceTypes[i];
        typeEditorMappings.put(mappingKeyFor(mapping), mapping);
      }
      extensionImages = new HashMap();
      rebuildEditorMap();
      firePropertyChange(PROP_CONTENTS);
    }

    void EditorRegistry::SetDefaultEditor(const std::string& fileName,
        const std::string& editorId)
    {
      EditorDescriptor desc = (EditorDescriptor) findEditor(editorId);
      FileEditorMapping[] mapping = getMappingForFilename(fileName);
      if (mapping[0] != null)
      {
        mapping[0].setDefaultEditor(desc);
      }
      if (mapping[1] != null)
      {
        mapping[1].setDefaultEditor(desc);
      }
    }

    std::vector<IEditorDescriptor::Pointer> SortEditors(
        const std::vector<IEditorDescriptor>& unsortedList)
    {
      Object[] array = new Object[unsortedList.size()];
      unsortedList.toArray(array);

      Collections.sort(Arrays.asList(array), comparer);
      return array;
    }

    void EditorRegistry::SortInternalEditors()
    {
      Object[] array = sortEditors(sortedEditorsFromPlugins);
      sortedEditorsFromPlugins = new ArrayList();
      for (int i = 0; i < array.length; i++)
      {
        sortedEditorsFromPlugins.add(array[i]);
      }
    }

    static void EditorRegistry::EditorMap::PutDefault(const std::string& key,
        FileEditorMapping::Pointer value)
    {
      defaultMap.put(key, value);
    }

    void EditorRegistry::EditorMap::Put(const std::string& key,
        FileEditorMapping::Pointer value)
    {
      Object result = defaultMap.get(key);
      if (value.equals(result))
      {
        map.remove(key);
      }
      else
      {
        map.put(key, value);
      }
    }

    FileEditorMapping::Pointer EditorRegistry::EditorMap::Get(
        const std::string& key)
    {
      Object result = map.get(key);
      if (result == null)
      {
        result = defaultMap.get(key);
      }
      return (FileEditorMapping) result;
    }

    std::vector<FileEditorMapping::Pointer> EditorRegistry::EditorMap::AllMappings()
    {
      HashMap merge = (HashMap) defaultMap.clone();
      merge.putAll(map);
      Collection values = merge.values();
      FileEditorMapping result[] = new FileEditorMapping[values.size()];
      return (FileEditorMapping[]) values.toArray(result);
    }

    std::vector<FileEditorMapping::Pointer> EditorRegistry::EditorMap::UserMappings()
    {
      Collection values = map.values();
      FileEditorMapping result[] = new FileEditorMapping[values.size()];
      return (FileEditorMapping[]) values.toArray(result);
    }

    bool EditorRegistry::IsSystemInPlaceEditorAvailable(
        const std::string& filename)
    {
      return ComponentSupport.inPlaceEditorAvailable(filename);
    }

    bool EditorRegistry::IsSystemExternalEditorAvailable(
        const std::string& filename)
    {
      int nDot = filename.lastIndexOf('.');
      if (nDot >= 0)
      {
        String strName = filename.substring(nDot);
        return Program.findProgram(strName) != null;
      }
      return false;
    }

    void EditorRegistry::RemoveEditorFromMapping(
        std::map<std::string, FileEditorMapping::Pointer>& map,
        IEditorDescriptor::Pointer desc)
    {
      Iterator iter = map.values().iterator();
      FileEditorMapping mapping;
      IEditorDescriptor[] editors;
      while (iter.hasNext())
      {
        mapping = (FileEditorMapping) iter.next();
        editors = mapping.getEditors();
        for (int i = 0; i < editors.length; i++)
        {
          if (editors[i] == desc)
          {
            mapping.removeEditor((EditorDescriptor) editors[i]);
            break;
          }
        }
        if (editors.length <= 0)
        {
          map.remove(mapping);
          break;
        }
      }
    }

    IExtensionPoint* EditorRegistry::GetExtensionPointFilter()
    {
      return Platform.getExtensionRegistry().getExtensionPoint(PlatformUI.PLUGIN_ID,
          IWorkbenchRegistryConstants.PL_EDITOR);
    }

    std::vector<IFileEditorMapping::Pointer> EditorRegistry::GetUnifiedMappings()
    {
      IFileEditorMapping[] standardMappings = PlatformUI.getWorkbench()
      .getEditorRegistry().getFileEditorMappings();

      List allMappings = new ArrayList(Arrays.asList(standardMappings));
      // mock-up content type extensions into IFileEditorMappings
      IContentType [] contentTypes = Platform.getContentTypeManager().getAllContentTypes();
      for (int i = 0; i < contentTypes.length; i++)
      {
        IContentType type = contentTypes[i];
        String [] extensions = type.getFileSpecs(IContentType.FILE_EXTENSION_SPEC);
        for (int j = 0; j < extensions.length; j++)
        {
          String extension = extensions[j];
          boolean found = false;
          for (Iterator k = allMappings.iterator(); k.hasNext();)
          {
            IFileEditorMapping mapping = (IFileEditorMapping) k.next();
            if ("*".equals(mapping.getName())
                && extension.equals(mapping.getExtension()))
            { //$NON-NLS-1$
              found = true;
              break;
            }
          }
          if (!found)
          {
            MockMapping mockMapping = new MockMapping(type, "*", extension); //$NON-NLS-1$
            allMappings.add(mockMapping);
          }
        }

        String [] filenames = type.getFileSpecs(IContentType.FILE_NAME_SPEC);
        for (int j = 0; j < filenames.length; j++)
        {
          String wholename = filenames[j];
          int idx = wholename.indexOf('.');
          String name = idx == -1 ? wholename : wholename.substring(0, idx);
          String extension = idx == -1 ? "" : wholename.substring(idx + 1); //$NON-NLS-1$

          boolean found = false;
          for (Iterator k = allMappings.iterator(); k.hasNext();)
          {
            IFileEditorMapping mapping = (IFileEditorMapping) k.next();
            if (name.equals(mapping.getName())
                && extension.equals(mapping.getExtension()))
            {
              found = true;
              break;
            }
          }
          if (!found)
          {
            MockMapping mockMapping = new MockMapping(type, name, extension);
            allMappings.add(mockMapping);
          }
        }
      }

      return (IFileEditorMapping []) allMappings
      .toArray(new IFileEditorMapping [allMappings.size()]);
    }

    MockMapping::MockMapping(/*IContentType type,*/const std::string& name,
        const std::string& ext)
    {
      //this.contentType = type;
      this.filename = name;
      this.extension = ext;
    }

    IEditorDescriptor::Pointer MockMapping::GetDefaultEditor()
    {
      IEditorDescriptor[] candidates = ((EditorRegistry) PlatformUI
          .getWorkbench().getEditorRegistry())
      .getEditorsForContentType(contentType);
      if (candidates.length == 0
          || WorkbenchActivityHelper.restrictUseOf(candidates[0]))
      {
        return null;
      }
      return candidates[0];
    }

    std::vector<IEditorDescriptor::Pointer> MockMapping::GetEditors()
    {
      IEditorDescriptor[] editorsForContentType = ((EditorRegistry) PlatformUI
          .getWorkbench().getEditorRegistry())
      .getEditorsForContentType(contentType);
      return (IEditorDescriptor[]) WorkbenchActivityHelper
      .restrictArray(editorsForContentType);
    }

    std::vector<IEditorDescriptor::Pointer> MockMapping::GetDeletedEditors()
    {
      return new IEditorDescriptor[0];
    }

    std::string MockMapping::GetExtension()
    {
      return extension;
    }

    std::string MockMapping::GetLabel()
    {
      return filename + '.' + extension;
    }

    std::string MockMapping::GetName()
    {
      return filename;
    }

    bool MockMapping::operator==(const MockMapping* obj)
    {
      if (this == obj)
      {
        return true;
      }

      if (!(obj instanceof MockMapping))
      {
        return false;
      }

      MockMapping mapping = (MockMapping) obj;
      if (!this.filename.equals(mapping.filename))
      {
        return false;
      }

      if (!this.extension.equals(mapping.extension))
      {
        return false;
      }

      if (!Util.equals(this.getEditors(), mapping.getEditors()))
      {
        return false;
      }
      return Util.equals(this.getDeletedEditors(), mapping
          .getDeletedEditors());
    }

  }
