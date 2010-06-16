/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef __BERRY_WORKBENCH_REGISTRY_CONSTANTS__
#define __BERRY_WORKBENCH_REGISTRY_CONSTANTS__

#include <iostream>

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 * Interface containing various registry constants (tag and attribute names).
 *
 */
struct WorkbenchRegistryConstants {

  /**
   * Accelerator attribute. Value <code>accelerator</code>.
   */
  static const std::string ATT_ACCELERATOR; // "accelerator"; //$NON-NLS-1$

  /**
   * Adaptable attribute. Value <code>adaptable</code>.
   */
  static const std::string ATT_ADAPTABLE; // "adaptable"; //$NON-NLS-1$

  /**
   * Advisor id attribute. Value <code>triggerPointAdvisorId</code>.
   */
  static const std::string ATT_ADVISORID; // "triggerPointAdvisorId"; //$NON-NLS-1$

  /**
   * Allow label update attribute. Value <code>allowLabelUpdate</code>.
   */
  static const std::string ATT_ALLOW_LABEL_UPDATE; // "allowLabelUpdate";//$NON-NLS-1$

  /**
   * View multiple attribute. Value <code>allowMultiple</code>.
   */
  static const std::string ATT_ALLOW_MULTIPLE; // "allowMultiple"; //$NON-NLS-1$

  /**
   * Attribute that specifies whether a view gets restored upon workbench restart. Value <code>restorable</code>.
   */
  static const std::string ATT_RESTORABLE; // = "restorable";

  /**
   * Attribute that specifies whether a wizard is immediately capable of
   * finishing. Value <code>canFinishEarly</code>.
   */
  static const std::string ATT_CAN_FINISH_EARLY; // "canFinishEarly"; //$NON-NLS-1$

  /**
   * The name of the category attribute, which appears on a command
   * definition.
   */
  static const std::string ATT_CATEGORY; // "category"; //$NON-NLS-1$

  /**
   * Category id attribute. Value <code>categoryId</code>.
   */
  static const std::string ATT_CATEGORY_ID; // "categoryId"; //$NON-NLS-1$

  /**
   * Class attribute. Value <code>class</code>.
   */
  static const std::string ATT_CLASS; // "class"; //$NON-NLS-1$

  /**
   * Sticky view closable attribute. Value <code>closable</code>.
   */
  static const std::string ATT_CLOSEABLE; // "closeable"; //$NON-NLS-1$

  /**
   * Color factory attribute. Value <code>colorFactory</code>.
   */
  static const std::string ATT_COLORFACTORY; // "colorFactory"; //$NON-NLS-1$

  /**
   * Editor command attribute. Value <code>command</code>.
   */
  static const std::string ATT_COMMAND; // "command";//$NON-NLS-1$

  /**
   * The name of the attribute storing the command id.
   */
  static const std::string ATT_COMMAND_ID; // "commandId"; //$NON-NLS-1$

  /**
   * The name of the configuration attribute storing the scheme id for a
   * binding.
   */
  static const std::string ATT_CONFIGURATION; // "configuration"; //$NON-NLS-1$

  /**
   * Intro content detector class attribute (optional). Value <code>contentDetector</code>.
   */
  static const std::string ATT_CONTENT_DETECTOR; // "contentDetector"; //$NON-NLS-1$

  /**
   * Editor content type id binding attribute. Value
   * <code>contentTypeId</code>.
   */
  static const std::string ATT_CONTENT_TYPE_ID; // "contentTypeId"; //$NON-NLS-1$

  /**
   * The name of the attribute storing the context id for a binding.
   */
  static const std::string ATT_CONTEXT_ID; // "contextId"; //$NON-NLS-1$

  /**
   * Editor contributor class attribute. Value <code>contributorClass</code>.
   */
  static const std::string ATT_CONTRIBUTOR_CLASS; // "contributorClass"; //$NON-NLS-1$

  /**
   * The name of the attribute storing the AbstractParameterValueConverter for
   * a commandParameterType.
   */
  static const std::string ATT_CONVERTER; // "converter"; //$NON-NLS-1$

  /**
   * Perspective default attribute. Value <code>default</code>.
   */
  static const std::string ATT_DEFAULT; // "default";//$NON-NLS-1$

  /**
   * The name of the default handler attribute, which appears on a command
   * definition.
   */
  static const std::string ATT_DEFAULT_HANDLER; // "defaultHandler"; //$NON-NLS-1$

  /**
   * Defaults-to attribute. Value <code>defaultsTo</code>.
   */
  static const std::string ATT_DEFAULTS_TO; // "defaultsTo"; //$NON-NLS-1$

  /**
   * Action definition id attribute. Value <code>definitionId</code>.
   */
  static const std::string ATT_DEFINITION_ID; // "definitionId";//$NON-NLS-1$

  /**
   * The name of the description attribute, which appears on named handle
   * objects.
   */
  static const std::string ATT_DESCRIPTION; // "description"; //$NON-NLS-1$

  /**
   * Description image attribute. Value <code>descriptionImage</code>.
   */
  static const std::string ATT_DESCRIPTION_IMAGE; // "descriptionImage"; //$NON-NLS-1$

  /**
   * Disabled icon attribute. Value <code>disabledIcon</code>.
   */
  static const std::string ATT_DISABLEDICON; // "disabledIcon";//$NON-NLS-1$

  /**
   * Enables-for attribute. Value <code>enablesFor</code>.
   */
  static const std::string ATT_ENABLES_FOR; // "enablesFor"; //$NON-NLS-1$

