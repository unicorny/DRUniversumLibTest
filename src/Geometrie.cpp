

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
	Uint32 startTicks = SDL_GetTicks();
	geometrie::BaseGeometrie* g = mGeometrieModel;
	// create buffer
	glGenBuffers(2, mGLBufferIDs);
 
	int vertexCount = g->getVertexCount();
	int vertexSize = g->getVertexSize();
	mIndexCount = g->getIndexCount();

	// fill buffer
	glBindBuffer( GL_ARRAY_BUFFER, mGLBufferIDs[0]); 
	glBufferData( GL_ARRAY_BUFFER, sizeof(float)*vertexCount*vertexSize, g->getVertices(), mUsage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// fill index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBufferIDs[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*mIndexCount, g->getIndices(), mUsage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// vao
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mGLBufferIDs[0]);
	glEnableVertexAttribArray(0);
	int offset = 0;
	int i = 0;
	for (int y = 0; pow(2, y) < geometrie::GEOMETRIE_MAX; y++) {
		//for(int x = 0; x < GEOMETRIE_MAX; x*=x)  {
		int x = pow(2, y);
		if (x & g->getFormatFlags()) {
			int componentCount = 0;
			if (x & geometrie::GEOMETRIE_2DVECTOR) {
				componentCount = 2;
			}
			else if (x & geometrie::GEOMETRIE_3DVECTOR) {
				componentCount = 3;
			}
			else if (x & geometrie::GEOMETRIE_4DVECTOR) {
				componentCount = 4;
			}
			glVertexAttribPointer(i, componentCount, GL_FLOAT, GL_FALSE, (vertexSize)*sizeof(GLfloat), (GLvoid *)(offset*sizeof(GLfloat)));
			i++;
			offset += componentCount;
		}
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBufferIDs[1]);
	glBindVertexArray(0);

	// deactivate buffer
	
	DR_SAVE_DELETE(mGeometrieModel);
	//glBindVertexArray(0);
	if (DRGrafikError("error by uploading geometrie to GPU")) LOG_ERROR("error in geometrie", DR_ERROR);
	return DR_OK;
}

DRReturn Geometrie::render()
{
	if (!mVAO) return DR_OK;
	glBindVertexArray (mVAO);
	//glDrawArrays(mRenderMode, 0, mVertexCount);
	//glBindBuffer(GL_ARRAY_BUFFER, mGLBufferIDs[0]);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLBufferIDs[1]);
	try {
		if (DRGrafikError("base geometrie render")) LOG_WARNING("error before call to glDrawElements");
		glDrawElements(mRenderMode, mIndexCount, GL_UNSIGNED_INT, 0);
	}
	catch (...)
	{
		LOG_WARNING("exception occured by draw element");
	}
	//glBindBuffer(GL_ARRAY_BUFFER, 0);//*/
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);//*/
	glBindVertexArray(0);
	if(DRGrafikError("base geometrie render")) 
		LOG_ERROR("object could't rendered", DR_ERROR);
	return DR_OK;
}

