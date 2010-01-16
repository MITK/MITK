SET(SRC_CPP_FILES
  berryElementHandler.cpp
  berryEvaluationContext.cpp
  berryEvaluationResult.cpp
  berryExpression.cpp
  berryExpressionConverter.cpp
  berryExpressionInfo.cpp
  berryExpressionTagNames.cpp
  berryPropertyTester.cpp
)

SET(INTERNAL_CPP_FILES
  berryAdaptExpression.cpp
  berryAndExpression.cpp
  berryCompositeExpression.cpp
  berryCountExpression.cpp
  berryDefaultVariable.cpp
  berryDefinitionRegistry.cpp
  berryEnablementExpression.cpp
  berryEqualsExpression.cpp
  berryExpressions.cpp
  berryInstanceofExpression.cpp
  berryIterateExpression.cpp
  berryNotExpression.cpp
  berryOrExpression.cpp
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
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
