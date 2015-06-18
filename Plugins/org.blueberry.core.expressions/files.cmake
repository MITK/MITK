set(MOC_H_FILES
  src/berryPropertyTester.h

  src/internal/berryPlatformPropertyTester.h
  src/internal/berryExpressionPlugin.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(SRC_CPP_FILES
  berryElementHandler.cpp
  berryEvaluationContext.cpp
  berryEvaluationResult.cpp
  berryExpression.cpp
  berryExpressionConverter.cpp
  berryExpressionInfo.cpp
  berryExpressionTagNames.cpp
  berryICountable.h
  berryIEvaluationContext.cpp
  berryIIterable.cpp
  berryIPropertyTester.cpp
  berryIVariableResolver.cpp
  berryPropertyTester.cpp
)

set(INTERNAL_CPP_FILES
  berryAdaptExpression.cpp
  berryAndExpression.cpp
  berryCompositeExpression.cpp
  berryCountExpression.cpp
  berryDefaultVariable.cpp
  berryDefinitionRegistry.cpp
  berryEnablementExpression.cpp
  berryEqualsExpression.cpp
  berryExpressionPlugin.cpp
  berryExpressions.cpp
  berryExpressionStatus.cpp
  berryInstanceofExpression.cpp
  berryIterateExpression.cpp
  berryNotExpression.cpp
  berryOrExpression.cpp
  berryPlatformPropertyTester.cpp
  berryProperty.cpp
  berryPropertyTesterDescriptor.cpp
  berryReferenceExpression.cpp
  berryResolveExpression.cpp
  berryStandardElementHandler.cpp
  berrySystemTestExpression.cpp
  berryTestExpression.cpp
  berryTypeExtension.cpp
  berryTypeExtensionManager.cpp
  berryWithExpression.cpp
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