  /**
   * Editor extensions attribute. Value <code>extensions</code>.
   */
  static const std::string ATT_EXTENSIONS; // "extensions";//$NON-NLS-1$

  /**
   * View ratio attribute. Value <code>fastViewWidthRatio</code>.
   */
  static const std::string ATT_FAST_VIEW_WIDTH_RATIO; // "fastViewWidthRatio"; //$NON-NLS-1$

  /**
   * Editor filenames attribute. Value <code>filenames</code>.
   */
  static const std::string ATT_FILENAMES; // "filenames";//$NON-NLS-1$

  /**
   * Trim fill major attribute. Value <code>fillMajor</code>.
   */
  static const std::string ATT_FILL_MAJOR; // "fillMajor";//$NON-NLS-1$

  /**
   * Trim fill minor attribute. Value <code>fillMinor</code>.
   */
  static const std::string ATT_FILL_MINOR; // "fillMinor";//$NON-NLS-1$

  /**
   * Perspective fixed attribute. Value <code>fixed</code>.
   */
  static const std::string ATT_FIXED; // "fixed";//$NON-NLS-1$

  /**
   * Attribute that specifies whether a wizard has any pages. Value
   * <code>hasPages</code>.
   */
  static const std::string ATT_HAS_PAGES; // "hasPages"; //$NON-NLS-1$

  /**
   * Help context id attribute. Value <code>helpContextId</code>.
   */
  static const std::string ATT_HELP_CONTEXT_ID; // "helpContextId";//$NON-NLS-1$

  /**
   * Help url attribute. Value <code>helpHref</code>.
   */
  static const std::string ATT_HELP_HREF; // "helpHref"; //$NON-NLS-1$

  /**
   * Hover icon attribute. Value <code>hoverIcon</code>.
   */
  static const std::string ATT_HOVERICON; // "hoverIcon";//$NON-NLS-1$

  /**
   * Icon attribute. Value <code>icon</code>.
   */
  static const std::string ATT_ICON; // "icon"; //$NON-NLS-1$

  /**
   * Id attribute. Value <code>id</code>.
   */
  static const std::string ATT_ID; // "id"; //$NON-NLS-1$

  /**
   * The name of the image style attribute, which is used on location elements
   * in the menus extension point.
   */
  static const std::string ATT_IMAGE_STYLE; // "imageStyle"; //$NON-NLS-1$

  /**
   * Is-editable attribute. Value <code>isEditable</code>.
   */
  static const std::string ATT_IS_EDITABLE; // "isEditable"; //$NON-NLS-1$

  /**
   * Keys attribute. Value <code>keys</code>.
   */
  static const std::string ATT_KEY; // "key"; //$NON-NLS-1$

  /**
   * The name of the attribute storing the identifier for the active key
   * configuration identifier. This provides legacy support for the
   * <code>activeKeyConfiguration</code> element in the commands extension
   * point.
   */
  static const std::string ATT_KEY_CONFIGURATION_ID; // "keyConfigurationId"; //$NON-NLS-1$

  /**
   * The name of the attribute storing the trigger sequence for a binding.
   * This is called a 'keySequence' for legacy reasons.
   */
  static const std::string ATT_KEY_SEQUENCE; // "keySequence"; //$NON-NLS-1$

  /**
   * Label attribute. Value <code>label</code>.
   */
  static const std::string ATT_LABEL; // "label"; //$NON-NLS-1$

  /**
   * Editor launcher attribute. Value <code>launcher</code>.
   */
  static const std::string ATT_LAUNCHER; // "launcher";//$NON-NLS-1$

  /**
   * Lightweight decorator tag. Value <code>lightweight</code>.
   */
  static const std::string ATT_LIGHTWEIGHT; // "lightweight"; //$NON-NLS-1$

  /**
   * The name of the attribute storing the locale for a binding.
   */
  static const std::string ATT_LOCALE; // "locale"; //$NON-NLS-1$

  /**
   * Sticky view location attribute. Value <code>location</code>.
   */
  static const std::string ATT_LOCATION; // "location"; //$NON-NLS-1$

  /**
   * Editor management strategy attribute. Value <code>matchingStrategy</code>.
   */
  static const std::string ATT_MATCHING_STRATEGY; // "matchingStrategy"; //$NON-NLS-1$

  /**
   * The name of the menu identifier attribute, which appears on items.
   */
  static const std::string ATT_MENU_ID; // "menuId"; //$NON-NLS-1$

  /**
   * Menubar path attribute. Value <code>menubarPath</code>.
   */
  static const std::string ATT_MENUBAR_PATH; // "menubarPath";//$NON-NLS-1$

  /**
   * The name of the mnemonic attribute, which appears on locations.
   */
  static const std::string ATT_MNEMONIC; // "mnemonic"; //$NON-NLS-1$

  /**
   * The name of the minimized attribute, which appears
   * when adding a view in a perspectiveExtension.
   */
  static const std::string ATT_MINIMIZED; // "minimized"; //$NON-NLS-1$

  /**
   * Sticky view moveable attribute. Value <code>moveable</code>.
   */
  static const std::string ATT_MOVEABLE; // "moveable"; //$NON-NLS-1$

  /**
   * Name attribute. Value <code>name</code>.
   */
  static const std::string ATT_NAME; // "name"; //$NON-NLS-1$

  /**
   * Name filter attribute. Value <code>nameFilter</code>.
   */
  static const std::string ATT_NAME_FILTER; // "nameFilter"; //$NON-NLS-1$

