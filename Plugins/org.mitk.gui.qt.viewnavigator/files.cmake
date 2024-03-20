set(CPP_FILES
  src/internal/mitkPluginActivator.cpp
  src/internal/QmitkCategoryItem.cpp
  src/internal/QmitkCategoryItem.h
  src/internal/QmitkViewItem.cpp
  src/internal/QmitkViewItem.h
  src/internal/QmitkViewNavigatorView.cpp
  src/internal/QmitkViewModel.cpp
  src/internal/QmitkViewModel.h
  src/internal/QmitkViewProxyModel.cpp
  src/internal/QmitkViewProxyModel.h
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/QmitkViewNavigatorView.h
)

set(UI_FILES
  src/internal/QmitkViewNavigatorView.ui
)

set(CACHED_RESOURCE_FILES
  resources/view-manager.svg
  plugin.xml
)
