#include "UniformSet.h"
#include "ShaderProgram.h"

using namespace UniLib;

UniformSet::UniformSet()
{

}

UniformSet::~UniformSet()
{

}

DRReturn UniformSet::addLocationToUniform(const char* name, model::ShaderProgram* program)
{
	assert(program != NULL && program->getID() != 0);
	ShaderProgram* sp = static_cast<ShaderProgram*>(program);
	if (sp->checkLoadingState() != LOADING_STATE_FULLY_LOADED) {
		lock();
		mWaitingUniformEntrys.push(NotYetReadyUniformEntry(name, sp));
		unlock();
		return DR_OK;
	}
	else {
		return addUniformMapping(name, (void*)glGetUniformLocation(sp->getProgram(), name), program->getID());
	}
}
DRReturn UniformSet::removeLocationFromUniform(const char* name, model::ShaderProgram* program)
{
	assert(program != NULL && program->getID() != 0);
	return removeUniformMapping(name, program->getID());
}
// uniform update
// for function pointer
__inline__ void setGlUniform1f(GLint loc, GLfloat* vvalue) {glUniform1fv(loc, 1, vvalue);}
__inline__ void setGlUniform2f(GLint loc, GLfloat* vvalue) {glUniform2fv(loc, 1, vvalue);}
__inline__ void setGlUniform3f(GLint loc, GLfloat* vvalue) {glUniform3fv(loc, 1, vvalue);}
__inline__ void setGlUniform4f(GLint loc, GLfloat* vvalue) {glUniform4fv(loc, 1, vvalue);}
__inline__ void setGlUniformMatrix4f(GLint loc, GLfloat* vvalue) {glUniformMatrix4fv(loc, 1, false, vvalue);}

__inline__ void setGlUniform1i(GLint loc, GLint* vvalue) {glUniform1iv(loc, 1, vvalue);}
__inline__ void setGlUniform2i(GLint loc, GLint* vvalue) {glUniform2iv(loc, 1, vvalue);}
__inline__ void setGlUniform3i(GLint loc, GLint* vvalue) {glUniform3iv(loc, 1, vvalue);}
__inline__ void setGlUniform4i(GLint loc, GLint* vvalue) {glUniform4iv(loc, 1, vvalue);}

void UniformSet::updateUniforms(model::ShaderProgram* program)
{
	lock();
	if(!isDirty()) {
		unlock();
		return;
	}
	while (!mWaitingUniformEntrys.empty()) {
		NotYetReadyUniformEntry e = mWaitingUniformEntrys.front();
		mWaitingUniformEntrys.pop();
		ShaderProgram* sp = e.shader;
		addUniformMapping(e.name.data(), (void*)glGetUniformLocation(sp->getProgram(), e.name.data()), sp->getID());
	}
	for(std::map<HASH, UniformEntry*>::iterator it = mUniformEntrys.begin(); it != mUniformEntrys.end(); it++) {
		UniformEntry* entry = it->second;
		if(entry->locations.find(program->getID()) == entry->locations.end()) continue;
		UniformEntry::Location* l = &entry->locations[program->getID()];
		if(!l->dirty) continue;
		void (*ffunc)(GLint, GLfloat*) = NULL;
		void (*ifunc)(GLint, GLint*) = NULL;
		if(entry->isFloat()) {
			switch(entry->getArraySize()) {
			case 1: ffunc = setGlUniform1f; break;
			case 2: ffunc = setGlUniform2f; break;
			case 3: ffunc = setGlUniform3f; break;
			case 4: ffunc = setGlUniform4f; break;
			case 16: ffunc = setGlUniformMatrix4f; break;
			}
		} else {
			switch(entry->getArraySize()) {
			case 1: ifunc = setGlUniform1i; break;
			case 2: ifunc = setGlUniform2i; break;
			case 3: ifunc = setGlUniform3i; break;
			case 4: ifunc = setGlUniform4i; break;
			}
		}
		
		
		if(entry->isFloat()) {
			ffunc((int)(l->location), entry->floatArray);
		} else {
			ifunc((int)(l->location), entry->intArray);
		}
		l->dirty = false;
	}
	//unsetDirty();
	unlock();
	DRGrafikError("error by updating uniforms");
}