  /**
   * Node attribute. Value <code>node</code>.
   */
  static const std::string ATT_NODE; // "node"; //$NON-NLS-1$

  /**
   * Object class attribute. Value <code>objectClass</code>.
   */
  static const std::string ATT_OBJECTCLASS; // "objectClass";//$NON-NLS-1$

  /**
   * The name of the optional attribute, which appears on parameter
   * definitions.
   */
  static const std::string ATT_OPTIONAL; // "optional"; //$NON-NLS-1$

  /**
   * Operating system attribute. Value <code>os</code>.
   */
  static const std::string ATT_OS; // "os"; //$NON-NLS-1$

  /**
   * The name of the deprecated parent attribute, which appears on scheme
   * definitions.
   */
  static const std::string ATT_PARENT; // "parent"; //$NON-NLS-1$

  /**
   * View parent category attribute. Value <code>parentCategory</code>.
   */
  static const std::string ATT_PARENT_CATEGORY; // "parentCategory"; //$NON-NLS-1$

  /**
   * Parent id attribute. Value <code>parentId</code>.
   */
  static const std::string ATT_PARENT_ID; // "parentId"; //$NON-NLS-1$

  /**
   * The name of the deprecated parent scope attribute, which appears on
   * contexts definitions.
   */
  static const std::string ATT_PARENT_SCOPE; // "parentScope"; //$NON-NLS-1$

  /**
   * Path attribute. Value <code>path</code>.
   */
  static const std::string ATT_PATH; // "path"; //$NON-NLS-1$

  /**
   * The name of the attribute storing the platform for a binding.
   */
  static const std::string ATT_PLATFORM; // "platform"; //$NON-NLS-1$

  /**
   * The name of the position attribute, which appears on order elements.
   */
  static const std::string ATT_POSITION; // "position"; //$NON-NLS-1$

  /**
   * Presentation id attribute. Value <code>presentationId</code>.
   */
  static const std::string ATT_PRESENTATIONID; // "presentationId"; //$NON-NLS-1$

  /**
   * Product id attribute. Value <code>productId</code>.
   */
  static const std::string ATT_PRODUCTID; // "productId"; //$NON-NLS-1$

  /**
   * Project attribute. Value <code>project</code>.
   */
  // @issue project-specific attribute and behavior
  static const std::string ATT_PROJECT; // "project";//$NON-NLS-1$  /**

  /**
   * The name of the pulldown attribute, which indicates whether the class is
   * a pulldown delegate.
   */
  static const std::string ATT_PULLDOWN; // "pulldown"; //$NON-NLS-1$

  /**
   * View ratio attribute. Value <code>ratio</code>.
   */
  static const std::string ATT_RATIO; // "ratio"; //$NON-NLS-1$

  /**
   * Relationship attribute. Value <code>relationship</code>.
   */
  static const std::string ATT_RELATIONSHIP; // "relationship";//$NON-NLS-1$

  /**
   * Relative attribute. Value <code>relative</code>.
   */
  static const std::string ATT_RELATIVE; // "relative";//$NON-NLS-1$

  /**
   * The name of the relativeTo attribute, which appears on order elements.
   */
  static const std::string ATT_RELATIVE_TO; // "relativeTo"; //$NON-NLS-1$

  /**
   * Retarget attribute. Value <code>retarget</code>.
   */
  static const std::string ATT_RETARGET; // "retarget";//$NON-NLS-1$

  /**
   * The name of the returnTypeId attribute, which appears on command
   * elements.
   */
  static const std::string ATT_RETURN_TYPE_ID; // "returnTypeId"; //$NON-NLS-1$

  /**
   * Role attribue. Value <code>role</code>.
   */
  static const std::string ATT_ROLE; // "role";

  /**
   * The name of the attribute storing the identifier for the active scheme.
   * This is called a 'keyConfigurationId' for legacy reasons.
   */
  static const std::string ATT_SCHEME_ID; // "schemeId"; //$NON-NLS-1$

  /**
   * Scope attribute. Value <code>scope</code>.
   */
  static const std::string ATT_SCOPE; // "scope"; //$NON-NLS-1$

  /**
   * The name of the separatorsVisible attribute, which appears on group
   * elements.
   */
  static const std::string ATT_SEPARATORS_VISIBLE; // "separatorsVisible"; //$NON-NLS-1$

  /**
   * The name of the sequence attribute for a key binding.
   */
  static const std::string ATT_SEQUENCE; // "sequence"; //$NON-NLS-1$

  /**
   * Show title attribute. Value <code>showTitle</code>.
   */
  static const std::string ATT_SHOW_TITLE; // "showTitle";//$NON-NLS-1$

  /**
   * Perspective singleton attribute. Value <code>singleton</code>.
   */
  static const std::string ATT_SINGLETON; // "singleton";//$NON-NLS-1$

  /**
   * Splash id attribute.  Value <code>splashId</code>.
   *
   * @since 3.3
   */
  static const std::string ATT_SPLASH_ID; // "splashId"; //$NON-NLS-1$

  /**
   * Standalone attribute. Value <code>standalone</code>.
   */
  static const std::string ATT_STANDALONE; // "standalone";//$NON-NLS-1$

  /**
   * Action state attribute. Value <code>state</code>.
   */
  static const std::string ATT_STATE; // "state";//$NON-NLS-1$

  /**
   * The name of the string attribute (key sequence) for a binding in the
   * commands extension point.
   */
  static const std::string ATT_STRING; // "string"; //$NON-NLS-1$

