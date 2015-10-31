

#include "BaseGeometrieContainer.h"

using namespace UniLib;
using namespace model;

BaseGeometrieContainer::BaseGeometrieContainer()
: mGLBuffers(NULL), mGLBufferCount(0)
{

}

BaseGeometrieContainer::~BaseGeometrieContainer()
{
	glDeleteBuffers(mGLBufferCount, mGLBuffers);
	DR_SAVE_DELETE_ARRAY(mGLBuffers);
	mGLBufferCount = 0;
}

void BaseGeometrieContainer::uploadToGPU()
{
	mGLBufferCount = 0;
	if(mVertexFormatFlags & geometrie::GEOMETRIE_VERTICES) mGLBufferCount++;
	if(mVertexFormatFlags & geometrie::GEOMETRIE_COLORS) mGLBufferCount++;
	if(mVertexFormatFlags & geometrie::GEOMETRIE_NORMALS) mGLBufferCount++;
	if(mVertexFormatFlags & geometrie::GEOMETRIE_TEX2D_1) mGLBufferCount++;
	mGLBuffers = new GLuint[mGLBufferCount];
	glGenBuffers(mGLBufferCount, mGLBuffers);
}

