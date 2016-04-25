

#include "Geometrie.h"
#include "model/geometrie/BaseGeometrie.h"

using namespace UniLib;
using namespace model;

Geometrie::Geometrie(model::geometrie::BaseGeometrie* geometrieModel)
	: view::Geometrie(geometrieModel), mVAO(0), mRenderMode(GL_TRIANGLE_STRIP), mUsage(GL_STATIC_DRAW), mIndexCount(0)
{

}

Geometrie::~Geometrie()
{
	glDeleteBuffers(2, mGLBufferIDs);
	glDeleteVertexArrays(1, &mVAO);
}

DRReturn Geometrie::uploadToGPU()
{
	geometrie::BaseGeometrie* g = mGeometrieModel;
	// create buffer
	glGenBuffers(2, mGLBufferIDs);
	glGenVertexArrays(1, &mVAO);

	int vertexCount = g->getVertexCount();
	int vertexSize = g->getVertexSize();
	mIndexCount = g->getIndexCount();

	// fill buffer
	glBindBuffer( GL_ARRAY_BUFFER, mGLBufferIDs[0]); 
	glBufferData( GL_ARRAY_BUFFER, sizeof(float)*vertexCount*vertexSize, g->getVertices(), mUsage);

	int cur = 0;
	for(int iv = 0; iv < vertexCount*vertexSize; iv++) {
		//printf("i: %d, float: %f ", iv, g->getVertex(iv));
		cur++;
		if(cur >= vertexSize) {
			cur = 0;
			//printf("\n");
		}
	}
	//printf("\n");

	glBindVertexArray (mVAO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer( GL_ARRAY_BUFFER, mGLBufferIDs[0]); 

	int offset = 0;
	int i = 0;
	for(int y = 0; pow(2, y) < geometrie::GEOMETRIE_MAX; y++) {
		//for(int x = 0; x < GEOMETRIE_MAX; x*=x)  {
		int x = pow(2, y);
		if(x & g->getFormatFlags()) {
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
			glVertexAttribPointer(i, componentCount, GL_FLOAT, GL_FALSE, (vertexSize)*sizeof(GLfloat), (GLvoid *)(offset*sizeof(GLfloat)));
			i++;
			offset += componentCount;
		}
	}
	// fill index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBufferIDs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*mIndexCount, g->getIndices(), mUsage);


	// deactivate buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	if (DRGrafikError("error by uploading geometrie to GPU")) LOG_ERROR("error in geometrie", DR_ERROR);
	return DR_OK;
}

DRReturn Geometrie::render()
{
	if (!mVAO) return DR_OK;
	glBindVertexArray (mVAO);
	//glDrawArrays(mRenderMode, 0, mVertexCount);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBufferIDs[1]);
	glDrawElements(mRenderMode, mIndexCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);//*/
	if(DRGrafikError("base geometrie render")) 
		LOG_ERROR("object could't rendered", DR_ERROR);
	return DR_OK;
}

