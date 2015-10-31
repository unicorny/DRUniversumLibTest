

#include "BaseGeometrieContainer.h"

using namespace UniLib;
using namespace model;

BaseGeometrieContainer::BaseGeometrieContainer()

{

}

BaseGeometrieContainer::~BaseGeometrieContainer()
{
	glDeleteBuffers(2, mGLBufferIDs);
	glDeleteVertexArrays(1, &mVAO);
}

DRReturn BaseGeometrieContainer::uploadToGPU(GLenum usage)
{
	// create buffer
	glGenBuffers(2, mGLBufferIDs);
	glGenVertexArrays(1, &mVAO);

	// fill buffer
	glBindBuffer( GL_ARRAY_BUFFER, mGLBufferIDs[0]); 
	glBufferData( GL_ARRAY_BUFFER, sizeof(float)*mVertexCount*mVertexSize, mVertices, usage);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBufferIDs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*mIndiceCount, mIndicesArray, usage);

	

	glBindVertexArray (mVAO);
	glEnableVertexAttribArray(0);

	int offset = 0;
	int i = 0;
	for(int x = 1; x < geometrie::GEOMETRIE_MAX; x*=x)  {
		if(x & mVertexFormatFlags) {
			int componentCount = 0;
			if(x & geometrie::GEOMETRIE_2DVECTOR) {
				componentCount = 2;
			}
			else if(x & geometrie::GEOMETRIE_3DVECTOR) {
				componentCount = 3;
			}
			else if(x & geometrie::GEOMETRIE_4DVECTOR) {
				componentCount = 4;
			}
			glVertexAttribPointer(i, componentCount, GL_FLOAT, GL_FALSE, sizeof(float)*mVertexSize, (GLvoid *)offset);
			i++;
			offset += componentCount;
		}
	}

	// deactivate buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return DR_OK;
}

void BaseGeometrieContainer::render()
{
	glBindVertexArray (mVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBufferIDs[1]);
	glDrawElements(mRenderMode, mIndiceCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

