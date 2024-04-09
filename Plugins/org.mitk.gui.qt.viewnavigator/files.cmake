set(CPP_FILES
  src/internal/mitkPluginActivator.cpp
  src/QmitkCategoryItem.cpp
  src/QmitkCategoryItem.h
  src/QmitkViewItem.cpp
  src/QmitkViewItem.h
  src/QmitkViewNavigatorView.cpp
  src/QmitkViewModel.cpp
  src/QmitkViewModel.h
  src/QmitkViewProxyModel.cpp
  src/QmitkViewProxyModel.h
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/QmitkViewNavigatorView.h
)

set(UI_FILES
  src/QmitkViewNavigatorView.ui
)

set(CACHED_RESOURCE_FILES
  resources/view-manager.svg
  plugin.xml
)
