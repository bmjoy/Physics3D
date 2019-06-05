#include "part.h"

Part::Part(const Shape& shape, const CFrame& position, double density, double friction) : hitbox(shape), cframe(position), properties({density, friction}) {
	this->maxRadius = hitbox.getMaxRadius();
	this->mass = this->hitbox.getVolume() * this->properties.density;
	this->inertia = this->hitbox.getInertia() * this->properties.density;
	this->localCenterOfMass = this->hitbox.getCenterOfMass();
}

/*Part::Part(VisualShape shape, CFrame position, double density, double friction) : properties({density, friction}) {
	CFramef backTransform;
	Vec3f* normalBuf = (shape.normals) ? new Vec3f[shape.vertexCount] : nullptr;
	NormalizedShape normalized = shape.normalized(new Vec3f[shape.vertexCount], normalBuf, backTransform);

	CFrame realCFrame = position.localToGlobal(CFrame(backTransform));

	this->hitbox = normalized;
	this->cframe = realCFrame;
	this->maxRadius = hitbox.getMaxRadius();

	this->mass = this->hitbox.getVolume() * this->properties.density;
	this->inertia = this->hitbox.getInertia() * this->properties.density;
}*/

bool Part::intersects(const Part& other, Vec3& intersection, Vec3& exitVector) const {
#ifdef USE_TRANSFORMATIONS
	Vec3f intersectionF, exitVectorF;
	if(this->transformed.intersects(other.transformed, intersectionF, exitVectorF, other.cframe.position - this->cframe.position)){
		intersection = Vec3(intersectionF);
		exitVector = Vec3(exitVectorF);
		return true;
	}
	return false;
#else
	const CFramef relativeCFrame(this->cframe.globalToLocal(other.cframe));
	Vec3f localIntersection, localExitVector;
	if(this->hitbox.intersectsTransformed(other.hitbox, relativeCFrame, localIntersection, localExitVector)) {
		intersection = this->cframe.localToGlobal(Vec3(localIntersection));
		exitVector = this->cframe.localToRelative(Vec3(localExitVector));
		return true;
	}
	return false;
#endif
}
