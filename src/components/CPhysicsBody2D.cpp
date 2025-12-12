#include "CPhysicsBody2D.h"
#include "SystemLocator.h"
#include "systems/S2DPhysics.h"

namespace Components
{

CPhysicsBody2D::~CPhysicsBody2D()
{
	auto* physics = Systems::SystemLocator::tryPhysics();
	if (!physics)
	{
		return;
	}

	if (b2Body_IsValid(bodyId))
	{
		physics->destroyBody(bodyId);
		bodyId = b2_nullBodyId;
	}

	if (owner.isValid())
	{
		physics->clearFixedUpdateCallback(owner);
	}
}

}  // namespace Components
