set(H_FILES
  include/mitkRestServerConfig.h
  include/mitkNodeQueryParams.h
)

set(CPP_FILES
  mitkIRestServerService.cpp
  mitkRestServer.cpp
  mitkDataStorageBridge.cpp
  mitkNodeUidMapper.cpp
  mitkErrorResponse.cpp
  mitkHealthController.cpp
  mitkDataStorageController.cpp
  mitkSwaggerController.cpp
  mitkRenderingController.cpp
  mitkRenderWindowBridge.cpp
)

set(RESOURCE_FILES
  openapi.json
  swagger/index.html
  swagger/swagger-ui-bundle.js
  swagger/swagger-ui.css
  swagger/LICENSE
)