  /**
   * Action style attribute. Value <code>style</code>.
   */
  static const std::string ATT_STYLE; // "style";//$NON-NLS-1$

  /**
   * Target attribute. Value <code>targetID</code>.
   */
  static const std::string ATT_TARGET_ID; // "targetID";//$NON-NLS-1$

  /**
   * Toolbar path attribute. Value <code>toolbarPath</code>.
   */
  static const std::string ATT_TOOLBAR_PATH; // "toolbarPath";//$NON-NLS-1$

  /**
   * Tooltip attribute. Value <code>tooltip</code>.
   */
  static const std::string ATT_TOOLTIP; // "tooltip";//$NON-NLS-1$

  /**
   * The name of the type attribute, which appears on bar elements and
   * commandParameterType elments.
   */
  static const std::string ATT_TYPE; // "type"; //$NON-NLS-1$

  /**
   * The name of the typeId attribute, which appears on commandParameter
   * elements.
   */
  static const std::string ATT_TYPE_ID; // "typeId"; //$NON-NLS-1$

  /**
   * Value attribute. Value <code>value</code>.
   */
  static const std::string ATT_VALUE; // "value"; //$NON-NLS-1$

  /**
   * Visible attribute. Value <code>visible</code>.
   */
  // ATT_VISIBLE added by dan_rubel@instantiations.com
  static const std::string ATT_VISIBLE; // "visible";//$NON-NLS-1$

  /**
   * Windowing system attribute. Value <code>ws</code>.
   */
  static const std::string ATT_WS; // "ws"; //$NON-NLS-1$

  /**
   * The prefix that all auto-generated identifiers start with. This makes the
   * identifier recognizable as auto-generated, and further helps ensure that
   * it does not conflict with existing identifiers.
   */
  static const std::string AUTOGENERATED_PREFIX; // "AUTOGEN:::"; //$NON-NLS-1$

  /**
   * The legacy extension point (2.1.x and earlier) for specifying a key
   * binding scheme.
   *
   * @since 3.1.1
   */
  static const std::string PL_ACCELERATOR_CONFIGURATIONS; // "acceleratorConfigurations"; //$NON-NLS-1$

  /**
   * The legacy extension point (2.1.x and earlier) for specifying a context.
   *
   * @since 3.1.1
   */
  static const std::string PL_ACCELERATOR_SCOPES; // "acceleratorScopes"; //$NON-NLS-1$

  /**
   * The legacy extension point (2.1.x and earlier) for specifying a command.
   *
   * @since 3.1.1
   */
  static const std::string PL_ACTION_DEFINITIONS; // "actionDefinitions"; //$NON-NLS-1$

  static const std::string PL_ACTION_SET_PART_ASSOCIATIONS; // "actionSetPartAssociations"; //$NON-NLS-1$

  static const std::string PL_ACTION_SETS; // "actionSets"; //$NON-NLS-1$

  static const std::string PL_ACTIVITIES; // "activities"; //$NON-NLS-1$

  static const std::string PL_ACTIVITYSUPPORT; // "activitySupport"; //$NON-NLS-1$

  /**
   * The extension point (3.1 and later) for specifying bindings, such as
   * keyboard shortcuts.
   *
   * @since 3.1.1
   */
  static const std::string PL_BINDINGS; // "bindings"; //$NON-NLS-1$

  static const std::string PL_BROWSER_SUPPORT; // "browserSupport"; //$NON-NLS-1$

  static const std::string PL_COLOR_DEFINITIONS; // "colorDefinitions"; //$NON-NLS-1$

  /**
   * The extension point (3.2 and later) for associating images with commands.
   *
   * @since 3.2
   */
  static const std::string PL_COMMAND_IMAGES; // "commandImages"; //$NON-NLS-1$

  /**
   * The extension point (2.1.x and later) for specifying a command. A lot of
   * other things have appeared first in this extension point and then been
   * moved to their own extension point.
   *
   * @since 3.1.1
   */
  static const std::string PL_COMMANDS; // "commands"; //$NON-NLS-1$

  /**
   * The extension point (3.0 and later) for specifying a context.
   *
   * @since 3.1.1
   */
  static const std::string PL_CONTEXTS; // "contexts"; //$NON-NLS-1$

  static const std::string PL_DECORATORS; // "decorators"; //$NON-NLS-1$

  static const std::string PL_DROP_ACTIONS; // "dropActions"; //$NON-NLS-1$

  static const std::string PL_EDITOR; // "editors"; //$NON-NLS-1$

  static const std::string PL_EDITOR_ACTIONS; // "editorActions"; //$NON-NLS-1$

  static const std::string PL_ELEMENT_FACTORY; // "elementFactories"; //$NON-NLS-1$

  /**
   * The extension point for encoding definitions.
   */
  static const std::string PL_ENCODINGS; // "encodings"; //$NON-NLS-1$

  static const std::string PL_EXPORT; // "exportWizards"; //$NON-NLS-1$

  static const std::string PL_FONT_DEFINITIONS; // "fontDefinitions"; //$NON-NLS-1$

  /**
   * The extension point (3.1 and later) for specifying handlers.
   *
   * @since 3.1.1
   */
  static const std::string PL_HANDLERS; // "handlers"; //$NON-NLS-1$

  static const std::string PL_HELPSUPPORT; // "helpSupport"; //$NON-NLS-1$

  static const std::string PL_IMPORT; // "importWizards"; //$NON-NLS-1$

  static const std::string PL_INTRO; // "intro"; //$NON-NLS-1$

