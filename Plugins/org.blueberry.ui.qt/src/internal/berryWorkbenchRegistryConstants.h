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

#ifndef __BERRY_WORKBENCH_REGISTRY_CONSTANTS__
#define __BERRY_WORKBENCH_REGISTRY_CONSTANTS__

#include <QString>

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
  static const QString ATT_ACCELERATOR; // "accelerator";

  /**
   * Adaptable attribute. Value <code>adaptable</code>.
   */
  static const QString ATT_ADAPTABLE; // "adaptable";

  /**
   * Advisor id attribute. Value <code>triggerPointAdvisorId</code>.
   */
  static const QString ATT_ADVISORID; // "triggerPointAdvisorId";

  /**
   * Allow label update attribute. Value <code>allowLabelUpdate</code>.
   */
  static const QString ATT_ALLOW_LABEL_UPDATE; // "allowLabelUpdate";

  /**
   * View multiple attribute. Value <code>allowMultiple</code>.
   */
  static const QString ATT_ALLOW_MULTIPLE; // "allowMultiple";

  /**
   * Attribute that specifies whether a view gets restored upon workbench restart. Value <code>restorable</code>.
   */
  static const QString ATT_RESTORABLE; // = "restorable";

  /**
   * Attribute that specifies whether a wizard is immediately capable of
   * finishing. Value <code>canFinishEarly</code>.
   */
  static const QString ATT_CAN_FINISH_EARLY; // "canFinishEarly";

  /**
   * The name of the category attribute, which appears on a command
   * definition.
   */
  static const QString ATT_CATEGORY; // "category";

  /**
   * Category id attribute. Value <code>categoryId</code>.
   */
  static const QString ATT_CATEGORY_ID; // "categoryId";

  /**
   * Class attribute. Value <code>class</code>.
   */
  static const QString ATT_CLASS; // "class";

  /**
   * Sticky view closable attribute. Value <code>closable</code>.
   */
  static const QString ATT_CLOSEABLE; // "closeable";

  /**
   * Color factory attribute. Value <code>colorFactory</code>.
   */
  static const QString ATT_COLORFACTORY; // "colorFactory";

  /**
   * Editor command attribute. Value <code>command</code>.
   */
  static const QString ATT_COMMAND; // "command";

  /**
   * The name of the attribute storing the command id.
   */
  static const QString ATT_COMMAND_ID; // "commandId";

  /**
   * The name of the configuration attribute storing the scheme id for a
   * binding.
   */
  static const QString ATT_CONFIGURATION; // "configuration";

  /**
   * Intro content detector class attribute (optional). Value <code>contentDetector</code>.
   */
  static const QString ATT_CONTENT_DETECTOR; // "contentDetector";

  /**
   * Editor content type id binding attribute. Value
   * <code>contentTypeId</code>.
   */
  static const QString ATT_CONTENT_TYPE_ID; // "contentTypeId";

  /**
   * The name of the attribute storing the context id for a binding.
   */
  static const QString ATT_CONTEXT_ID; // "contextId";

  /**
   * Editor contributor class attribute. Value <code>contributorClass</code>.
   */
  static const QString ATT_CONTRIBUTOR_CLASS; // "contributorClass";

  /**
   * The name of the attribute storing the IParameterValueConverter for
   * a commandParameterType.
   */
  static const QString ATT_CONVERTER; // "converter";

  /**
   * Perspective default attribute. Value <code>default</code>.
   */
  static const QString ATT_DEFAULT; // "default";

  /**
   * The name of the default handler attribute, which appears on a command
   * definition.
   */
  static const QString ATT_DEFAULT_HANDLER; // "defaultHandler";

  /**
   * Defaults-to attribute. Value <code>defaultsTo</code>.
   */
  static const QString ATT_DEFAULTS_TO; // "defaultsTo";

  /**
   * Action definition id attribute. Value <code>definitionId</code>.
   */
  static const QString ATT_DEFINITION_ID; // "definitionId";

  /**
   * The name of the description attribute, which appears on named handle
   * objects.
   */
  static const QString ATT_DESCRIPTION; // "description";

  /**
   * Description image attribute. Value <code>descriptionImage</code>.
   */
  static const QString ATT_DESCRIPTION_IMAGE; // "descriptionImage";

  /**
   * Disabled icon attribute. Value <code>disabledIcon</code>.
   */
  static const QString ATT_DISABLEDICON; // "disabledIcon";

  /**
   * Enables-for attribute. Value <code>enablesFor</code>.
   */
  static const QString ATT_ENABLES_FOR; // "enablesFor";

  /**
   * Editor extensions attribute. Value <code>extensions</code>.
   */
  static const QString ATT_EXTENSIONS; // "extensions";

  /**
   * View ratio attribute. Value <code>fastViewWidthRatio</code>.
   */
  static const QString ATT_FAST_VIEW_WIDTH_RATIO; // "fastViewWidthRatio";

  /**
   * Editor filenames attribute. Value <code>filenames</code>.
   */
  static const QString ATT_FILENAMES; // "filenames";

  /**
   * Trim fill major attribute. Value <code>fillMajor</code>.
   */
  static const QString ATT_FILL_MAJOR; // "fillMajor";

  /**
   * Trim fill minor attribute. Value <code>fillMinor</code>.
   */
  static const QString ATT_FILL_MINOR; // "fillMinor";

  /**
   * Perspective fixed attribute. Value <code>fixed</code>.
   */
  static const QString ATT_FIXED; // "fixed";

  /**
   * Attribute that specifies whether a wizard has any pages. Value
   * <code>hasPages</code>.
   */
  static const QString ATT_HAS_PAGES; // "hasPages";

  /**
   * Help context id attribute. Value <code>helpContextId</code>.
   */
  static const QString ATT_HELP_CONTEXT_ID; // "helpContextId";

  /**
   * Help url attribute. Value <code>helpHref</code>.
   */
  static const QString ATT_HELP_HREF; // "helpHref";

  /**
   * Hover icon attribute. Value <code>hoverIcon</code>.
   */
  static const QString ATT_HOVERICON; // "hoverIcon";

  /**
   * Icon attribute. Value <code>icon</code>.
   */
  static const QString ATT_ICON; // "icon";

  /**
   * Id attribute. Value <code>id</code>.
   */
  static const QString ATT_ID; // "id";

  /**
   * The name of the image style attribute, which is used on location elements
   * in the menus extension point.
   */
  static const QString ATT_IMAGE_STYLE; // "imageStyle";

  /**
   * Is-editable attribute. Value <code>isEditable</code>.
   */
  static const QString ATT_IS_EDITABLE; // "isEditable";

  /**
   * Keys attribute. Value <code>keys</code>.
   */
  static const QString ATT_KEY; // "key";

  /**
   * The name of the attribute storing the identifier for the active key
   * configuration identifier. This provides legacy support for the
   * <code>activeKeyConfiguration</code> element in the commands extension
   * point.
   */
  static const QString ATT_KEY_CONFIGURATION_ID; // "keyConfigurationId";

  /**
   * The name of the attribute storing the trigger sequence for a binding.
   * This is called a 'keySequence' for legacy reasons.
   */
  static const QString ATT_KEY_SEQUENCE; // "keySequence";

  /**
   * Label attribute. Value <code>label</code>.
   */
  static const QString ATT_LABEL; // "label";

  /**
   * Editor launcher attribute. Value <code>launcher</code>.
   */
  static const QString ATT_LAUNCHER; // "launcher";

  /**
   * Lightweight decorator tag. Value <code>lightweight</code>.
   */
  static const QString ATT_LIGHTWEIGHT; // "lightweight";

  /**
   * The name of the attribute storing the locale for a binding.
   */
  static const QString ATT_LOCALE; // "locale";

  /**
   * Sticky view location attribute. Value <code>location</code>.
   */
  static const QString ATT_LOCATION; // "location";

  /**
   * Editor management strategy attribute. Value <code>matchingStrategy</code>.
   */
  static const QString ATT_MATCHING_STRATEGY; // "matchingStrategy";

  /**
   * The name of the menu identifier attribute, which appears on items.
   */
  static const QString ATT_MENU_ID; // "menuId";

  /**
   * Menubar path attribute. Value <code>menubarPath</code>.
   */
  static const QString ATT_MENUBAR_PATH; // "menubarPath";

  /**
   * The name of the mnemonic attribute, which appears on locations.
   */
  static const QString ATT_MNEMONIC; // "mnemonic";

  /**
   * The name of the minimized attribute, which appears
   * when adding a view in a perspectiveExtension.
   */
  static const QString ATT_MINIMIZED; // "minimized";

  /**
   * Sticky view moveable attribute. Value <code>moveable</code>.
   */
  static const QString ATT_MOVEABLE; // "moveable";

  /**
  * Name attribute. Value <code>case</code>.
  */
  static const QString ATT_CASE; // "case";

  /**
   * Name attribute. Value <code>name</code>.
   */
  static const QString ATT_NAME; // "name";

  /**
   * Name filter attribute. Value <code>nameFilter</code>.
   */
  static const QString ATT_NAME_FILTER; // "nameFilter";

  /**
   * Node attribute. Value <code>node</code>.
   */
  static const QString ATT_NODE; // "node";

  /**
   * Object class attribute. Value <code>objectClass</code>.
   */
  static const QString ATT_OBJECTCLASS; // "objectClass";

  /**
   * The name of the optional attribute, which appears on parameter
   * definitions.
   */
  static const QString ATT_OPTIONAL; // "optional";

  /**
   * Operating system attribute. Value <code>os</code>.
   */
  static const QString ATT_OS; // "os";

  /**
   * The name of the deprecated parent attribute, which appears on scheme
   * definitions.
   */
  static const QString ATT_PARENT; // "parent";

  /**
   * View parent category attribute. Value <code>parentCategory</code>.
   */
  static const QString ATT_PARENT_CATEGORY; // "parentCategory";

  /**
   * Parent id attribute. Value <code>parentId</code>.
   */
  static const QString ATT_PARENT_ID; // "parentId";

  /**
   * The name of the deprecated parent scope attribute, which appears on
   * contexts definitions.
   */
  static const QString ATT_PARENT_SCOPE; // "parentScope";

  /**
   * Path attribute. Value <code>path</code>.
   */
  static const QString ATT_PATH; // "path";

  /**
   * The name of the attribute storing the platform for a binding.
   */
  static const QString ATT_PLATFORM; // "platform";

  /**
   * The name of the position attribute, which appears on order elements.
   */
  static const QString ATT_POSITION; // "position";

  /**
   * Presentation id attribute. Value <code>presentationId</code>.
   */
  static const QString ATT_PRESENTATIONID; // "presentationId";

  /**
   * Product id attribute. Value <code>productId</code>.
   */
  static const QString ATT_PRODUCTID; // "productId";

  /**
   * Project attribute. Value <code>project</code>.
   */
  // @issue project-specific attribute and behavior
  static const QString ATT_PROJECT; // "project";  /**

  /**
   * The name of the pulldown attribute, which indicates whether the class is
   * a pulldown delegate.
   */
  static const QString ATT_PULLDOWN; // "pulldown";

  /**
   * View ratio attribute. Value <code>ratio</code>.
   */
  static const QString ATT_RATIO; // "ratio";

  /**
   * Relationship attribute. Value <code>relationship</code>.
   */
  static const QString ATT_RELATIONSHIP; // "relationship";

  /**
   * Relative attribute. Value <code>relative</code>.
   */
  static const QString ATT_RELATIVE; // "relative";

  /**
   * The name of the relativeTo attribute, which appears on order elements.
   */
  static const QString ATT_RELATIVE_TO; // "relativeTo";

  /**
   * Retarget attribute. Value <code>retarget</code>.
   */
  static const QString ATT_RETARGET; // "retarget";

  /**
   * The name of the returnTypeId attribute, which appears on command
   * elements.
   */
  static const QString ATT_RETURN_TYPE_ID; // "returnTypeId";

  /**
   * Role attribue. Value <code>role</code>.
   */
  static const QString ATT_ROLE; // "role";

  /**
   * The name of the attribute storing the identifier for the active scheme.
   * This is called a 'keyConfigurationId' for legacy reasons.
   */
  static const QString ATT_SCHEME_ID; // "schemeId";

  /**
   * Scope attribute. Value <code>scope</code>.
   */
  static const QString ATT_SCOPE; // "scope";

  /**
   * The name of the separatorsVisible attribute, which appears on group
   * elements.
   */
  static const QString ATT_SEPARATORS_VISIBLE; // "separatorsVisible";

  /**
   * The name of the sequence attribute for a key binding.
   */
  static const QString ATT_SEQUENCE; // "sequence";

  /**
   * Show title attribute. Value <code>showTitle</code>.
   */
  static const QString ATT_SHOW_TITLE; // "showTitle";

  /**
   * Perspective singleton attribute. Value <code>singleton</code>.
   */
  static const QString ATT_SINGLETON; // "singleton";

  /**
   * Splash id attribute.  Value <code>splashId</code>.
   *
   * @since 3.3
   */
  static const QString ATT_SPLASH_ID; // "splashId";

  /**
   * Standalone attribute. Value <code>standalone</code>.
   */
  static const QString ATT_STANDALONE; // "standalone";

  /**
   * Action state attribute. Value <code>state</code>.
   */
  static const QString ATT_STATE; // "state";

  /**
   * The name of the string attribute (key sequence) for a binding in the
   * commands extension point.
   */
  static const QString ATT_STRING; // "string";

  /**
   * Action style attribute. Value <code>style</code>.
   */
  static const QString ATT_STYLE; // "style";

  /**
   * Target attribute. Value <code>targetID</code>.
   */
  static const QString ATT_TARGET_ID; // "targetID";

  /**
   * Toolbar path attribute. Value <code>toolbarPath</code>.
   */
  static const QString ATT_TOOLBAR_PATH; // "toolbarPath";

  /**
   * Tooltip attribute. Value <code>tooltip</code>.
   */
  static const QString ATT_TOOLTIP; // "tooltip";

  /**
   * The name of the type attribute, which appears on bar elements and
   * commandParameterType elments.
   */
  static const QString ATT_TYPE; // "type";

  /**
   * The name of the typeId attribute, which appears on commandParameter
   * elements.
   */
  static const QString ATT_TYPE_ID; // "typeId";

  /**
   * Value attribute. Value <code>value</code>.
   */
  static const QString ATT_VALUE; // "value";

  /**
   * Visible attribute. Value <code>visible</code>.
   */
  // ATT_VISIBLE added by dan_rubel@instantiations.com
  static const QString ATT_VISIBLE; // "visible";

  /**
   * Windowing system attribute. Value <code>ws</code>.
   */
  static const QString ATT_WS; // "ws";

  /**
   * The prefix that all auto-generated identifiers start with. This makes the
   * identifier recognizable as auto-generated, and further helps ensure that
   * it does not conflict with existing identifiers.
   */
  static const QString AUTOGENERATED_PREFIX; // "AUTOGEN:::";

  /**
   * The legacy extension point (2.1.x and earlier) for specifying a key
   * binding scheme.
   *
   * @since 3.1.1
   */
  static const QString PL_ACCELERATOR_CONFIGURATIONS; // "acceleratorConfigurations";

  /**
   * The legacy extension point (2.1.x and earlier) for specifying a context.
   *
   * @since 3.1.1
   */
  static const QString PL_ACCELERATOR_SCOPES; // "acceleratorScopes";

  /**
   * The legacy extension point (2.1.x and earlier) for specifying a command.
   *
   * @since 3.1.1
   */
  static const QString PL_ACTION_DEFINITIONS; // "actionDefinitions";

  static const QString PL_ACTION_SET_PART_ASSOCIATIONS; // "actionSetPartAssociations";

  static const QString PL_ACTION_SETS; // "actionSets";

  static const QString PL_ACTIVITIES; // "activities";

  static const QString PL_ACTIVITYSUPPORT; // "activitySupport";

  /**
   * The extension point (3.1 and later) for specifying bindings, such as
   * keyboard shortcuts.
   *
   * @since 3.1.1
   */
  static const QString PL_BINDINGS; // "bindings";

  static const QString PL_BROWSER_SUPPORT; // "browserSupport";

  static const QString PL_COLOR_DEFINITIONS; // "colorDefinitions";

  /**
   * The extension point (3.2 and later) for associating images with commands.
   *
   * @since 3.2
   */
  static const QString PL_COMMAND_IMAGES; // "commandImages";

  /**
   * The extension point (2.1.x and later) for specifying a command. A lot of
   * other things have appeared first in this extension point and then been
   * moved to their own extension point.
   *
   * @since 3.1.1
   */
  static const QString PL_COMMANDS; // "commands";

  /**
   * The extension point (3.0 and later) for specifying a context.
   *
   * @since 3.1.1
   */
  static const QString PL_CONTEXTS; // "contexts";

  static const QString PL_DECORATORS; // "decorators";

  static const QString PL_DROP_ACTIONS; // "dropActions";

  static const QString PL_EDITOR; // "editors";

  static const QString PL_EDITOR_ACTIONS; // "editorActions";

  static const QString PL_ELEMENT_FACTORY; // "elementFactories";

  /**
   * The extension point for encoding definitions.
   */
  static const QString PL_ENCODINGS; // "encodings";

  static const QString PL_EXPORT; // "exportWizards";

  static const QString PL_FONT_DEFINITIONS; // "fontDefinitions";

  /**
   * The extension point (3.1 and later) for specifying handlers.
   *
   * @since 3.1.1
   */
  static const QString PL_HANDLERS; // "handlers";

  static const QString PL_HELPSUPPORT; // "helpSupport";

  static const QString PL_IMPORT; // "importWizards";

  static const QString PL_INTRO; // "intro";

  /**
   * The extension point for keyword definitions.
   *
   * @since 3.1
   */
  static const QString PL_KEYWORDS; // "keywords";

  /**
   * The extension point (3.2 and later) for specifying menu contributions.
   *
   * @since 3.2
   */
  static const QString PL_MENUS; // "menus";

  /**
   * The extension point (3.3 and later) for specifying menu contributions.
   *
   * @since 3.3
   */
  static const QString PL_MENU_CONTRIBUTION; // "menuContribution";

  static const QString PL_NEW; // "newWizards";

  static const QString PL_PERSPECTIVE_EXTENSIONS; // "perspectiveExtensions";

  static const QString PL_PERSPECTIVES; // "perspectives";

  static const QString PL_POPUP_MENU; // "popupMenus";

  static const QString PL_PREFERENCE_TRANSFER; // "preferenceTransfer";

  static const QString PL_PREFERENCES; // "preferencePages";

  static const QString PL_PRESENTATION_FACTORIES; // "presentationFactories";

  static const QString PL_PROPERTY_PAGES; // "propertyPages";

  static const QString PL_STARTUP; // "startup";

  /**
   * @since 3.3
   */
  static const QString PL_SPLASH_HANDLERS; // "splashHandlers";

  static const QString PL_SYSTEM_SUMMARY_SECTIONS; // "systemSummarySections";

  static const QString PL_THEMES; // "themes";

  static const QString PL_VIEW_ACTIONS; // "viewActions";

  static const QString PL_VIEWS; // "views";

  static const QString PL_WORKINGSETS; // "workingSets";

  /**
   * The name of the deprecated accelerator configurations extension point.
   */
  static const QString EXTENSION_ACCELERATOR_CONFIGURATIONS;

  /**
   * The name of the accelerator scopes extension point.
   */
  static const QString EXTENSION_ACCELERATOR_SCOPES;

  /**
   * The name of the action definitions extension point.
   */
  static const QString EXTENSION_ACTION_DEFINITIONS;

  /**
   * The name of the <code>org.blueberry.ui.actionSets</code> extension point.
   */
  static const QString EXTENSION_ACTION_SETS;

  /**
   * The name of the bindings extension point.
   */
  static const QString EXTENSION_BINDINGS;

  /**
   * The name of the commands extension point.
   */
  static const QString EXTENSION_COMMAND_IMAGES;

  /**
   * The name of the commands extension point, and the name of the key for the
   * commands preferences.
   */
  static const QString EXTENSION_COMMANDS;

  /**
   * The name of the contexts extension point.
   */
  static const QString EXTENSION_CONTEXTS;

  /**
   * The name of the <code>org.blueberry.ui.editorActions</code> extension
   * point.
   */
  static const QString EXTENSION_EDITOR_ACTIONS;

  /**
   * The name of the commands extension point.
   */
  static const QString EXTENSION_HANDLERS;

  /**
   * The name of the <code>org.blueberry.ui.menus</code> extension point.
   */
  static const QString EXTENSION_MENUS;

  /**
   * The name of the <code>org.blueberry.ui.menus2</code> extension point.
   */
  static const QString COMMON_MENU_ADDITIONS;

  /**
   * The name of the <code>org.blueberry.ui.popupMenus</code> extension point.
   */
  static const QString EXTENSION_POPUP_MENUS;

  /**
   * The name of the <code>org.blueberry.ui.viewActions</code> extension
   * point.
   */
  static const QString EXTENSION_VIEW_ACTIONS;

  /**
   * The constant for the position attribute corresponding to
   * {@link SOrder#POSITION_AFTER}.
   */
  static const QString POSITION_AFTER; // "after";

  /**
   * The constant for the position attribute corresponding to
   * {@link SOrder#POSITION_BEFORE}.
   */
  static const QString POSITION_BEFORE; // "before";

  /**
   * The constant for the position attribute corresponding to
   * {@link SOrder#POSITION_END}.
   */
  static const QString POSITION_END; // "end";

  /**
   * The constant for the position attribute corresponding to
   * {@link SOrder#POSITION_START}.
   */
  static const QString POSITION_START; // "start";

  /**
   * The action style for drop-down menus.
   */
  static const QString STYLE_PULLDOWN; // "pulldown";

  /**
   * The action style for radio buttons.
   */
  static const QString STYLE_RADIO; // "radio";

  /**
   * The action style for check boxes.
   */
  static const QString STYLE_TOGGLE; // "toggle";

  /**
   * The name of the deprecated accelerator configuration element. This
   * element was used in 2.1.x and earlier to define groups of what are now
   * called schemes.
   */
  static const QString TAG_ACCELERATOR_CONFIGURATION; // "acceleratorConfiguration";

  /**
   * The name of the element storing a deprecated accelerator scope.
   */
  static const QString TAG_ACCELERATOR_SCOPE; // "acceleratorScope";

  /**
   * Action tag. Value <code>action</code>.
   */
  static const QString TAG_ACTION; // "action";

  /**
   * The name of the element storing an action definition. This element only
   * existed in
   */
  static const QString TAG_ACTION_DEFINITION; // "actionDefinition";

  /**
   * Action set tag. Value <code>actionSet</code>.
   */
  static const QString TAG_ACTION_SET; // "actionSet";

  /**
   * Part association tag. Value <code>actionSetPartAssociation</code>.
   */
  static const QString TAG_ACTION_SET_PART_ASSOCIATION; // "actionSetPartAssociation";

  /**
   * The name of the element storing the active key configuration from the
   * commands extension point.
   */
  static const QString TAG_ACTIVE_KEY_CONFIGURATION; // "activeKeyConfiguration";

  /**
   * The name of the active when element, which appears on a handler
   * definition.
   */
  static const QString TAG_ACTIVE_WHEN; // "activeWhen";

  /**
   * Activity image binding tag. Value <code>activityImageBindingw</code>.
   */
  static const QString TAG_ACTIVITY_IMAGE_BINDING; // "activityImageBinding";

  /**
   * Advisor to product binding element. Value
   * <code>triggerPointAdvisorProductBinding</code>.
   */
  static const QString TAG_ADVISORPRODUCTBINDING; // "triggerPointAdvisorProductBinding";

  /**
   * The name of the bar element, which appears in a location definition.
   */
  static const QString TAG_BAR; // "bar";

  /**
   * Category tag. Value <code>category</code>.
   */
  static const QString TAG_CATEGORY; // "category";

  /**
   * Category image binding tag. Value <code>categoryImageBinding</code>.
   */
  static const QString TAG_CATEGORY_IMAGE_BINDING; // "categoryImageBinding";

  /**
   * Element category tag. Value <code>themeElementCategory</code>.
   */
  static const QString TAG_CATEGORYDEFINITION; // "themeElementCategory";

  /**
   * Category presentation tag. Value <code>categoryPresentationBinding</code>.
   */
  static const QString TAG_CATEGORYPRESENTATIONBINDING; // "categoryPresentationBinding";

  /**
   * The name of the class element, which appears on an executable extension.
   */
  static const QString TAG_CLASS; // ATT_CLASS;

  /**
   * Color definition tag. Value <code>colorDefinition</code>.
   */
  static const QString TAG_COLORDEFINITION; // "colorDefinition";

  /**
   * Color override tag. Value <code>colorOverride</code>.
   */
  static const QString TAG_COLOROVERRIDE; // "colorOverride";

  /**
   * Color value tag. Value <code>colorValue</code>.
   */
  static const QString TAG_COLORVALUE; // "colorValue";

  /**
   * The name of the element storing a command.
   */
  static const QString TAG_COMMAND; // "command";

  /**
   * The name of the element storing a parameter.
   */
  static const QString TAG_COMMAND_PARAMETER; // "commandParameter";

  /**
   * The name of the element storing a parameter type.
   */
  static const QString TAG_COMMAND_PARAMETER_TYPE; // "commandParameterType";

  /**
   * Editor content type binding tag. Value <code>contentTypeBinding</code>.
   */
  static const QString TAG_CONTENT_TYPE_BINDING; // "contentTypeBinding";

  /**
   * The name of the element storing a context.
   */
  static const QString TAG_CONTEXT; // "context";

  /**
   * Data tag. Value <code>data</code>.
   */
  static const QString TAG_DATA; // "data";

  /**
   * The name of the default handler element, which appears on a command
   * definition.
   */
  static const QString TAG_DEFAULT_HANDLER; // ATT_DEFAULT_HANDLER;

  /**
   * Description element. Value <code>description</code>.
   */
  static const QString TAG_DESCRIPTION; // "description";

  /**
   * The name of the dynamic menu element, which appears in a group or menu
   * definition.
   */
  static const QString TAG_DYNAMIC; // "dynamic";

  /**
   * Editor tag. Value <code>editor</code>.
   */
  static const QString TAG_EDITOR; // "editor";

  /**
   * The name of the deprecated editorContribution element. This is used for
   * contributing actions to the top-level menus and tool bars when particular
   * editors are visible.
   */
  static const QString TAG_EDITOR_CONTRIBUTION; // "editorContribution";

  /**
   * The name of the enabled when element, which appears on a handler
   * definition.
   */
  static const QString TAG_ENABLED_WHEN; // "enabledWhen";

  /**
   * Enablement tag. Value <code>enablement</code>.
   */
  static const QString TAG_ENABLEMENT; // "enablement";

  /**
   * Entry tag. Value <code>entry</code>.
   */
  static const QString TAG_ENTRY; // "entry";

  /**
   * Filter tag. Value <code>filter</code>.
   */
  static const QString TAG_FILTER; // "filter";

  /***************************************************************************
   * Font definition tag. Value <code>fontDefinition</code>.
   */
  static const QString TAG_FONTDEFINITION; // "fontDefinition";

  /**
   * Font override tag. Value <code>fontOverride</code>.
   */
  static const QString TAG_FONTOVERRIDE; // "fontOverride";

  /**
   * Font value tag. Value <code>fontValue</code>.
   */
  static const QString TAG_FONTVALUE; // "fontValue";

  /**
   * The name of the element storing a group.
   */
  static const QString TAG_GROUP; // "group";

  /**
   * Group marker tag. Value <code>groupMarker</code>.
   */
  static const QString TAG_GROUP_MARKER; // "groupMarker";

  /**
   * The name of the element storing a handler.
   */
  static const QString TAG_HANDLER; // "handler";

  /**
   * The name of the element storing a handler submission.
   */
  static const QString TAG_HANDLER_SUBMISSION; // "handlerSubmission";

  /**
   * Trigger point hint tag. Value <code>hint</code>.
   */
  static const QString TAG_HINT; // "hint";

  /**
   * The name of the element storing an image.
   */
  static const QString TAG_IMAGE; // "image";

  /**
   * The name of the element storing a key binding.
   */
  static const QString TAG_KEY; // "key";

  /**
   * The name of the key binding element in the commands extension point.
   */
  static const QString TAG_KEY_BINDING; // "keyBinding";

  /**
   * The name of the deprecated key configuration element in the commands
   * extension point. This element has been replaced with the scheme element
   * in the bindings extension point.
   */
  static const QString TAG_KEY_CONFIGURATION; // "keyConfiguration";

  /**
   * The name of the element storing a location.
   */
  static const QString TAG_LOCATION; // "location";

  /**
   * The name of the element defining the insertion point for menu
   * additions.
   *
   * @since 3.3
   */
  static const QString TAG_LOCATION_URI; // "locationURI";

  /**
   * The name of the element storing trim layout info for a widget.
   */
  static const QString TAG_LAYOUT; // "layout";

  /**
   * Mapping tag. Value <code>mapping</code>.
   */
  static const QString TAG_MAPPING; // "mapping";

  /**
   * Menu tag. Value <code>menu</code>.
   */
  static const QString TAG_MENU; // "menu";

  /**
   * Wizard shortcut tag. Value <code>newWizardShortcut</code>.
   */
  static const QString TAG_NEW_WIZARD_SHORTCUT; // "newWizardShortcut";

  /**
   * Object contribution tag. Value <code>objectContribution</code>.
   */
  static const QString TAG_OBJECT_CONTRIBUTION; // "objectContribution";

  /**
   * The name of the element storing the ordering information.
   */
  static const QString TAG_ORDER; // "order";

  /**
   * The name of the element storing a parameter.
   */
  static const QString TAG_PARAMETER; // "parameter";

  /**
   * Part tag. Value <code>part</code>.
   */
  static const QString TAG_PART; // "part";

  /**
   * Perspective shortcut tag. Value <code>perspectiveShortcut</code>.
   */
  static const QString TAG_PERSP_SHORTCUT; // "perspectiveShortcut";

  /**
   * Perspective tag. Value <code>perspective</code>.
   */
  static const QString TAG_PERSPECTIVE; // "perspective";

  /**
   * Perspective extension tag. Value <code>perspectiveExtension</code>.
   */
  static const QString TAG_PERSPECTIVE_EXTENSION; // "perspectiveExtension";

  /**
   * Primary wizard tag. Value <code>primaryWizard</code>.
   */
  static const QString TAG_PRIMARYWIZARD; // "primaryWizard";

  /**
   * The name of the element storing the a menu element reference.
   */
  static const QString TAG_REFERENCE; // "reference";

  /**
   * The name of the scheme element in the bindings extension point.
   */
  static const QString TAG_SCHEME; // "scheme";

  /**
   * The name of the element storing a deprecated scope.
   */
  static const QString TAG_SCOPE; // "scope";

  /**
   * Selectiont tag. Value <code>selection</code>.
   */
  static const QString TAG_SELECTION; // "selection";

  /**
   * Separator tag. Value <code>separator</code>.
   */
  static const QString TAG_SEPARATOR; // "separator";


  /**
   * Tag for the settings transfer entry.
   */
  static const QString TAG_SETTINGS_TRANSFER; // "settingsTransfer";

  /**
   * Show in part tag. Value <code>showInPart</code>.
   */
  static const QString TAG_SHOW_IN_PART; // "showInPart";

  /**
   * The name of the element storing some state.
   */
  static const QString TAG_STATE; // "state";


  /**
   * The name of the element describing splash handlers. Value
   * <code>splashHandler</code>.
   * @since 3.3
   */
  static const QString TAG_SPLASH_HANDLER; // "splashHandler";


  /**
   * The name of the element describing splash handler product bindings. Value
   * <code>splashHandlerProductBinding</code>.
   * @since 3.3
   */
  static const QString TAG_SPLASH_HANDLER_PRODUCT_BINDING; // "splashHandlerProductBinding";

  /**
   * Sticky view tag. Value <code>stickyView</code>.
   */
  static const QString TAG_STICKYVIEW; // "stickyView";

  /**
   * Browser support tag. Value <code>support</code>.
   */
  static const QString TAG_SUPPORT; // "support";

  /**
   * Theme tag. Value <code>theme</code>.
   */
  static const QString TAG_THEME; // "theme";

  /**
   * Transfer tag. Value <code>transfer</code>.
   */
  static const QString TAG_TRANSFER; // "transfer";

  /**
   * Trigger point tag. Value <code>triggerPoint</code>.
   */
  static const QString TAG_TRIGGERPOINT; // "triggerPoint";

  /**
   * Advisor tag. Value <code>triggerPointAdvisor</code>.
   */
  static const QString TAG_TRIGGERPOINTADVISOR; // "triggerPointAdvisor";

  /**
   * View tag. Value <code>view</code>.
   */
  static const QString TAG_VIEW; // "view";

  /**
   * View shortcut tag. Value <code>viewShortcut</code>.
   */
  static const QString TAG_VIEW_SHORTCUT; // "viewShortcut";

  /**
   * The name of the element storing a view contribution.
   */
  static const QString TAG_VIEW_CONTRIBUTION; // "viewContribution";

  /**
   * Viewer contribution tag. Value <code>viewerContribution</code>.
   */
  static const QString TAG_VIEWER_CONTRIBUTION; // "viewerContribution";

  /**
   * Visibility tag. Value <code>visibility</code>.
   */
  static const QString TAG_VISIBILITY; // "visibility";

  /**
   * The name of the element storing the visible when condition.
   */
  static const QString TAG_VISIBLE_WHEN; // "visibleWhen";

  /**
   * The name of the element storing a widget.
   */
  static const QString TAG_WIDGET; // "widget";

  /**
   * The name of the element storing a control hosted in a ToolBar.
   */
  static const QString TAG_CONTROL; // "control";

  /**
   * Wizard tag. Value <code>wizard</code>.
   */
  static const QString TAG_WIZARD; // "wizard";

  /**
   * Working set tag. Value <code>workingSet</code>.
   */
  static const QString TAG_WORKING_SET; // "workingSet";

  /**
   * The type of reference which refers to a group.
   */
  static const QString TYPE_GROUP; // "group";

  /**
   * The type of reference which refers to an item.
   */
  static const QString TYPE_ITEM; // "item";

  /**
   * The type of reference which refers to an menu.
   */
  static const QString TYPE_MENU; // "menu";

  /**
   * The type of reference which refers to the widget.
   */
  static const QString TYPE_WIDGET; // "widget";

  static const QString TAG_TOOLBAR; // "toolbar";

  static const QString TAG_SERVICE_FACTORY; // "serviceFactory";

  static const QString TAG_SERVICE; // "service";

  static const QString ATTR_FACTORY_CLASS; // "factoryClass";

  static const QString ATTR_SERVICE_CLASS; // "serviceClass";

  static const QString TAG_SOURCE_PROVIDER; // "sourceProvider";

  static const QString ATTR_PROVIDER; // "provider";

  static const QString TAG_VARIABLE; // "variable";

  static const QString ATT_PRIORITY_LEVEL; // "priorityLevel";

  static const QString ATT_MODE; // "mode";

};

} // namespace berry

#endif // __BERRY_WORKBENCH_REGISTRY_CONSTANTS__
