#include "UniformSet.h"
#include "ShaderProgram.h"


UniformSet::UniformSet()
{

}

UniformSet::~UniformSet()
{

}

DRReturn UniformSet::addLocationToUniform(const char* name, ShaderProgram* program)
{
	assert(program != NULL && program->getProgram() != 0);
	return addUniformMapping(name, (void*)glGetUniformLocation(program->getProgram(), name), program->getProgram());
}
DRReturn UniformSet::removeLocationFromUniform(const char* name, ShaderProgram* program)
{
	assert(program != NULL && program->getProgram() != 0);
	return removeUniformMapping(name, program->getProgram());
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

void UniformSet::updateUniforms(ShaderProgram* program)
{
	lock();
	if(!isDirty()) {
		unlock();
		return;
	}
	for(std::map<HASH, UniformEntry*>::iterator it = mUniformEntrys.begin(); it != mUniformEntrys.end(); it++) {
		UniformEntry* entry = it->second;
		UniformEntry::Location* l = &entry->locations[program->getProgram()];
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
}