  /**
   * The extension point for keyword definitions.
   *
   * @since 3.1
   */
  static const std::string PL_KEYWORDS; // "keywords"; //$NON-NLS-1$

  /**
   * The extension point (3.2 and later) for specifying menu contributions.
   *
   * @since 3.2
   */
  static const std::string PL_MENUS; // "menus"; //$NON-NLS-1$

  /**
   * The extension point (3.3 and later) for specifying menu contributions.
   *
   * @since 3.3
   */
  static const std::string PL_MENU_CONTRIBUTION; // "menuContribution"; //$NON-NLS-1$

  static const std::string PL_NEW; // "newWizards"; //$NON-NLS-1$

  static const std::string PL_PERSPECTIVE_EXTENSIONS; // "perspectiveExtensions"; //$NON-NLS-1$

  static const std::string PL_PERSPECTIVES; // "perspectives"; //$NON-NLS-1$

  static const std::string PL_POPUP_MENU; // "popupMenus"; //$NON-NLS-1$

  static const std::string PL_PREFERENCE_TRANSFER; // "preferenceTransfer"; //$NON-NLS-1$

  static const std::string PL_PREFERENCES; // "preferencePages"; //$NON-NLS-1$

  static const std::string PL_PRESENTATION_FACTORIES; // "presentationFactories"; //$NON-NLS-1$

  static const std::string PL_PROPERTY_PAGES; // "propertyPages"; //$NON-NLS-1$

  static const std::string PL_STARTUP; // "startup"; //$NON-NLS-1$

  /**
   * @since 3.3
   */
  static const std::string PL_SPLASH_HANDLERS; // "splashHandlers"; //$NON-NLS-1$

  static const std::string PL_SYSTEM_SUMMARY_SECTIONS; // "systemSummarySections"; //$NON-NLS-1$

  static const std::string PL_THEMES; // "themes"; //$NON-NLS-1$

  static const std::string PL_VIEW_ACTIONS; // "viewActions"; //$NON-NLS-1$

  static const std::string PL_VIEWS; // "views"; //$NON-NLS-1$

  static const std::string PL_WORKINGSETS; // "workingSets"; //$NON-NLS-1$

  /**
   * The name of the deprecated accelerator configurations extension point.
   */
  static const std::string EXTENSION_ACCELERATOR_CONFIGURATIONS;

  /**
   * The name of the accelerator scopes extension point.
   */
  static const std::string EXTENSION_ACCELERATOR_SCOPES;

  /**
   * The name of the action definitions extension point.
   */
  static const std::string EXTENSION_ACTION_DEFINITIONS;

  /**
   * The name of the <code>org.blueberry.ui.actionSets</code> extension point.
   */
  static const std::string EXTENSION_ACTION_SETS;

  /**
   * The name of the bindings extension point.
   */
  static const std::string EXTENSION_BINDINGS;

  /**
   * The name of the commands extension point.
   */
  static const std::string EXTENSION_COMMAND_IMAGES;

  /**
   * The name of the commands extension point, and the name of the key for the
   * commands preferences.
   */
  static const std::string EXTENSION_COMMANDS;

  /**
   * The name of the contexts extension point.
   */
  static const std::string EXTENSION_CONTEXTS;

  /**
   * The name of the <code>org.blueberry.ui.editorActions</code> extension
   * point.
   */
  static const std::string EXTENSION_EDITOR_ACTIONS;

  /**
   * The name of the commands extension point.
   */
  static const std::string EXTENSION_HANDLERS;

  /**
   * The name of the <code>org.blueberry.ui.menus</code> extension point.
   */
  static const std::string EXTENSION_MENUS;

  /**
   * The name of the <code>org.blueberry.ui.menus2</code> extension point.
   */
  static const std::string COMMON_MENU_ADDITIONS;

  /**
   * The name of the <code>org.blueberry.ui.popupMenus</code> extension point.
   */
  static const std::string EXTENSION_POPUP_MENUS;

  /**
   * The name of the <code>org.blueberry.ui.viewActions</code> extension
   * point.
   */
  static const std::string EXTENSION_VIEW_ACTIONS;

  /**
   * The constant for the position attribute corresponding to
   * {@link SOrder#POSITION_AFTER}.
   */
  static const std::string POSITION_AFTER; // "after"; //$NON-NLS-1$

  /**
   * The constant for the position attribute corresponding to
   * {@link SOrder#POSITION_BEFORE}.
   */
  static const std::string POSITION_BEFORE; // "before"; //$NON-NLS-1$

  /**
   * The constant for the position attribute corresponding to
   * {@link SOrder#POSITION_END}.
   */
  static const std::string POSITION_END; // "end"; //$NON-NLS-1$

  /**
   * The constant for the position attribute corresponding to
   * {@link SOrder#POSITION_START}.
   */
  static const std::string POSITION_START; // "start"; //$NON-NLS-1$

  /**
   * The action style for drop-down menus.
   */
  static const std::string STYLE_PULLDOWN; // "pulldown"; //$NON-NLS-1$

  /**
   * The action style for radio buttons.
   */
  static const std::string STYLE_RADIO; // "radio"; //$NON-NLS-1$

  /**
   * The action style for check boxes.
   */
  static const std::string STYLE_TOGGLE; // "toggle"; //$NON-NLS-1$

  /**
   * The name of the deprecated accelerator configuration element. This
   * element was used in 2.1.x and earlier to define groups of what are now
   * called schemes.
   */
  static const std::string TAG_ACCELERATOR_CONFIGURATION; // "acceleratorConfiguration"; //$NON-NLS-1$

