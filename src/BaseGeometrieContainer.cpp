

#include "BaseGeometrieContainer.h"

using namespace UniLib;
using namespace model;

BaseGeometrieContainer::BaseGeometrieContainer()
	: mRenderMode(GL_TRIANGLE_STRIP), mUsage(GL_STATIC_DRAW)
{

}

BaseGeometrieContainer::~BaseGeometrieContainer()
{
	glDeleteBuffers(2, mGLBufferIDs);
	glDeleteVertexArrays(1, &mVAO);
}

DRReturn BaseGeometrieContainer::uploadToGPU()
{
	// create buffer
	glGenBuffers(2, mGLBufferIDs);
	glGenVertexArrays(1, &mVAO);

	// fill buffer
	glBindBuffer( GL_ARRAY_BUFFER, mGLBufferIDs[0]); 
	glBufferData( GL_ARRAY_BUFFER, sizeof(float)*mVertexCount*mVertexSize, mVertices, mUsage);

	int cur = 0;
	for(int iv = 0; iv < mVertexCount*mVertexSize; iv++) {
		printf("i: %d, float: %f ", iv, mVertices[iv]);
		cur++;
		if(cur >= mVertexSize) {
			cur = 0;
			printf("\n");
		}
	}
	printf("\n");

	glBindVertexArray (mVAO);
	glEnableVertexAttribArray(0);
	glBindBuffer( GL_ARRAY_BUFFER, mGLBufferIDs[0]); 

	int offset = 0;
	int i = 0;
	for(int y = 0; pow(2, y) < geometrie::GEOMETRIE_MAX; y++) {
		//for(int x = 0; x < GEOMETRIE_MAX; x*=x)  {
		int x = pow(2, y);
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
			glVertexAttribPointer(i, componentCount, GL_FLOAT, GL_FALSE, mVertexSize-componentCount, (GLvoid *)offset);
			i++;
			offset += componentCount;
		}
	}
	// fill index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBufferIDs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*mIndiceCount, mIndicesArray, mUsage);


	// deactivate buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return DR_OK;
}

DRReturn BaseGeometrieContainer::render()
{
	glBindVertexArray (mVAO);
	//glDrawArrays(mRenderMode, 0, mVertexCount);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBufferIDs[1]);
	glDrawElements(mRenderMode, mIndiceCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);//*/
	if(DRGrafikError("base geometrie render")) 
		LOG_ERROR("object could't rendered", DR_ERROR);
	return DR_OK;
}

