SET(CPP_FILES
  cherryElementHandler.cpp
  cherryEvaluationContext.cpp
  cherryEvaluationResult.cpp
  cherryExpression.cpp
  cherryExpressionConverter.cpp
  cherryExpressionInfo.cpp
  cherryExpressionTagNames.cpp
  cherryPropertyTester.cpp
)

SET(INTERNAL_CPP_FILES
  cherryAdaptExpression.cpp
  cherryAndExpression.cpp
  cherryCompositeExpression.cpp
  cherryCountExpression.cpp
  cherryDefaultVariable.cpp
  cherryDefinitionRegistry.cpp
  cherryEnablementExpression.cpp
  cherryEqualsExpression.cpp
  cherryExpressions.cpp
  cherryInstanceofExpression.cpp
  cherryIterateExpression.cpp
  cherryNotExpression.cpp
  cherryOrExpression.cpp
  cherryProperty.cpp
  cherryPropertyTesterDescriptor.cpp
  cherryReferenceExpression.cpp
  cherryResolveExpression.cpp
  cherryStandardElementHandler.cpp
  cherrySystemTestExpression.cpp
  cherryTestExpression.cpp
  cherryTypeExtension.cpp
  cherryTypeExtensionManager.cpp
  cherryWithExpression.cpp
)

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})