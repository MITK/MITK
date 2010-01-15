/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryEvaluationService.h"

namespace cherry {

  IEvaluationReference::Pointer EvaluationService::AddEvaluationListener(Expression::Pointer expression,
      IPropertyChangeListener::Pointer listener, const std::string& property) {
    IEvaluationReference::Pointer expressionReference(new EvaluationReference(
        expression, listener, property));

    evaluationAuthority.AddEvaluationListener(expressionReference);
    return expressionReference;
  }

  void EvaluationService::AddEvaluationReference(IEvaluationReference::Pointer ref) {
    poco_assert(ref.Cast<EvaluationReference>()); // "Invalid type"
    evaluationAuthority.AddEvaluationListener(ref);
  }

  void EvaluationService::RemoveEvaluationListener(IEvaluationReference::Pointer ref) {
    evaluationAuthority.RemoveEvaluationListener(ref);
  }

  void EvaluationService::AddSourceProvider(ISourceProvider::Pointer provider) {
    evaluationAuthority.AddSourceProvider(provider);
  }

  void EvaluationService::RemoveSourceProvider(ISourceProvider::Pointer provider) {
    evaluationAuthority.RemoveSourceProvider(provider);
  }

  void EvaluationService::Dispose() {
    evaluationAuthority.Dispose();
  }

  IEvaluationContext::ConstPointer EvaluationService::GetCurrentState() {
    return evaluationAuthority.GetCurrentState();
  }

  void EvaluationService::AddServiceListener(IPropertyChangeListener::Pointer listener) {
    evaluationAuthority.AddServiceListener(listener);
  }

  void EvaluationService::RemoveServiceListener(IPropertyChangeListener::Pointer listener) {
    evaluationAuthority.RemoveServiceListener(listener);
  }

  void EvaluationService::RequestEvaluation(const std::string& propertyName) {
    std::vector<std::string> sources;
    sources.push_back(propertyName);
    evaluationAuthority.SourceChanged(sources);
  }

}
