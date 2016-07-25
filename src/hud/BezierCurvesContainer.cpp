#include "hud/BezierCurvesContainer.h"

/*
DRVector2*	mPoints;
u16			mPointCount;
u16*		mIndices;
u16			mIndiceCount;
u16			mCursor;
u16			mIndiceCursor;
*/

BezierCurvesContainer::BezierCurvesContainer() :
	mPoints(NULL), mPointCount(0), mIndices(NULL), mIndiceCount(0),
	mCursor(0), mIndiceCursor(0)
{
}
BezierCurvesContainer::~BezierCurvesContainer()
{
	DR_SAVE_DELETE_ARRAY(mPoints);
	DR_SAVE_DELETE_ARRAY(mIndices);
}

void BezierCurvesContainer::init(u16 _indiceCount, u16 _pointCount)
{
	mIndiceCount = _indiceCount;
	mPointCount = _pointCount;
	mPoints = new DRVector2[_pointCount];
	mIndices = new u16[_indiceCount * 2];
	mCursor = 0;
	mIndiceCursor = 0;
}

void BezierCurvesContainer::addCurve(DRBezierCurve* b, bool conturStartCurve/* = false*/)
{
	int firstCurvePoint = conturStartCurve ? 0 : 1;
	mIndices[mIndiceCursor * 2] = mCursor - firstCurvePoint;

	for (int iCurvePoint = firstCurvePoint; iCurvePoint < b->getNodeCount(); iCurvePoint++) {
		if (b->getNodeCount() > 3) {
			LOG_WARNING("bezier kurve has to many points");
			break;
		}
		if (mCursor >= mPointCount) {
			LOG_WARNING("to many points added to array");
			break;
		}
		if (b->getNodeCount() <= iCurvePoint) break;
		mPoints[mCursor++] = (*b)[iCurvePoint];
	}
	mIndices[mIndiceCursor * 2 + 1] = mCursor - 1;
	mIndiceCursor++;
}

void BezierCurvesContainer::scale(DRVector2 scaleFaktor) {
	for (int i = 0; i < mPointCount; i++) {
		mPoints[i] *= scaleFaktor;
	}
}

void BezierCurvesContainer::print()
{
	printf("print Bezier points:\n");
	for (int i = 0; i < mIndiceCursor; i++) {
		for (int iVertex = mIndices[i * 2]; iVertex <= mIndices[i * 2 + 1]; iVertex++) {
			if (iVertex > mIndices[i * 2]) printf(", ");
			printf("(%d) %.2f %.2f", iVertex, mPoints[iVertex].x, mPoints[iVertex].y);
		}
		printf("\n");
	}
	printf("\n");
}

DRBoundingBox BezierCurvesContainer::getBoundingBoxForBezier(int index) {
	DRVector2 minV(100.0f);
	DRVector2 maxV(0.0f);
	for (int i = mIndices[index * 2]; i < mIndices[index * 2 + 1]; i++) {
		minV = min(mPoints[i], minV);
		maxV = max(mPoints[i], maxV);
	}
	return DRBoundingBox(minV, maxV);
}