  /**
   * The name of the element storing a deprecated accelerator scope.
   */
  static const std::string TAG_ACCELERATOR_SCOPE; // "acceleratorScope"; //$NON-NLS-1$

  /**
   * Action tag. Value <code>action</code>.
   */
  static const std::string TAG_ACTION; // "action"; //$NON-NLS-1$

  /**
   * The name of the element storing an action definition. This element only
   * existed in
   */
  static const std::string TAG_ACTION_DEFINITION; // "actionDefinition"; //$NON-NLS-1$

  /**
   * Action set tag. Value <code>actionSet</code>.
   */
  static const std::string TAG_ACTION_SET; // "actionSet";//$NON-NLS-1$

  /**
   * Part association tag. Value <code>actionSetPartAssociation</code>.
   */
  static const std::string TAG_ACTION_SET_PART_ASSOCIATION; // "actionSetPartAssociation";//$NON-NLS-1$

  /**
   * The name of the element storing the active key configuration from the
   * commands extension point.
   */
  static const std::string TAG_ACTIVE_KEY_CONFIGURATION; // "activeKeyConfiguration"; //$NON-NLS-1$

  /**
   * The name of the active when element, which appears on a handler
   * definition.
   */
  static const std::string TAG_ACTIVE_WHEN; // "activeWhen"; //$NON-NLS-1$

  /**
   * Activity image binding tag. Value <code>activityImageBindingw</code>.
   */
  static const std::string TAG_ACTIVITY_IMAGE_BINDING; // "activityImageBinding"; //$NON-NLS-1$

  /**
   * Advisor to product binding element. Value
   * <code>triggerPointAdvisorProductBinding</code>.
   */
  static const std::string TAG_ADVISORPRODUCTBINDING; // "triggerPointAdvisorProductBinding"; //$NON-NLS-1$

  /**
   * The name of the bar element, which appears in a location definition.
   */
  static const std::string TAG_BAR; // "bar"; //$NON-NLS-1$

  /**
   * Category tag. Value <code>category</code>.
   */
  static const std::string TAG_CATEGORY; // "category";//$NON-NLS-1$

  /**
   * Category image binding tag. Value <code>categoryImageBinding</code>.
   */
  static const std::string TAG_CATEGORY_IMAGE_BINDING; // "categoryImageBinding"; //$NON-NLS-1$

  /**
   * Element category tag. Value <code>themeElementCategory</code>.
   */
  static const std::string TAG_CATEGORYDEFINITION; // "themeElementCategory"; //$NON-NLS-1$

  /**
   * Category presentation tag. Value <code>categoryPresentationBinding</code>.
   */
  static const std::string TAG_CATEGORYPRESENTATIONBINDING; // "categoryPresentationBinding"; //$NON-NLS-1$

  /**
   * The name of the class element, which appears on an executable extension.
   */
  static const std::string TAG_CLASS; // ATT_CLASS;

  /**
   * Color definition tag. Value <code>colorDefinition</code>.
   */
  static const std::string TAG_COLORDEFINITION; // "colorDefinition"; //$NON-NLS-1$

  /**
   * Color override tag. Value <code>colorOverride</code>.
   */
  static const std::string TAG_COLOROVERRIDE; // "colorOverride"; //$NON-NLS-1$

  /**
   * Color value tag. Value <code>colorValue</code>.
   */
  static const std::string TAG_COLORVALUE; // "colorValue"; //$NON-NLS-1$

  /**
   * The name of the element storing a command.
   */
  static const std::string TAG_COMMAND; // "command"; //$NON-NLS-1$

  /**
   * The name of the element storing a parameter.
   */
  static const std::string TAG_COMMAND_PARAMETER; // "commandParameter"; //$NON-NLS-1$

  /**
   * The name of the element storing a parameter type.
   */
  static const std::string TAG_COMMAND_PARAMETER_TYPE; // "commandParameterType"; //$NON-NLS-1$

  /**
   * Editor content type binding tag. Value <code>contentTypeBinding</code>.
   */
  static const std::string TAG_CONTENT_TYPE_BINDING; // "contentTypeBinding"; //$NON-NLS-1$

  /**
   * The name of the element storing a context.
   */
  static const std::string TAG_CONTEXT; // "context"; //$NON-NLS-1$

  /**
   * Data tag. Value <code>data</code>.
   */
  static const std::string TAG_DATA; // "data"; //$NON-NLS-1$

  /**
   * The name of the default handler element, which appears on a command
   * definition.
   */
  static const std::string TAG_DEFAULT_HANDLER; // ATT_DEFAULT_HANDLER;

  /**
   * Description element. Value <code>description</code>.
   */
  static const std::string TAG_DESCRIPTION; // "description"; //$NON-NLS-1$

  /**
   * The name of the dynamic menu element, which appears in a group or menu
   * definition.
   */
  static const std::string TAG_DYNAMIC; // "dynamic"; //$NON-NLS-1$

  /**
   * Editor tag. Value <code>editor</code>.
   */
  static const std::string TAG_EDITOR; // "editor";//$NON-NLS-1$

  /**
   * The name of the deprecated editorContribution element. This is used for
   * contributing actions to the top-level menus and tool bars when particular
   * editors are visible.
   */
  static const std::string TAG_EDITOR_CONTRIBUTION; // "editorContribution"; //$NON-NLS-1$

  /**
   * The name of the enabled when element, which appears on a handler
   * definition.
   */
  static const std::string TAG_ENABLED_WHEN; // "enabledWhen"; //$NON-NLS-1$

