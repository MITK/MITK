#include "BaseRenderer.h"

//##ModelId=3D6A1791038B
void mitk::BaseRenderer::SetData(mitk::DataTreeIterator* iterator)
{
	m_DataTreeIterator = iterator;
	Modified();
}

//##ModelId=3E3314720003
void mitk::BaseRenderer::SetWindowId(void *id)
{
}

//##ModelId=3E330C4D0395
const MapperSlotId mitk::BaseRenderer::defaultMapper = 1;

//##ModelId=3E33162C00D0
void mitk::BaseRenderer::PaintGL()
{
}

//##ModelId=3E331632031E
void mitk::BaseRenderer::InitializeGL()
{
}

//##ModelId=3E33163703D9
void mitk::BaseRenderer::ResizeGL(int w, int h)
{
}

//##ModelId=3E33163A0261
void mitk::BaseRenderer::InitRenderer()
{
}

//##ModelId=3E3799250397
void mitk::BaseRenderer::InitSize(int w, int h)
{
}

