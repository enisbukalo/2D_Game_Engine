#include "CCollider2D.h"
#include "CPhysicsBody2D.h"
#include "Entity.h"

CCollider2D::CCollider2D()
    : m_shapeId(b2_nullShapeId)
    , m_shapeType(ColliderShape::Circle)
    , m_isSensor(false)
    , m_density(1.0f)
    , m_friction(0.3f)
    , m_restitution(0.0f)
    , m_initialized(false)
{
    m_shapeData.circle.center = {0.0f, 0.0f};
    m_shapeData.circle.radius = 0.5f;
}

CCollider2D::~CCollider2D() {
    destroyShape();
}

void CCollider2D::createCircle(float radius, const b2Vec2& center) {
    m_shapeType = ColliderShape::Circle;
    m_shapeData.circle.center = center;
    m_shapeData.circle.radius = radius;
    attachToBody();
}

void CCollider2D::createBox(float halfWidth, float halfHeight) {
    m_shapeType = ColliderShape::Box;
    m_shapeData.box.halfWidth = halfWidth;
    m_shapeData.box.halfHeight = halfHeight;
    attachToBody();
}

void CCollider2D::attachToBody() {
    if (!getOwner()) {
        return;
    }

    // Get the physics body component
    auto physicsBody = getOwner()->getComponent<CPhysicsBody2D>();
    if (!physicsBody || !physicsBody->isInitialized()) {
        return;
    }

    b2BodyId bodyId = physicsBody->getBodyId();
    if (!b2Body_IsValid(bodyId)) {
        return;
    }

    // Destroy old shape if exists
    destroyShape();

    // Create shape definition
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = m_density;
    // Note: friction and restitution are set per-shape, not in shapeDef for Box2D v3
    shapeDef.enableSensorEvents = m_isSensor;

    // Create the appropriate shape
    switch (m_shapeType) {
        case ColliderShape::Circle: {
            b2Circle circle;
            circle.center = m_shapeData.circle.center;
            circle.radius = m_shapeData.circle.radius;
            m_shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
            break;
        }

        case ColliderShape::Box: {
            b2Polygon box = b2MakeBox(m_shapeData.box.halfWidth, m_shapeData.box.halfHeight);
            m_shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
            break;
        }

        case ColliderShape::Polygon:
            // TODO: Support arbitrary polygons in future
            break;
    }

    m_initialized = b2Shape_IsValid(m_shapeId);

    // Set friction and restitution after creation
    if (m_initialized) {
        b2Shape_SetFriction(m_shapeId, m_friction);
        b2Shape_SetRestitution(m_shapeId, m_restitution);
    }
}

void CCollider2D::destroyShape() {
    if (m_initialized && b2Shape_IsValid(m_shapeId)) {
        b2DestroyShape(m_shapeId, true);
        m_shapeId = b2_nullShapeId;
        m_initialized = false;
    }
}

void CCollider2D::setIsSensor(bool isSensor) {
    m_isSensor = isSensor;
    if (m_initialized && b2Shape_IsValid(m_shapeId)) {
        b2Shape_EnableSensorEvents(m_shapeId, isSensor);
    }
}

void CCollider2D::setDensity(float density) {
    m_density = density;
    if (m_initialized && b2Shape_IsValid(m_shapeId)) {
        b2Shape_SetDensity(m_shapeId, density, true);

        // Note: updateBodyMass parameter in SetDensity already updates mass
    }
}

void CCollider2D::setFriction(float friction) {
    m_friction = friction;
    if (m_initialized && b2Shape_IsValid(m_shapeId)) {
        b2Shape_SetFriction(m_shapeId, friction);
    }
}

void CCollider2D::setRestitution(float restitution) {
    m_restitution = restitution;
    if (m_initialized && b2Shape_IsValid(m_shapeId)) {
        b2Shape_SetRestitution(m_shapeId, restitution);
    }
}

float CCollider2D::getCircleRadius() const {
    if (m_shapeType == ColliderShape::Circle) {
        return m_shapeData.circle.radius;
    }
    return 0.0f;
}

b2Vec2 CCollider2D::getCircleCenter() const {
    if (m_shapeType == ColliderShape::Circle) {
        return m_shapeData.circle.center;
    }
    return {0.0f, 0.0f};
}