  /**
   * Enablement tag. Value <code>enablement</code>.
   */
  static const std::string TAG_ENABLEMENT; // "enablement"; //$NON-NLS-1$

  /**
   * Entry tag. Value <code>entry</code>.
   */
  static const std::string TAG_ENTRY; // "entry"; //$NON-NLS-1$

  /**
   * Filter tag. Value <code>filter</code>.
   */
  static const std::string TAG_FILTER; // "filter"; //$NON-NLS-1$

  /***************************************************************************
   * Font definition tag. Value <code>fontDefinition</code>.
   */
  static const std::string TAG_FONTDEFINITION; // "fontDefinition"; //$NON-NLS-1$

  /**
   * Font override tag. Value <code>fontOverride</code>.
   */
  static const std::string TAG_FONTOVERRIDE; // "fontOverride"; //$NON-NLS-1$

  /**
   * Font value tag. Value <code>fontValue</code>.
   */
  static const std::string TAG_FONTVALUE; // "fontValue"; //$NON-NLS-1$

  /**
   * The name of the element storing a group.
   */
  static const std::string TAG_GROUP; // "group"; //$NON-NLS-1$

  /**
   * Group marker tag. Value <code>groupMarker</code>.
   */
  static const std::string TAG_GROUP_MARKER; // "groupMarker"; //$NON-NLS-1$

  /**
   * The name of the element storing a handler.
   */
  static const std::string TAG_HANDLER; // "handler"; //$NON-NLS-1$

  /**
   * The name of the element storing a handler submission.
   */
  static const std::string TAG_HANDLER_SUBMISSION; // "handlerSubmission"; //$NON-NLS-1$

  /**
   * Trigger point hint tag. Value <code>hint</code>.
   */
  static const std::string TAG_HINT; // "hint"; //$NON-NLS-1$

  /**
   * The name of the element storing an image.
   */
  static const std::string TAG_IMAGE; // "image"; //$NON-NLS-1$

  /**
   * The name of the element storing a key binding.
   */
  static const std::string TAG_KEY; // "key"; //$NON-NLS-1$

  /**
   * The name of the key binding element in the commands extension point.
   */
  static const std::string TAG_KEY_BINDING; // "keyBinding"; //$NON-NLS-1$

  /**
   * The name of the deprecated key configuration element in the commands
   * extension point. This element has been replaced with the scheme element
   * in the bindings extension point.
   */
  static const std::string TAG_KEY_CONFIGURATION; // "keyConfiguration"; //$NON-NLS-1$

  /**
   * The name of the element storing a location.
   */
  static const std::string TAG_LOCATION; // "location"; //$NON-NLS-1$

  /**
   * The name of the element defining the insertion point for menu
   * additions.
   *
   * @since 3.3
   */
  static const std::string TAG_LOCATION_URI; // "locationURI"; //$NON-NLS-1$

  /**
   * The name of the element storing trim layout info for a widget.
   */
  static const std::string TAG_LAYOUT; // "layout"; //$NON-NLS-1$

  /**
   * Mapping tag. Value <code>mapping</code>.
   */
  static const std::string TAG_MAPPING; // "mapping"; //$NON-NLS-1$

  /**
   * Menu tag. Value <code>menu</code>.
   */
  static const std::string TAG_MENU; // "menu"; //$NON-NLS-1$

  /**
   * Wizard shortcut tag. Value <code>newWizardShortcut</code>.
   */
  static const std::string TAG_NEW_WIZARD_SHORTCUT; // "newWizardShortcut";//$NON-NLS-1$

  /**
   * Object contribution tag. Value <code>objectContribution</code>.
   */
  static const std::string TAG_OBJECT_CONTRIBUTION; // "objectContribution";//$NON-NLS-1$

  /**
   * The name of the element storing the ordering information.
   */
  static const std::string TAG_ORDER; // "order"; //$NON-NLS-1$

  /**
   * The name of the element storing a parameter.
   */
  static const std::string TAG_PARAMETER; // "parameter"; //$NON-NLS-1$

  /**
   * Part tag. Value <code>part</code>.
   */
  static const std::string TAG_PART; // "part";//$NON-NLS-1$

  /**
   * Perspective shortcut tag. Value <code>perspectiveShortcut</code>.
   */
  static const std::string TAG_PERSP_SHORTCUT; // "perspectiveShortcut";//$NON-NLS-1$

  /**
   * Perspective tag. Value <code>perspective</code>.
   */
  static const std::string TAG_PERSPECTIVE; // "perspective";//$NON-NLS-1$

  /**
   * Perspective extension tag. Value <code>perspectiveExtension</code>.
   */
  static const std::string TAG_PERSPECTIVE_EXTENSION; // "perspectiveExtension";//$NON-NLS-1$

  /**
   * Primary wizard tag. Value <code>primaryWizard</code>.
   */
  static const std::string TAG_PRIMARYWIZARD; // "primaryWizard"; //$NON-NLS-1$

  /**
   * The name of the element storing the a menu element reference.
   */
  static const std::string TAG_REFERENCE; // "reference"; //$NON-NLS-1$

  /**
   * The name of the scheme element in the bindings extension point.
   */
  static const std::string TAG_SCHEME; // "scheme"; //$NON-NLS-1$

  /**
   * The name of the element storing a deprecated scope.
   */
  static const std::string TAG_SCOPE; // "scope"; //$NON-NLS-1$

  /**
   * Selectiont tag. Value <code>selection</code>.
   */
  static const std::string TAG_SELECTION; // "selection"; //$NON-NLS-1$

