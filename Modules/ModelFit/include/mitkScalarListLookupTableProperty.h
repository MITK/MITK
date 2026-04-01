/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file mitkScalarListLookupTableProperty.h
 * \brief Declares the ScalarListLookupTableProperty, a mitk::GenericProperty wrapping
 *        a ScalarListLookupTable for storage in MITK property lists.
 *
 * This property type is primarily used by the model fit framework to persist maps
 * of named scalar lists (e.g. static model parameters) as node or data properties.
 *
 * \sa mitk::ScalarListLookupTable, mitk::GenericProperty
 * \ingroup ModelFit
 */

#ifndef mitkScalarListLookupTableProperty_h
#define mitkScalarListLookupTableProperty_h

#include <MitkModelFitExports.h>
#include <mitkGenericProperty.h>
#include <mitkScalarListLookupTable.h>

namespace mitk
{
    /**
     * \brief Property type that holds a ScalarListLookupTable value.
     *
     * Generated via the mitkDeclareGenericProperty macro, which creates a
     * BaseProperty subclass with value semantics for ScalarListLookupTable.
     *
     * \sa mitk::ScalarListLookupTable
     */
    mitkDeclareGenericProperty(ScalarListLookupTableProperty, ScalarListLookupTable,
                               MITKMODELFIT_EXPORT);
}


#endif