float CCollider2D::getBoxHalfWidth() const {
    if (m_shapeType == ColliderShape::Box) {
        return m_shapeData.box.halfWidth;
    }
    return 0.0f;
}

float CCollider2D::getBoxHalfHeight() const {
    if (m_shapeType == ColliderShape::Box) {
        return m_shapeData.box.halfHeight;
    }
    return 0.0f;
}

void CCollider2D::serialize(JsonBuilder& builder) const {
    builder.beginObject();
    builder.addKey("cCollider2D");
    builder.beginObject();

    // Shape type
    builder.addKey("shapeType");
    switch (m_shapeType) {
        case ColliderShape::Circle:
            builder.addString("Circle");
            break;
        case ColliderShape::Box:
            builder.addString("Box");
            break;
        case ColliderShape::Polygon:
            builder.addString("Polygon");
            break;
    }

    // Shape data
    if (m_shapeType == ColliderShape::Circle) {
        builder.addKey("radius");
        builder.addNumber(m_shapeData.circle.radius);
        builder.addKey("centerX");
        builder.addNumber(m_shapeData.circle.center.x);
        builder.addKey("centerY");
        builder.addNumber(m_shapeData.circle.center.y);
    } else if (m_shapeType == ColliderShape::Box) {
        builder.addKey("halfWidth");
        builder.addNumber(m_shapeData.box.halfWidth);
        builder.addKey("halfHeight");
        builder.addNumber(m_shapeData.box.halfHeight);
    }

    // Fixture properties
    builder.addKey("isSensor");
    builder.addBool(m_isSensor);
    builder.addKey("density");
    builder.addNumber(m_density);
    builder.addKey("friction");
    builder.addNumber(m_friction);
    builder.addKey("restitution");
    builder.addNumber(m_restitution);

    builder.endObject();
    builder.endObject();
}

void CCollider2D::deserialize(const JsonValue& value) {
    if (!value.isObject()) return;

    const auto& collider = value["cCollider2D"];
    if (!collider.isObject()) return;

    // Shape type
    const auto& shapeTypeValue = collider["shapeType"];
    if (shapeTypeValue.isString()) {
        std::string typeStr = shapeTypeValue.getString();
        if (typeStr == "Circle") {
            m_shapeType = ColliderShape::Circle;

            const auto& radiusValue = collider["radius"];
            if (radiusValue.isNumber()) {
                m_shapeData.circle.radius = static_cast<float>(radiusValue.getNumber());
            }

            const auto& centerXValue = collider["centerX"];
            if (centerXValue.isNumber()) {
                m_shapeData.circle.center.x = static_cast<float>(centerXValue.getNumber());
            }

            const auto& centerYValue = collider["centerY"];
            if (centerYValue.isNumber()) {
                m_shapeData.circle.center.y = static_cast<float>(centerYValue.getNumber());
            }
        } else if (typeStr == "Box") {
            m_shapeType = ColliderShape::Box;

            const auto& halfWidthValue = collider["halfWidth"];
            if (halfWidthValue.isNumber()) {
                m_shapeData.box.halfWidth = static_cast<float>(halfWidthValue.getNumber());
            }

            const auto& halfHeightValue = collider["halfHeight"];
            if (halfHeightValue.isNumber()) {
                m_shapeData.box.halfHeight = static_cast<float>(halfHeightValue.getNumber());
            }
        }
    }

    // Fixture properties
    const auto& isSensorValue = collider["isSensor"];
    if (isSensorValue.isBool()) {
        m_isSensor = isSensorValue.getBool();
    }

    const auto& densityValue = collider["density"];
    if (densityValue.isNumber()) {
        m_density = static_cast<float>(densityValue.getNumber());
    }

    const auto& frictionValue = collider["friction"];
    if (frictionValue.isNumber()) {
        m_friction = static_cast<float>(frictionValue.getNumber());
    }

    const auto& restitutionValue = collider["restitution"];
    if (restitutionValue.isNumber()) {
        m_restitution = static_cast<float>(restitutionValue.getNumber());
    }
}

std::string CCollider2D::getType() const {
    return "CCollider2D";
}