  /**
   * Separator tag. Value <code>separator</code>.
   */
  static const std::string TAG_SEPARATOR; // "separator"; //$NON-NLS-1$


  /**
   * Tag for the settings transfer entry.
   */
  static const std::string TAG_SETTINGS_TRANSFER; // "settingsTransfer"; //$NON-NLS-1$

  /**
   * Show in part tag. Value <code>showInPart</code>.
   */
  static const std::string TAG_SHOW_IN_PART; // "showInPart";//$NON-NLS-1$

  /**
   * The name of the element storing some state.
   */
  static const std::string TAG_STATE; // "state"; //$NON-NLS-1$


  /**
   * The name of the element describing splash handlers. Value
   * <code>splashHandler</code>.
   * @since 3.3
   */
  static const std::string TAG_SPLASH_HANDLER; // "splashHandler"; //$NON-NLS-1$


  /**
   * The name of the element describing splash handler product bindings. Value
   * <code>splashHandlerProductBinding</code>.
   * @since 3.3
   */
  static const std::string TAG_SPLASH_HANDLER_PRODUCT_BINDING; // "splashHandlerProductBinding"; //$NON-NLS-1$

  /**
   * Sticky view tag. Value <code>stickyView</code>.
   */
  static const std::string TAG_STICKYVIEW; // "stickyView";//$NON-NLS-1$

  /**
   * Browser support tag. Value <code>support</code>.
   */
  static const std::string TAG_SUPPORT; // "support"; //$NON-NLS-1$

  /**
   * Theme tag. Value <code>theme</code>.
   */
  static const std::string TAG_THEME; // "theme";//$NON-NLS-1$

  /**
   * Transfer tag. Value <code>transfer</code>.
   */
  static const std::string TAG_TRANSFER; // "transfer";//$NON-NLS-1$

  /**
   * Trigger point tag. Value <code>triggerPoint</code>.
   */
  static const std::string TAG_TRIGGERPOINT; // "triggerPoint"; //$NON-NLS-1$

  /**
   * Advisor tag. Value <code>triggerPointAdvisor</code>.
   */
  static const std::string TAG_TRIGGERPOINTADVISOR; // "triggerPointAdvisor"; //$NON-NLS-1$

  /**
   * View tag. Value <code>view</code>.
   */
  static const std::string TAG_VIEW; // "view";//$NON-NLS-1$

  /**
   * View shortcut tag. Value <code>viewShortcut</code>.
   */
  static const std::string TAG_VIEW_SHORTCUT; // "viewShortcut";//$NON-NLS-1$

  /**
   * The name of the element storing a view contribution.
   */
  static const std::string TAG_VIEW_CONTRIBUTION; // "viewContribution"; //$NON-NLS-1$

  /**
   * Viewer contribution tag. Value <code>viewerContribution</code>.
   */
  static const std::string TAG_VIEWER_CONTRIBUTION; // "viewerContribution"; //$NON-NLS-1$

  /**
   * Visibility tag. Value <code>visibility</code>.
   */
  static const std::string TAG_VISIBILITY; // "visibility"; //$NON-NLS-1$

  /**
   * The name of the element storing the visible when condition.
   */
  static const std::string TAG_VISIBLE_WHEN; // "visibleWhen"; //$NON-NLS-1$

  /**
   * The name of the element storing a widget.
   */
  static const std::string TAG_WIDGET; // "widget"; //$NON-NLS-1$

  /**
   * The name of the element storing a control hosted in a ToolBar.
   */
  static const std::string TAG_CONTROL; // "control"; //$NON-NLS-1$

  /**
   * Wizard tag. Value <code>wizard</code>.
   */
  static const std::string TAG_WIZARD; // "wizard";//$NON-NLS-1$

  /**
   * Working set tag. Value <code>workingSet</code>.
   */
  static const std::string TAG_WORKING_SET; // "workingSet"; //$NON-NLS-1$

  /**
   * The type of reference which refers to a group.
   */
  static const std::string TYPE_GROUP; // "group"; //$NON-NLS-1$

  /**
   * The type of reference which refers to an item.
   */
  static const std::string TYPE_ITEM; // "item"; //$NON-NLS-1$

  /**
   * The type of reference which refers to an menu.
   */
  static const std::string TYPE_MENU; // "menu"; //$NON-NLS-1$

  /**
   * The type of reference which refers to the widget.
   */
  static const std::string TYPE_WIDGET; // "widget"; //$NON-NLS-1$

  static const std::string TAG_TOOLBAR; // "toolbar"; //$NON-NLS-1$

  static const std::string TAG_SERVICE_FACTORY; // "serviceFactory"; //$NON-NLS-1$

  static const std::string TAG_SERVICE; // "service"; //$NON-NLS-1$

  static const std::string ATTR_FACTORY_CLASS; // "factoryClass"; //$NON-NLS-1$

  static const std::string ATTR_SERVICE_CLASS; // "serviceClass"; //$NON-NLS-1$

  static const std::string TAG_SOURCE_PROVIDER; // "sourceProvider"; //$NON-NLS-1$

  static const std::string ATTR_PROVIDER; // "provider"; //$NON-NLS-1$

  static const std::string TAG_VARIABLE; // "variable"; //$NON-NLS-1$

  static const std::string ATT_PRIORITY_LEVEL; // "priorityLevel"; //$NON-NLS-1$

  static const std::string ATT_MODE; // "mode"; //$NON-NLS-1$

};

} // namespace berry

#endif // __BERRY_WORKBENCH_REGISTRY_CONSTANTS__
