#include "intersection.h"

#include "genericIntersection.h"
#include "../physicsProfiler.h"
#include "../profiling.h"
#include "computationBuffer.h"

#include "shape.h"
#include "polyhedron.h"

#include "../misc/validityHelper.h"
#include "shapeClass.h"

#include <algorithm>

Intersection intersectsTransformed(const Shape& first, const Shape& second, const CFrame& relativeTransform) {
	return intersectsTransformed(*first.baseShape, *second.baseShape, relativeTransform, first.scale, second.scale);
}


static void incDebugTally(HistoricTally<long long, IterationTime>& tally, int iterTime) {
	if(iterTime >= 200) {
		tally.addToTally(IterationTime::LIMIT_REACHED, 1);
	} else if(iterTime >= 15) {
		tally.addToTally(IterationTime::TOOMANY, 1);
	} else {
		tally.addToTally(static_cast<IterationTime>(iterTime), 1);
	}
}


ComputationBuffers buffers(1000, 2000);

Intersection intersectsTransformed(const GenericCollidable& first, const GenericCollidable& second, const CFrame& relativeTransform, const DiagonalMat3& scaleFirst, const DiagonalMat3& scaleSecond) {
	Tetrahedron result;
	int iter;
	ColissionPair info{first, second, relativeTransform, scaleFirst, scaleSecond};
	physicsMeasure.mark(PhysicsProcess::GJK_COL);
	bool collides = runGJKTransformed(info, -relativeTransform.position, result, iter);

	if(collides) {
		incDebugTally(GJKCollidesIterationStatistics, iter + 2);
		physicsMeasure.mark(PhysicsProcess::EPA);
		Vec3f intersection;
		Vec3f exitVector;

		if(!isfinite(result.A.p.x) || !isfinite(result.A.p.y) || !isfinite(result.A.p.z)) {
			intersection = Vec3f(0.0f, 0.0f, 0.0f);
			exitVector = Vec3f(std::min(scaleFirst[0], std::min(scaleFirst[1], std::min(scaleFirst[2], std::min(scaleSecond[0], std::min(scaleSecond[1], scaleSecond[2]))))),
							   0.0f, 0.0f);

			return Intersection(intersection, exitVector);
		}

		assert(isVecValid(result.A.p));
		assert(isVecValid(result.A.originFirst));
		assert(isVecValid(result.A.originSecond));
		assert(isVecValid(result.B.p));
		assert(isVecValid(result.B.originFirst));
		assert(isVecValid(result.B.originSecond));
		assert(isVecValid(result.C.p));
		assert(isVecValid(result.C.originFirst));
		assert(isVecValid(result.C.originSecond));
		assert(isVecValid(result.D.p));
		assert(isVecValid(result.D.originFirst));
		assert(isVecValid(result.D.originSecond));

		bool epaResult = runEPATransformed(info, result, intersection, exitVector, buffers, iter);

		assert(isVecValid(exitVector));
		incDebugTally(EPAIterationStatistics, iter);
		if(!epaResult) {
			return Intersection();
		} else {
			return Intersection(intersection, exitVector);
		}
	} else {
		incDebugTally(GJKNoCollidesIterationStatistics, iter + 2);
		physicsMeasure.mark(PhysicsProcess::OTHER, PhysicsProcess::GJK_NO_COL);
		return Intersection();
	}
}
