/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkFreeSurferParcellationHandler.h"
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkImageRegionAccessor.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkNodePredicateDataType.h>
#include <mitkMousePressEvent.h>
#include <mitkMouseWheelEvent.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleRegistry.h>
#include <usServiceProperties.h>
#include <string>
using namespace mitk;
using namespace std;
US_USE_NAMESPACE

QmitkFreeSurferParcellationHandler::QmitkFreeSurferParcellationHandler() :
  m_LastPixelvalue( 0 ),
  m_Translator( mitk::FreeSurferParcellationTranslator::New() )
{
  us::ServiceProperties props;
  props["name"] = std::string("ParcellationHandler");
  us::ModuleContext* context = us::ModuleRegistry::GetModule(1)->GetModuleContext();
  m_ServiceRegistration = context->RegisterService<InteractionEventObserver>( this, props);
}
QmitkFreeSurferParcellationHandler::~QmitkFreeSurferParcellationHandler()
{
  m_ServiceRegistration.Unregister();
}

void QmitkFreeSurferParcellationHandler::Notify(InteractionEvent *interactionEvent, bool isHandled)
{
  Q_UNUSED( isHandled )
  typedef itk::VectorContainer<unsigned int, mitk::DataNode::Pointer> SetOfObjects;
  BaseRenderer* sender = interactionEvent->GetSender();
  InteractionPositionEvent* positionEvent = static_cast<InteractionPositionEvent*>(interactionEvent);
  TNodePredicateDataType<Image>::Pointer isImageData = TNodePredicateDataType<Image>::New();
  DataStorage::SetOfObjects::ConstPointer nodes = sender->GetDataStorage()->GetSubset(isImageData).GetPointer();
  if(nodes.IsNull() || nodes->size() <= 0)
    return;
  Point3D worldposition = positionEvent->GetPositionInWorld();
  for (unsigned int x = 0; x < nodes->size(); x++)
  {
    DataNode::Pointer node = static_cast<DataNode::Pointer>( nodes->at(x) );
    if(node.IsNotNull())
    {
        Image::Pointer image = dynamic_cast<Image*>( node->GetData() );
        if( image.IsNotNull() && image->GetGeometry()->IsInside(worldposition) )
        {
          string typeStr = image->GetPixelType().GetComponentTypeAsString();
          int value = 0;
          try
          {
            mitk::ImageRegionAccessor readAccess(image);
            itk::Index<3> idx;
            image->GetGeometry()->WorldToIndex(worldposition, idx);
            if( typeStr == "int" )
            {
              value = static_cast<int>( *(int*)readAccess.getPixel( idx ) );
            }
            else if( typeStr == "unsigned_char" )
            {
              value = static_cast<int>( *(unsigned char*)readAccess.getPixel( idx ) );
            }
            else
            {
              MITK_WARN("QmitkFreeSurferParcellationHandler") << "Pixeltype '" << typeStr << "' is not implemented yet.";
              return;
            }
            emit this->changed( value );
            QString name( QString::fromStdString( this->m_Translator->GetName( value ) ) );
            emit this->changed( name );
            MousePressEvent::Pointer mouseEvent = dynamic_cast<MousePressEvent*>( interactionEvent );
            MouseWheelEvent::Pointer wheelEvent = dynamic_cast<MouseWheelEvent*>( interactionEvent );
            if(mouseEvent.IsNotNull())
            {
              emit this->clicked( value );
              emit this->clicked( name );
            }
            if( wheelEvent.IsNotNull() )
            {
              emit this->scrolled( value );
              emit this->scrolled( name );
            }
            return; // exit loop
          }
          catch( const Exception& ex )
          {
            MITK_WARN("QmitkFreeSurferParcellationHandler") << "Could not access image for reading pixelvalue due to: " << ex.GetDescription();
          }
          catch(...)
          {
            MITK_WARN("QmitkFreeSurferParcellationHandler") << "Could not access image for reading pixelvalue.";
          }
        }
    }
  }
}
