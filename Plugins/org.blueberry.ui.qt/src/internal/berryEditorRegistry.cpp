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

#include "berryEditorRegistry.h"

#include "berryWorkbenchPlugin.h"
#include "berryEditorRegistryReader.h"
#include "berryWorkbenchRegistryConstants.h"

namespace berry
{

const QString EditorRegistry::EMPTY_EDITOR_ID =
    "org.blueberry.ui.internal.emptyEditorTab";

QHash<QString, FileEditorMapping::Pointer>
    EditorRegistry::EditorMap::defaultMap;

QHash<QString, FileEditorMapping::Pointer> EditorRegistry::EditorMap::map;

EditorRegistry::RelatedRegistry::RelatedRegistry(EditorRegistry* reg) :
  editorRegistry(reg)
{

}

QList<IEditorDescriptor::Pointer> EditorRegistry::RelatedRegistry::GetRelatedObjects(
    const QString& fileName)
{
  IFileEditorMapping::Pointer mapping = editorRegistry->GetMappingFor(fileName);
  if (mapping.IsNull())
  {
    return QList<IEditorDescriptor::Pointer>();
  }

  return mapping->GetEditors();
}

EditorRegistry::EditorRegistry() :
  relatedRegistry(this)
{
  this->InitialIdToEditorMap(mapIDtoEditor);
  this->InitializeFromStorage();
  //IExtensionTracker tracker = PlatformUI.getWorkbench().getExtensionTracker();
  //tracker.registerHandler(this,
  //    ExtensionTracker.createExtensionPointFilter(getExtensionPointFilter()));
}

void EditorRegistry::AddEditorFromPlugin(EditorDescriptor::Pointer editor,
    const QList<QString>& extensions,
    const QList<QString>& filenames,
    const QList<QString>&  /*contentTypeVector*/, bool bDefault)
{

  //PlatformUI.getWorkbench().getExtensionTracker().registerObject(editor.getConfigurationElement().getDeclaringExtension(), editor, IExtensionTracker.REF_WEAK);
  // record it in our quick reference list
  sortedEditorsFromPlugins.push_back(editor);

  // add it to the table of mappings
  for (QList<QString>::const_iterator itr = extensions.begin(); itr
      != extensions.end(); ++itr)
  {
    QString fileExtension = *itr;

    if (!fileExtension.isEmpty())
    {
      FileEditorMapping::Pointer mapping = this->GetMappingFor("*." + fileExtension);
      if (mapping.IsNull())
      { // no mapping for that extension
        mapping = new FileEditorMapping(fileExtension);
        typeEditorMappings.PutDefault(this->MappingKeyFor(mapping), mapping);
      }
      mapping->AddEditor(editor);
      if (bDefault)
      {
        mapping->SetDefaultEditor(editor);
      }
    }
  }

  // add it to the table of mappings
  for (QList<QString>::const_iterator itr = filenames.begin(); itr
      != filenames.end(); ++itr)
  {
    QString filename = *itr;

    if (!filename.isEmpty())
    {
      FileEditorMapping::Pointer mapping = this->GetMappingFor(filename);
      if (mapping.IsNull())
      { // no mapping for that extension
        QString name;
        QString extension;
        int index = filename.indexOf('.');
        if (index == -1)
        {
          name = filename;
          extension = "";
        }
        else
        {
          name = filename.left(index);
          extension = filename.mid(index + 1);
        }
        mapping = new FileEditorMapping(name, extension);
        typeEditorMappings.PutDefault(this->MappingKeyFor(mapping), mapping);
      }
      mapping->AddEditor(editor);
      if (bDefault)
      {
        mapping->SetDefaultEditor(editor);
      }
    }
  }

  //  for (QList<QString>::const_iterator itr = contentTypeVector.begin();
  //         itr != contentTypeVector.end(); ++itr)
  //  {
  //    QString contentTypeId = *itr;
  //    if (!contentTypeId.empty())
  //    {
  //      IContentType contentType = Platform.getContentTypeManager().getContentType(contentTypeId);
  //      if (contentType != null)
  //      {
  //        IEditorDescriptor [] editorArray = (IEditorDescriptor[]) contentTypeToEditorMappings.get(contentType);
  //        if (editorArray == null)
  //        {
  //          editorArray = new IEditorDescriptor[]
  //          { editor};
  //          contentTypeToEditorMappings.put(contentType, editorArray);
  //        }
  //        else
  //        {
  //          IEditorDescriptor [] newArray = new IEditorDescriptor[editorArray.length + 1];
  //          if (bDefault)
  //          { // default editors go to the front of the line
  //            newArray[0] = editor;
  //            System.arraycopy(editorArray, 0, newArray, 1, editorArray.length);
  //          }
  //          else
  //          {
  //            newArray[editorArray.length] = editor;
  //            System.arraycopy(editorArray, 0, newArray, 0, editorArray.length);
  //          }
  //          contentTypeToEditorMappings.put(contentType, newArray);
  //        }
  //      }
  //    }
  //  }

  // Update editor map.
  mapIDtoEditor[editor->GetId()] = editor;
}

void EditorRegistry::AddExternalEditorsToEditorMap()
{
  // Add registered editors (may include external editors).
  QList<FileEditorMapping::Pointer> maps =
      typeEditorMappings.AllMappings();
  for (int i = 0; i < maps.size(); ++i)
  {
    FileEditorMapping::Pointer map = maps[i];
    QList<IEditorDescriptor::Pointer> descArray = map->GetEditors();
    for (QList<IEditorDescriptor::Pointer>::iterator itr =
        descArray.begin(); itr != descArray.end(); ++itr)
    {
      mapIDtoEditor[(*itr)->GetId()] = itr->Cast<EditorDescriptor> ();
    }
  }
}

IEditorDescriptor::Pointer EditorRegistry::FindEditor(const QString& id)
{
  return mapIDtoEditor[id];
}

IEditorDescriptor::Pointer EditorRegistry::GetDefaultEditor()
{
  // the default editor will always be the system external editor
  // this should never return null
  return this->FindEditor(IEditorRegistry::SYSTEM_EXTERNAL_EDITOR_ID);
}

IEditorDescriptor::Pointer EditorRegistry::GetDefaultEditor(
    const QString& fileName)
{
  //return this->GetDefaultEditor(filename, guessAtContentType(filename));
  return this->GetEditorForContentType(fileName /*, contentType*/);
}

IEditorDescriptor::Pointer EditorRegistry::GetEditorForContentType(
    const QString& filename
/*IContentType contentType*/)
{
  IEditorDescriptor::Pointer desc;
  ;
  QList<IEditorDescriptor::Pointer> contentTypeResults =
      this->FindRelatedObjects(/*contentType,*/filename, relatedRegistry);
  if (contentTypeResults.size() > 0)
  {
    desc = contentTypeResults.front();
  }
  return desc;
}

QList<IEditorDescriptor::Pointer> EditorRegistry::FindRelatedObjects(
    /*IContentType type,*/const QString& fileName,
    RelatedRegistry&  /*registry*/)
{
  QList<IEditorDescriptor::Pointer> allRelated;
  QList<IEditorDescriptor::Pointer> nonDefaultFileEditors;
  QList<IEditorDescriptor::Pointer> related;

  if (!fileName.isEmpty())
  {
    FileEditorMapping::Pointer mapping = this->GetMappingFor(fileName);
    if (!mapping.IsNull())
    {
      // backwards compatibility - add editors flagged as "default"
      related = mapping->GetDeclaredDefaultEditors();
      for (QList<IEditorDescriptor::Pointer>::iterator itr =
          related.begin(); itr != related.end(); ++itr)
      {
        // we don't want to return duplicates
        if (std::find(allRelated.begin(), allRelated.end(), *itr)
            == allRelated.end())
        {
          allRelated.push_back(*itr);
        }
      }

      // add all filename editors to the nonDefaultList
      // we'll later try to add them all after content types are resolved
      // duplicates (ie: default editors) will be ignored
      QList<IEditorDescriptor::Pointer> tmpList = mapping->GetEditors();
      nonDefaultFileEditors.append(tmpList);
    }

    int index = fileName.indexOf('.');
    if (index != -1)
    {
      QString extension = "*" + fileName.mid(index);
      mapping = this->GetMappingFor(extension);
      if (!mapping.IsNull())
      {
        related = mapping->GetDeclaredDefaultEditors();
        for (QList<IEditorDescriptor::Pointer>::iterator itr =
            related.begin(); itr != related.end(); ++itr)
        {
          // we don't want to return duplicates
          if (std::find(allRelated.begin(), allRelated.end(), *itr)
              == allRelated.end())
          {
            allRelated.push_back(*itr);
          }
        }
        QList<IEditorDescriptor::Pointer> tmpList = mapping->GetEditors();
        nonDefaultFileEditors.append(tmpList);
      }
    }
  }

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

  // add all non-default editors to the list
  for (QList<IEditorDescriptor::Pointer>::iterator i =
      nonDefaultFileEditors.begin(); i != nonDefaultFileEditors.end(); ++i)
  {
    IEditorDescriptor::Pointer editor = *i;
    if (std::find(allRelated.begin(), allRelated.end(), editor)
        == allRelated.end())
    {
      allRelated.push_back(editor);
    }
  }

  return allRelated;
}

QList<IEditorDescriptor::Pointer> EditorRegistry::GetEditors(
    const QString& filename)
{
  //return getEditors(filename, guessAtContentType(filename));
  return this->FindRelatedObjects(/*contentType,*/filename, relatedRegistry);
}

QList<IFileEditorMapping::Pointer> EditorRegistry::GetFileEditorMappings()
{
  QList<FileEditorMapping::Pointer>
      array(typeEditorMappings.AllMappings());
  std::sort(array.begin(), array.end(), CmpFileEditorMapping());

  QList<IFileEditorMapping::Pointer> result;
  for (QList<FileEditorMapping::Pointer>::iterator itr = array.begin(); itr
      != array.end(); ++itr)
  {
    result.push_back(itr->Cast<IFileEditorMapping> ());
  }
  return result;
}

FileEditorMapping::Pointer EditorRegistry::GetMappingFor(const QString& ext)
{
  QString key = this->MappingKeyFor(ext);
  return typeEditorMappings.Get(key);
}

QList<FileEditorMapping::Pointer> EditorRegistry::GetMappingForFilename(
    const QString& filename)
{
  QList<FileEditorMapping::Pointer> mapping;

  // Lookup on entire filename
  mapping[0] = this->GetMappingFor(filename);

  // Lookup on filename's extension
  int index = filename.indexOf('.');
  if (index != -1)
  {
    QString extension = filename.mid(index);
    mapping[1] = this->GetMappingFor("*" + extension);
  }

  return mapping;
}

//    QList<IEditorDescriptor::Pointer> EditorRegistry::GetSortedEditorsFromOS()
//    {
//      List externalEditors = new ArrayList();
//      Program[] programs = Program.getPrograms();
//
//      for (int i = 0; i < programs.length; i++)
//      {
//        //1FPLRL2: ITPUI:WINNT - NOTEPAD editor cannot be launched
//        //Some entries start with %SystemRoot%
//        //For such cases just use the file name as they are generally
//        //in directories which are on the path
//        /*
//         * if (fileName.charAt(0) == '%') { fileName = name + ".exe"; }
//         */
//
//        EditorDescriptor editor = new EditorDescriptor();
//        editor.setOpenMode(EditorDescriptor.OPEN_EXTERNAL);
//        editor.setProgram(programs[i]);
//
//        // determine the program icon this editor would need (do not let it
//        // be cached in the workbench registry)
//        ImageDescriptor desc = new ExternalProgramImageDescriptor(
//            programs[i]);
//        editor.setImageDescriptor(desc);
//        externalEditors.add(editor);
//      }
//
//      Object[] tempArray = sortEditors(externalEditors);
//      IEditorDescriptor[] array = new IEditorDescriptor[externalEditors
//      .size()];
//      for (int i = 0; i < tempArray.length; i++)
//      {
//        array[i] = (IEditorDescriptor) tempArray[i];
//      }
//      return array;
//    }

QList<IEditorDescriptor::Pointer> EditorRegistry::GetSortedEditorsFromPlugins()
{
  QList<IEditorDescriptor::Pointer> result;
  for (QList<EditorDescriptor::Pointer>::iterator itr =
      sortedEditorsFromPlugins.begin(); itr != sortedEditorsFromPlugins.end(); ++itr)
  {
    result.push_back((*itr).Cast<IEditorDescriptor> ());
  }
  return result;
}

void EditorRegistry::InitialIdToEditorMap(
    QHash<QString, EditorDescriptor::Pointer>& map)
{
  this->AddSystemEditors(map);
}

void EditorRegistry::AddSystemEditors(
    QHash<QString, EditorDescriptor::Pointer>& map)
{
  // there will always be a system external editor descriptor
  EditorDescriptor::Pointer editor(new EditorDescriptor());
  editor->SetID(IEditorRegistry::SYSTEM_EXTERNAL_EDITOR_ID);
  editor->SetName("System Editor");
  editor->SetOpenMode(EditorDescriptor::OPEN_EXTERNAL);
  // @issue we need a real icon for this editor?
  map[IEditorRegistry::SYSTEM_EXTERNAL_EDITOR_ID] = editor;

  // there may be a system in-place editor if supported by platform
  //      if (ComponentSupport.inPlaceEditorSupported())
  //      {
  //        editor = new EditorDescriptor();
  //        editor.setID(IEditorRegistry.SYSTEM_INPLACE_EDITOR_ID);
  //        editor.setName(WorkbenchMessages.SystemInPlaceDescription_name);
  //        editor.setOpenMode(EditorDescriptor.OPEN_INPLACE);
  //        // @issue we need a real icon for this editor?
  //        map.put(IEditorRegistry.SYSTEM_INPLACE_EDITOR_ID, editor);
  //      }

  EditorDescriptor::Pointer emptyEditorDescriptor(new EditorDescriptor());
  emptyEditorDescriptor->SetID(EMPTY_EDITOR_ID);
  emptyEditorDescriptor->SetName("(Empty)"); //$NON-NLS-1$
  //emptyEditorDescriptor.setImageDescriptor(WorkbenchImages
  //.getImageDescriptor(IWorkbenchGraphicConstants.IMG_OBJ_ELEMENT));
  map[EMPTY_EDITOR_ID] = emptyEditorDescriptor;
}

void EditorRegistry::InitializeFromStorage()
{

  //Get editors from the registry
  EditorRegistryReader registryReader;
  registryReader.AddEditors(this);
  this->SortInternalEditors();
  this->RebuildInternalEditorMap();

  //      IPreferenceStore store = PlatformUI.getPreferenceStore();
  //      String defaultEditors = store
  //      .getString(IPreferenceConstants.DEFAULT_EDITORS);
  //      String chachedDefaultEditors = store
  //      .getString(IPreferenceConstants.DEFAULT_EDITORS_CACHE);

  //If defaults has changed load it afterwards so it overrides the users
  // associations.
  //if (defaultEditors == null
  //    || defaultEditors.equals(chachedDefaultEditors))
  //{
  this->SetProductDefaults("");//defaultEditors);
  this->LoadAssociations(); //get saved earlier state
  //      }
  //      else
  //      {
  //        loadAssociations(); //get saved earlier state
  //        setProductDefaults(defaultEditors);
  //        store.putValue(IPreferenceConstants.DEFAULT_EDITORS_CACHE,
  //            defaultEditors);
  //      }
  this->AddExternalEditorsToEditorMap();
}

void EditorRegistry::SetProductDefaults(const QString& defaultEditors)
{
  if (defaultEditors.isEmpty())
  {
    return;
  }

  //      Poco::StringTokenizer extEditors(defaultEditors,
  //          IPreferenceConstants::SEPARATOR, Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
  //      while (extEditors.hasMoreTokens())
  //      {
  //        String extEditor = extEditors.nextToken().trim();
  //        int index = extEditor.indexOf(':');
  //        if (extEditor.length() < 3 || index <= 0 || index
  //            >= (extEditor.length() - 1))
  //        {
  //          //Extension and id must have at least one char.
  //          WorkbenchPlugin
  //          .log("Error setting default editor. Could not parse '" + extEditor
  //              + "'. Default editors should be specified as '*.ext1:editorId1;*.ext2:editorId2'"); //$NON-NLS-1$ //$NON-NLS-2$
  //          return;
  //        }
  //        String ext = extEditor.substring(0, index).trim();
  //        String editorId = extEditor.substring(index + 1).trim();
  //        FileEditorMapping mapping = getMappingFor(ext);
  //        if (mapping == null)
  //        {
  //          WorkbenchPlugin
  //          .log("Error setting default editor. Could not find mapping for '"
  //              + ext + "'."); //$NON-NLS-1$ //$NON-NLS-2$
  //          continue;
  //        }
  //        EditorDescriptor editor = (EditorDescriptor) findEditor(editorId);
  //        if (editor == null)
  //        {
  //          WorkbenchPlugin
  //          .log("Error setting default editor. Could not find editor: '"
  //              + editorId + "'."); //$NON-NLS-1$ //$NON-NLS-2$
  //          continue;
  //        }
  //        mapping.setDefaultEditor(editor);
  //      }
}

bool EditorRegistry::ReadEditors(
    QHash<QString, EditorDescriptor::Pointer>&  /*editorTable*/)
{
  //Get the workbench plugin's working directory
  QString workbenchStatePath = WorkbenchPlugin::GetDefault()->GetDataLocation();
  if (workbenchStatePath.isNull())
  {
    return false;
  }
  //      IPreferenceStore store = WorkbenchPlugin.getDefault()
  //      .getPreferenceStore();
  //      Reader reader = null;
  //      try
  //      {
  //        // Get the editors defined in the preferences store
  //        String xmlString = store.getString(IPreferenceConstants.EDITORS);
  //        if (xmlString == null || xmlString.length() == 0)
  //        {
  //          FileInputStream stream = new FileInputStream(workbenchStatePath
  //              .append(IWorkbenchConstants.EDITOR_FILE_NAME)
  //              .toOSString());
  //          reader = new BufferedReader(new InputStreamReader(stream,
  //                  "utf-8")); //$NON-NLS-1$
  //        }
  //        else
  //        {
  //          reader = new StringReader(xmlString);
  //        }
  //        XMLMemento memento = XMLMemento.createReadRoot(reader);
  //        EditorDescriptor editor;
  //        IMemento[] edMementos = memento
  //        .getChildren(IWorkbenchConstants.TAG_DESCRIPTOR);
  //        // Get the editors and validate each one
  //        for (int i = 0; i < edMementos.length; i++)
  //        {
  //          editor = new EditorDescriptor();
  //          boolean valid = editor.loadValues(edMementos[i]);
  //          if (!valid)
  //          {
  //            continue;
  //          }
  //          if (editor.getPluginID() != null)
  //          {
  //            //If the editor is from a plugin we use its ID to look it
  //            // up in the mapping of editors we
  //            //have obtained from plugins. This allows us to verify that
  //            // the editor is still valid
  //            //and allows us to get the editor description from the
  //            // mapping table which has
  //            //a valid config element field.
  //            EditorDescriptor validEditorDescritor = (EditorDescriptor) mapIDtoEditor
  //            .get(editor.getId());
  //            if (validEditorDescritor != null)
  //            {
  //              editorTable.put(validEditorDescritor.getId(),
  //                  validEditorDescritor);
  //            }
  //          }
  //          else
  //          { //This is either from a program or a user defined
  //            // editor
  //            ImageDescriptor descriptor;
  //            if (editor.getProgram() == null)
  //            {
  //              descriptor = new ProgramImageDescriptor(editor
  //                  .getFileName(), 0);
  //            }
  //            else
  //            {
  //              descriptor = new ExternalProgramImageDescriptor(editor
  //                  .getProgram());
  //            }
  //            editor.setImageDescriptor(descriptor);
  //            editorTable.put(editor.getId(), editor);
  //          }
  //        }
  //      }
  //      catch (IOException e)
  //      {
  //        try
  //        {
  //          if (reader != null)
  //          {
  //            reader.close();
  //          }
  //        }
  //        catch (IOException ex)
  //        {
  //          e.printStackTrace();
  //        }
  //        //Ignore this as the workbench may not yet have saved any state
  //        return false;
  //      }
  //      catch (WorkbenchException e)
  //      {
  //        ErrorDialog.openError((Shell) null, WorkbenchMessages.EditorRegistry_errorTitle,
  //            WorkbenchMessages.EditorRegistry_errorMessage,
  //            e.getStatus());
  //        return false;
  //      }

  return true;
}

void EditorRegistry::ReadResources(
    QHash<QString, EditorDescriptor::Pointer>& /*editorTable*/,
    std::ostream&  /*reader*/)
{
  //      XMLMemento memento = XMLMemento.createReadRoot(reader);
  //      String versionString = memento.getString(IWorkbenchConstants.TAG_VERSION);
  //      boolean versionIs31 = "3.1".equals(versionString); //$NON-NLS-1$
  //
  //      IMemento[] extMementos = memento
  //      .getChildren(IWorkbenchConstants.TAG_INFO);
  //      for (int i = 0; i < extMementos.length; i++)
  //      {
  //        String name = extMementos[i]
  //        .getString(IWorkbenchConstants.TAG_NAME);
  //        if (name == null)
  //        {
  //          name = "*"; //$NON-NLS-1$
  //        }
  //        String extension = extMementos[i]
  //        .getString(IWorkbenchConstants.TAG_EXTENSION);
  //        IMemento[] idMementos = extMementos[i]
  //        .getChildren(IWorkbenchConstants.TAG_EDITOR);
  //        String[] editorIDs = new String[idMementos.length];
  //        for (int j = 0; j < idMementos.length; j++)
  //        {
  //          editorIDs[j] = idMementos[j]
  //          .getString(IWorkbenchConstants.TAG_ID);
  //        }
  //        idMementos = extMementos[i]
  //        .getChildren(IWorkbenchConstants.TAG_DELETED_EDITOR);
  //        String[] deletedEditorIDs = new String[idMementos.length];
  //        for (int j = 0; j < idMementos.length; j++)
  //        {
  //          deletedEditorIDs[j] = idMementos[j]
  //          .getString(IWorkbenchConstants.TAG_ID);
  //        }
  //        FileEditorMapping mapping = getMappingFor(name + "." + extension); //$NON-NLS-1$
  //        if (mapping == null)
  //        {
  //          mapping = new FileEditorMapping(name, extension);
  //        }
  //        List editors = new ArrayList();
  //        for (int j = 0; j < editorIDs.length; j++)
  //        {
  //          if (editorIDs[j] != null)
  //          {
  //            EditorDescriptor editor = (EditorDescriptor) editorTable
  //            .get(editorIDs[j]);
  //            if (editor != null)
  //            {
  //              editors.add(editor);
  //            }
  //          }
  //        }
  //        List deletedEditors = new ArrayList();
  //        for (int j = 0; j < deletedEditorIDs.length; j++)
  //        {
  //          if (deletedEditorIDs[j] != null)
  //          {
  //            EditorDescriptor editor = (EditorDescriptor) editorTable
  //            .get(deletedEditorIDs[j]);
  //            if (editor != null)
  //            {
  //              deletedEditors.add(editor);
  //            }
  //          }
  //        }
  //
  //        List defaultEditors = new ArrayList();
  //
  //        if (versionIs31)
  //        { // parse the new format
  //          idMementos = extMementos[i]
  //          .getChildren(IWorkbenchConstants.TAG_DEFAULT_EDITOR);
  //          String[] defaultEditorIds = new String[idMementos.length];
  //          for (int j = 0; j < idMementos.length; j++)
  //          {
  //            defaultEditorIds[j] = idMementos[j]
  //            .getString(IWorkbenchConstants.TAG_ID);
  //          }
  //          for (int j = 0; j < defaultEditorIds.length; j++)
  //          {
  //            if (defaultEditorIds[j] != null)
  //            {
  //              EditorDescriptor editor = (EditorDescriptor) editorTable
  //              .get(defaultEditorIds[j]);
  //              if (editor != null)
  //              {
  //                defaultEditors.add(editor);
  //              }
  //            }
  //          }
  //        }
  //        else
  //        { // guess at pre 3.1 format defaults
  //          if (!editors.isEmpty())
  //          {
  //            EditorDescriptor editor = (EditorDescriptor) editors.get(0);
  //            if (editor != null)
  //            {
  //              defaultEditors.add(editor);
  //            }
  //          }
  //          defaultEditors.addAll(Arrays.asList(mapping.getDeclaredDefaultEditors()));
  //        }
  //
  //        // Add any new editors that have already been read from the registry
  //        // which were not deleted.
  //        IEditorDescriptor[] editorsArray = mapping.getEditors();
  //        for (int j = 0; j < editorsArray.length; j++)
  //        {
  //          if (!contains(editors, editorsArray[j])
  //              && !deletedEditors.contains(editorsArray[j]))
  //          {
  //            editors.add(editorsArray[j]);
  //          }
  //        }
  //        // Map the editor(s) to the file type
  //        mapping.setEditorsList(editors);
  //        mapping.setDeletedEditorsList(deletedEditors);
  //        mapping.setDefaultEditors(defaultEditors);
  //        typeEditorMappings.put(mappingKeyFor(mapping), mapping);
  //      }
}

bool EditorRegistry::Contains(
    const QList<IEditorDescriptor::Pointer>& editorsArray,
    IEditorDescriptor::Pointer editorDescriptor)
{
  IEditorDescriptor::Pointer currentEditorDescriptor;
  for (QList<IEditorDescriptor::Pointer>::const_iterator i =
      editorsArray.begin(); i != editorsArray.end(); ++i)
  {
    currentEditorDescriptor = *i;
    if (currentEditorDescriptor->GetId() == editorDescriptor->GetId())
    {
      return true;
    }
  }
  return false;

}

bool EditorRegistry::ReadResources(
    QHash<QString, EditorDescriptor::Pointer>&  /*editorTable*/)
{
  //Get the workbench plugin's working directory
  QString workbenchStatePath = WorkbenchPlugin::GetDefault()->GetDataLocation();
  // XXX: nobody cares about this return value
  if (workbenchStatePath.isNull())
  {
    return false;
  }
  //      IPreferenceStore store = WorkbenchPlugin.getDefault()
  //      .getPreferenceStore();
  //      Reader reader = null;
  //      try
  //      {
  //        // Get the resource types
  //        String xmlString = store.getString(IPreferenceConstants.RESOURCES);
  //        if (xmlString == null || xmlString.length() == 0)
  //        {
  //          FileInputStream stream = new FileInputStream(workbenchStatePath
  //              .append(IWorkbenchConstants.RESOURCE_TYPE_FILE_NAME)
  //              .toOSString());
  //          reader = new BufferedReader(new InputStreamReader(stream,
  //                  "utf-8")); //$NON-NLS-1$
  //        }
  //        else
  //        {
  //          reader = new StringReader(xmlString);
  //        }
  //        // Read the defined resources into the table
  //        readResources(editorTable, reader);
  //      }
  //      catch (IOException e)
  //      {
  //        try
  //        {
  //          if (reader != null)
  //          {
  //            reader.close();
  //          }
  //        }
  //        catch (IOException ex)
  //        {
  //          ex.printStackTrace();
  //        }
  //        MessageDialog.openError((Shell) null, WorkbenchMessages.EditorRegistry_errorTitle,
  //            WorkbenchMessages.EditorRegistry_errorMessage);
  //        return false;
  //      }
  //      catch (WorkbenchException e)
  //      {
  //        ErrorDialog.openError((Shell) null, WorkbenchMessages.EditorRegistry_errorTitle,
  //            WorkbenchMessages.EditorRegistry_errorMessage,
  //            e.getStatus());
  //        return false;
  //      }
  return true;

}

bool EditorRegistry::LoadAssociations()
{
  QHash<QString, EditorDescriptor::Pointer> editorTable;
  if (!this->ReadEditors(editorTable))
  {
    return false;
  }
  return this->ReadResources(editorTable);
}

QString EditorRegistry::MappingKeyFor(const QString& type)
{
  // keep everyting lower case for case-sensitive platforms
  return type.toLower();
}

QString EditorRegistry::MappingKeyFor(FileEditorMapping::Pointer mapping)
{
  return this->MappingKeyFor(mapping->GetName()
      + (mapping->GetExtension().size() == 0 ? "" : "."
          + mapping->GetExtension())); //$NON-NLS-1$ //$NON-NLS-2$
}

void EditorRegistry::RebuildEditorMap()
{
  this->RebuildInternalEditorMap();
  this->AddExternalEditorsToEditorMap();
}

void EditorRegistry::RebuildInternalEditorMap()
{
  EditorDescriptor::Pointer desc;

  // Allocate a new map.
  mapIDtoEditor.clear();
  this->InitialIdToEditorMap(mapIDtoEditor);

  // Add plugin editors.
  for (QList<EditorDescriptor::Pointer>::iterator itr =
      sortedEditorsFromPlugins.begin(); itr != sortedEditorsFromPlugins.end(); ++itr)
  {
    desc = *itr;
    mapIDtoEditor[desc->GetId()] = desc;
  }
}

void EditorRegistry::SaveAssociations()
{
  //Save the resource type descriptions
  //      List editors = new ArrayList();
  //      IPreferenceStore store = WorkbenchPlugin.getDefault()
  //      .getPreferenceStore();
  //
  //      XMLMemento memento = XMLMemento
  //      .createWriteRoot(IWorkbenchConstants.TAG_EDITORS);
  //      memento.putString(IWorkbenchConstants.TAG_VERSION, "3.1"); //$NON-NLS-1$
  //      FileEditorMapping maps[] = typeEditorMappings.userMappings();
  //      for (int mapsIndex = 0; mapsIndex < maps.length; mapsIndex++)
  //      {
  //        FileEditorMapping type = maps[mapsIndex];
  //        IMemento editorMemento = memento
  //        .createChild(IWorkbenchConstants.TAG_INFO);
  //        editorMemento.putString(IWorkbenchConstants.TAG_NAME, type
  //        .getName());
  //        editorMemento.putString(IWorkbenchConstants.TAG_EXTENSION, type
  //        .getExtension());
  //        IEditorDescriptor[] editorArray = type.getEditors();
  //        for (int i = 0; i < editorArray.length; i++)
  //        {
  //          EditorDescriptor editor = (EditorDescriptor) editorArray[i];
  //          if (!editors.contains(editor))
  //          {
  //            editors.add(editor);
  //          }
  //          IMemento idMemento = editorMemento
  //          .createChild(IWorkbenchConstants.TAG_EDITOR);
  //          idMemento.putString(IWorkbenchConstants.TAG_ID, editorArray[i]
  //          .getId());
  //        }
  //        editorArray = type.getDeletedEditors();
  //        for (int i = 0; i < editorArray.length; i++)
  //        {
  //          EditorDescriptor editor = (EditorDescriptor) editorArray[i];
  //          if (!editors.contains(editor))
  //          {
  //            editors.add(editor);
  //          }
  //          IMemento idMemento = editorMemento
  //          .createChild(IWorkbenchConstants.TAG_DELETED_EDITOR);
  //          idMemento.putString(IWorkbenchConstants.TAG_ID, editorArray[i]
  //          .getId());
  //        }
  //        editorArray = type.getDeclaredDefaultEditors();
  //        for (int i = 0; i < editorArray.length; i++)
  //        {
  //          EditorDescriptor editor = (EditorDescriptor) editorArray[i];
  //          if (!editors.contains(editor))
  //          {
  //            editors.add(editor);
  //          }
  //          IMemento idMemento = editorMemento
  //          .createChild(IWorkbenchConstants.TAG_DEFAULT_EDITOR);
  //          idMemento.putString(IWorkbenchConstants.TAG_ID, editorArray[i]
  //          .getId());
  //        }
  //      }
  //      Writer writer = null;
  //      try
  //      {
  //        writer = new StringWriter();
  //        memento.save(writer);
  //        writer.close();
  //        store.setValue(IPreferenceConstants.RESOURCES, writer.toString());
  //      }
  //      catch (IOException e)
  //      {
  //        try
  //        {
  //          if (writer != null)
  //          {
  //            writer.close();
  //          }
  //        }
  //        catch (IOException ex)
  //        {
  //          ex.printStackTrace();
  //        }
  //        MessageDialog.openError((Shell) null, "Saving Problems", //$NON-NLS-1$
  //            "Unable to save resource associations."); //$NON-NLS-1$
  //        return;
  //      }
  //
  //      memento = XMLMemento.createWriteRoot(IWorkbenchConstants.TAG_EDITORS);
  //      Iterator itr = editors.iterator();
  //      while (itr.hasNext())
  //      {
  //        EditorDescriptor editor = (EditorDescriptor) itr.next();
  //        IMemento editorMemento = memento
  //        .createChild(IWorkbenchConstants.TAG_DESCRIPTOR);
  //        editor.saveValues(editorMemento);
  //      }
  //      writer = null;
  //      try
  //      {
  //        writer = new StringWriter();
  //        memento.save(writer);
  //        writer.close();
  //        store.setValue(IPreferenceConstants.EDITORS, writer.toString());
  //      }
  //      catch (IOException e)
  //      {
  //        try
  //        {
  //          if (writer != null)
  //          {
  //            writer.close();
  //          }
  //        }
  //        catch (IOException ex)
  //        {
  //          ex.printStackTrace();
  //        }
  //        MessageDialog.openError((Shell) null,
  //            "Error", "Unable to save resource associations."); //$NON-NLS-1$ //$NON-NLS-2$
  //        return;
  //      }
}

void EditorRegistry::SetFileEditorMappings(
    const QList<FileEditorMapping::Pointer>& newResourceTypes)
{
  typeEditorMappings.Clear();
  for (int i = 0; i < newResourceTypes.size(); i++)
  {
    FileEditorMapping::Pointer mapping = newResourceTypes[i];
    typeEditorMappings.Put(this->MappingKeyFor(mapping), mapping);
  }
  //extensionImages = new HashMap();
  this->RebuildEditorMap();
  //firePropertyChange(PROP_CONTENTS);
}

void EditorRegistry::SetDefaultEditor(const QString& fileName,
    const QString& editorId)
{
  EditorDescriptor::Pointer desc = this->FindEditor(editorId).Cast<
      EditorDescriptor> ();
  QList<FileEditorMapping::Pointer> mapping = this->GetMappingForFilename(
      fileName);
  if (!mapping[0].IsNull())
  {
    mapping[0]->SetDefaultEditor(desc);
  }
  if (!mapping[1].IsNull())
  {
    mapping[1]->SetDefaultEditor(desc);
  }
}

QList<IEditorDescriptor::Pointer> EditorRegistry::SortEditors(
    const QList<IEditorDescriptor::Pointer>& unsortedList)
{
  QList<IEditorDescriptor::Pointer> result(unsortedList);
  std::sort(result.begin(), result.end(), CmpIEditorDescriptor());

  return result;
}

void EditorRegistry::SortInternalEditors()
{
  qSort(sortedEditorsFromPlugins.begin(), sortedEditorsFromPlugins.end(), CmpEditorDescriptor());
}

void EditorRegistry::EditorMap::PutDefault(const QString& key,
    FileEditorMapping::Pointer value)
{
  defaultMap[key] = value;
}

void EditorRegistry::EditorMap::Put(const QString& key,
    FileEditorMapping::Pointer value)
{
  QHash<QString, FileEditorMapping::Pointer>::iterator result =
      defaultMap.find(key);
  if (result != defaultMap.end())
  {
    map[key] = value;
  }
}

FileEditorMapping::Pointer EditorRegistry::EditorMap::Get(
    const QString& key)
{
  QHash<QString, FileEditorMapping::Pointer>::const_iterator result =
      map.find(key);
  if (result == map.end())
  {
    return defaultMap[key];
  }
  return result.value();
}

void EditorRegistry::EditorMap::Clear()
{
  defaultMap.clear();
  map.clear();
}

QList<FileEditorMapping::Pointer> EditorRegistry::EditorMap::AllMappings()
{
  QSet<FileEditorMapping::Pointer> resultSet;
  QHash<QString, FileEditorMapping::Pointer>::const_iterator iter;
  for (iter = defaultMap.begin(); iter != defaultMap.end(); ++iter)
  {
    resultSet.insert(iter.value());
  }
  for (iter = map.begin(); iter != map.end(); ++iter)
  {
    resultSet.insert(iter.value());
  }

  return resultSet.toList();
}

QList<FileEditorMapping::Pointer> EditorRegistry::EditorMap::UserMappings()
{
  return map.values();
}

bool EditorRegistry::IsSystemInPlaceEditorAvailable(const QString&  /*filename*/)
{
  //return ComponentSupport.inPlaceEditorAvailable(filename);
  return false;
}

bool EditorRegistry::IsSystemExternalEditorAvailable(
    const QString&  /*filename*/)
{
  //      QString::size_type nDot = filename.find_last_of('.');
  //      if (nDot != QString::npos)
  //      {
  //        QString strName = filename.substr(nDot);
  //        return Program.findProgram(strName) != null;
  //      }
  return false;
}

void EditorRegistry::RemoveEditorFromMapping(
    QHash<QString, FileEditorMapping::Pointer>& map,
    IEditorDescriptor::Pointer desc)
{
  FileEditorMapping::Pointer mapping;
  for (QHash<QString, FileEditorMapping::Pointer>::iterator iter =
      map.begin(); iter != map.end(); ++iter)
  {
    mapping = iter.value();
    QList<IEditorDescriptor::Pointer> editors(mapping->GetEditors());
    QList<IEditorDescriptor::Pointer>::iterator result = std::find(
        editors.begin(), editors.end(), desc);
    if (result != editors.end())
    {
      mapping->RemoveEditor(result->Cast<EditorDescriptor> ());
      editors.erase(result);
    }

    if (editors.empty())
    {
      map.erase(iter);
      break;
    }
  }
}

IExtensionPoint::Pointer EditorRegistry::GetExtensionPointFilter()
{
  return Platform::GetExtensionRegistry()->GetExtensionPoint(
        PlatformUI::PLUGIN_ID(), WorkbenchRegistryConstants::PL_EDITOR);
}

QList<IFileEditorMapping::Pointer> EditorRegistry::GetUnifiedMappings()
{
  QList<IFileEditorMapping::Pointer>
      standardMappings(
          dynamic_cast<EditorRegistry*> (PlatformUI::GetWorkbench() ->GetEditorRegistry())->GetFileEditorMappings());

  QList<IFileEditorMapping::Pointer> allMappings(standardMappings);
  // mock-up content type extensions into IFileEditorMappings
  //      IContentType [] contentTypes = Platform.getContentTypeManager().getAllContentTypes();
  //      for (int i = 0; i < contentTypes.length; i++)
  //      {
  //        IContentType type = contentTypes[i];
  //        String [] extensions = type.getFileSpecs(IContentType.FILE_EXTENSION_SPEC);
  //        for (int j = 0; j < extensions.length; j++)
  //        {
  //          String extension = extensions[j];
  //          boolean found = false;
  //          for (Iterator k = allMappings.iterator(); k.hasNext();)
  //          {
  //            IFileEditorMapping mapping = (IFileEditorMapping) k.next();
  //            if ("*".equals(mapping.getName())
  //                && extension.equals(mapping.getExtension()))
  //            { //$NON-NLS-1$
  //              found = true;
  //              break;
  //            }
  //          }
  //          if (!found)
  //          {
  //            MockMapping mockMapping = new MockMapping(type, "*", extension); //$NON-NLS-1$
  //            allMappings.add(mockMapping);
  //          }
  //        }
  //
  //        String [] filenames = type.getFileSpecs(IContentType.FILE_NAME_SPEC);
  //        for (int j = 0; j < filenames.length; j++)
  //        {
  //          String wholename = filenames[j];
  //          int idx = wholename.indexOf('.');
  //          String name = idx == -1 ? wholename : wholename.substring(0, idx);
  //          String extension = idx == -1 ? "" : wholename.substring(idx + 1); //$NON-NLS-1$
  //
  //          boolean found = false;
  //          for (Iterator k = allMappings.iterator(); k.hasNext();)
  //          {
  //            IFileEditorMapping mapping = (IFileEditorMapping) k.next();
  //            if (name.equals(mapping.getName())
  //                && extension.equals(mapping.getExtension()))
  //            {
  //              found = true;
  //              break;
  //            }
  //          }
  //          if (!found)
  //          {
  //            MockMapping mockMapping = new MockMapping(type, name, extension);
  //            allMappings.add(mockMapping);
  //          }
  //        }
  //      }

  return allMappings;
}

MockMapping::MockMapping(/*IContentType type,*/const QString& name,
    const QString& ext) :
  extension(ext), filename(name)
{
  //this.contentType = type;
}

IEditorDescriptor::Pointer MockMapping::GetDefaultEditor()
{
  //      QList<IEditorDescriptor::Pointer> candidates = PlatformUI::GetWorkbench()->GetEditorRegistry()
  //      ->GetEditorsForContentType(contentType);
  //      if (candidates.empty())
  //      {
  //        return IEditorDescriptor::Pointer();
  //      }
  //      return candidates[0];

  return IEditorDescriptor::Pointer();
}

QList<IEditorDescriptor::Pointer> MockMapping::GetEditors() const
{
  //      QList<IEditorDescriptor::Pointer> editorsForContentType = (dynamic_cast<EditorRegistry*>(PlatformUI
  //          ::GetWorkbench()->GetEditorRegistry())
  //      ->GetEditorsForContentType(contentType);
  //      return editorsForContentType;
  return QList<IEditorDescriptor::Pointer>();
}

QList<IEditorDescriptor::Pointer> MockMapping::GetDeletedEditors() const
{
  return QList<IEditorDescriptor::Pointer>();
}

QString MockMapping::GetExtension() const
{
  return extension;
}

QString MockMapping::GetLabel() const
{
  return filename + '.' + extension;
}

QString MockMapping::GetName() const
{
  return filename;
}

bool MockMapping::operator==(const Object* obj) const
{
  if (const MockMapping* other = dynamic_cast<const MockMapping*>(obj))
  {
    if (this == other)
    {
      return true;
    }

    //MockMapping mapping = (MockMapping) obj;
    if (!(this->filename == other->filename))
    {
      return false;
    }

    if (!(this->extension == other->extension))
    {
      return false;
    }

    if (!(this->GetEditors() == other->GetEditors()))
    {
      return false;
    }
    return this->GetDeletedEditors() == other->GetDeletedEditors();
  }

  return false;
}

}
