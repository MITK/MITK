set(REQUIRED_PLUGINS
  org.mitk.example.gui.extensionpointdefinition
  org.mitk.example.gui.extensionpointcontribution
)

set(DESCRIPTION
"This BlueBerry example consists of two plugins that demonstrate the use of the extension point concept. The first plugin defines an extension point and holds the GUI. The user can insert a text in a text field. The second plugin contributes an extension in the form of two classes that can change the user text to upper or lower case.

<p>
The following features are demonstrated:

<ul>
<li> creating an extension point </li>
<li> creating an extension </li>
<li> collecting extensions for an extension point </li>
</ul>

See the main <a href=\"http://docs.mitk.org/nightly-qt4/BlueBerryExampleExtensionPoint.html\">documentation</a> for details."
)
