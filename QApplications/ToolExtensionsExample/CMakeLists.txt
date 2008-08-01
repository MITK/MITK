PROJECT ( ExternalTool )

# When configuring cmake, 
# set ITK_DIR to the directory, 
# where you compiled or installed ITK
FIND_PACKAGE(ITK)

# When configuring cmake, 
# Set MITK_DIR to the directory, 
# where you compiled MITK
FIND_PACKAGE(MITK)

# Make sure we find Qt3, this won't yet work with Qt4 
# (and MITK could even be compiled without any Qt)
FIND_PACKAGE(Qt3)

# Just needs to be here (?)
ADD_DEFINITIONS(${QT_DEFINITIONS})

# Fill in all tool GUI files (implementations) here.
# The macro later will process the file names and
# expect that these files contain a class of the
# same name as the file, i.e QmitkComplexExternalToolGUI.cpp
# is expected to contain a class QmitkComplexExternalToolGUI.
# The macro will create a factory for this tool GUI, which
# is constructed for these expected names, in this case
# the factory name would be QmitkComplexExternalToolGUIFactory.
SET( TOOL_QT3GUI_FILES
     QmitkComplexExternalToolGUI.cpp
   )

# Fill in all tool files (implementations) here.
# The macro later will process the file names and
# expect that these files contain a class of the
# same name as the file, i.e mitkPlainExternalTool.cpp
# is expected to contain a class mitk::PlainExternalTool.
# The macro will create a factory for this tool, which
# is constructed for these expected names, in this case
# the factory name would be mitk::PlainExternalToolFactory.
SET( TOOL_FILES
     mitkPlainExternalTool.cpp
     mitkComplexExternalTool.cpp
   )

# List here all additional files which need to be compiled
# into the new library.
SET( TOOL_ADDITIONAL_CPPS
)

# List here all additional headers which need to be run
# through Qt's moc. The moc-created files will automatically
# be added to TOOL_ADDITIONAL_CPPS and compiled into the lib.
SET( TOOL_ADDITIONAL_MOC_H
)

# Create a library called libmitkExternalTools.so/DLL.
# The macro will create this library and add all tools,
# tool GUIs and their respective factories to it.
MITK_GENERATE_TOOLS_LIBRARY(mitkExternalTools)

