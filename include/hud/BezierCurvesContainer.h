#ifndef __DR_MICRO_SPACECRAFT_BEZIER_CURVES_CONTAINER_H
#define __DR_MICRO_SPACECRAFT_BEZIER_CURVES_CONTAINER_H

#include "UniversumLib.h"

class BezierCurvesContainer
{
public:
	BezierCurvesContainer();
	~BezierCurvesContainer();

	void init(u16 _indiceCount, u16 _pointCount);
	void addCurve(DRBezierCurve* b, bool conturStartCurve = false);
	void scale(DRVector2 scaleFaktor);
	void print();

	DRBoundingBox getBoundingBoxForBezier(int index);

	// getter and setter
	__inline__ u16 getIndexCount() { return mIndiceCount; }
	__inline__ u16 getPointCount() { return mPointCount; }

protected:
	DRVector2*	mPoints;
	u16			mPointCount;
	u16*		mIndices;
	u16			mIndiceCount;
	u16			mCursor;
	u16			mIndiceCursor;
};

#endif //__DR_MICRO_SPACECRAFT_BEZIER_CURVES_CONTAINER_H
