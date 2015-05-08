# Called by CTK.cmake (ExternalProject_Add) as a patch for CTK

# Fix pluginLoader.load() failing because of missing libraries. Use ctkAbstractPluginFactory.tpp as reference implementation.
set(path "Libs/PluginFramework/ctkPluginStorageSQL.cpp")
file(STRINGS ${path} contents NEWLINE_CONSUME)

# Replace
# #include "ctkPluginFrameworkContext_p.h"
# #include "ctkServiceException.h"
# with
# #include "ctkPluginFrameworkContext_p.h"
# #include "ctkScopedCurrentDir.h"
# #include "ctkServiceException.h"
string(REPLACE "#include \"ctkPluginFrameworkContext_p.h\"\n#include \"ctkServiceException.h\"" "#include \"ctkPluginFrameworkContext_p.h\"\n#include \"ctkScopedCurrentDir.h\"\n#include \"ctkServiceException.h\"" contents "${contents}")

# Replace
#  // Load the plugin and cache the resources
#
#  QPluginLoader pluginLoader;
# with
#  // Load the plugin and cache the resources
#
#  ctkScopedCurrentDir scopedCurrentDir(QFileInfo(pa->getLibLocation()).path());
#  QPluginLoader pluginLoader;
string(REPLACE "  // Load the plugin and cache the resources\n\n  QPluginLoader pluginLoader;" "  // Load the plugin and cache the resources\n\n  ctkScopedCurrentDir scopedCurrentDir(QFileInfo(pa->getLibLocation()).path());\n  QPluginLoader pluginLoader;" contents ${contents})

set(CONTENTS ${contents})
configure_file(${TEMPLATE_FILE} ${path} @ONLY NEWLINE_STYLE UNIX)

