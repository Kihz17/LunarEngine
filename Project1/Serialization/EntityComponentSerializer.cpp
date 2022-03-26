#include "EntityComponentSerializer.h"
#include "RigidBody.h"
#include "BulletUtils.h"
#include "AnimatedMesh.h"

EntityComponentSerializer::EntityComponentSerializer(Component* c, Entity* entity)
	: component(c),
	entity(entity)
{

}
void EntityComponentSerializer::Serialize(YAML::Emitter& emitter)
{
	emitter << YAML::BeginMap;

	if (dynamic_cast<PositionComponent*>(component))
	{
		SavePositionComponent(emitter, dynamic_cast<PositionComponent*>(component));
	}
	else if (dynamic_cast<RotationComponent*>(component))
	{
		SaveRotationComponent(emitter, dynamic_cast<RotationComponent*>(component));
	}
	else if (dynamic_cast<ScaleComponent*>(component))
	{
		SaveScaleComponent(emitter, dynamic_cast<ScaleComponent*>(component));
	}
	else if (dynamic_cast<RenderComponent*>(component))
	{
		SaveRenderComponent(emitter, dynamic_cast<RenderComponent*>(component));
	}
	else if (dynamic_cast<RigidBodyComponent*>(component))
	{
		SaveRigidComponent(emitter, dynamic_cast<RigidBodyComponent*>(component));
	}

	emitter << YAML::EndMap;
}

void EntityComponentSerializer::Deserialize(YAML::Node& node)
{

}

void EntityComponentSerializer::SavePositionComponent(YAML::Emitter& emitter, PositionComponent* posComp)
{
	emitter << YAML::Key << "ComponentType" << YAML::Value << "Position";
	emitter << YAML::Key << "Position" << YAML::Value << posComp->value;
}

void EntityComponentSerializer::SaveRotationComponent(YAML::Emitter& emitter, RotationComponent* rotComp)
{
	emitter << YAML::Key << "ComponentType" << YAML::Value << "Rotation";
	emitter << YAML::Key << "Rotation" << YAML::Value << rotComp->value;
}

void EntityComponentSerializer::SaveScaleComponent(YAML::Emitter& emitter, ScaleComponent* scaleComp)
{
	emitter << YAML::Key << "ComponentType" << YAML::Value << "Scale";
	emitter << YAML::Key << "Scale" << YAML::Value << scaleComp->value;
}

void EntityComponentSerializer::SaveRenderComponent(YAML::Emitter& emitter, RenderComponent* renderComp)
{
	emitter << YAML::Key << "ComponentType" << YAML::Value << "Render";
	bool animated = dynamic_cast<AnimatedMesh*>(renderComp->mesh);
	emitter << YAML::Key << "Mesh" << YAML::Value << renderComp->mesh->GetPath();
	emitter << YAML::Key << "Animated" << YAML::Value << animated;
	emitter << YAML::Key << "IsColorOverride" << YAML::Value << renderComp->isColorOverride;
	emitter << YAML::Key << "ColorOverride" << YAML::Value << renderComp->colorOverride;
	emitter << YAML::Key << "AlbedoTextures" << YAML::Value << YAML::BeginSeq;
	for (std::pair<ITexture*, float>& p : renderComp->albedoTextures)
	{
		emitter << YAML::BeginMap;
		emitter << YAML::Key << "Path" << YAML::Value << p.first->GetPath();
		emitter << YAML::Key << "FilterType" << YAML::Value << (int)p.first->GetFilterType();
		emitter << YAML::Key << "WrapType" << YAML::Value << (int)p.first->GetWrapType();
		emitter << YAML::Key << "Strenth" << YAML::Value << p.second;
		emitter << YAML::EndMap;
	}
	emitter << YAML::EndSeq;

	if (renderComp->normalTexture)
	{
		emitter << YAML::Key << "NormalTexture" << YAML::Value << YAML::BeginMap;
		emitter << YAML::Key << "Path" << YAML::Value << renderComp->normalTexture->GetPath();
		emitter << YAML::Key << "FilterType" << YAML::Value << (int)renderComp->normalTexture->GetFilterType();
		emitter << YAML::Key << "WrapType" << YAML::Value << (int)renderComp->normalTexture->GetWrapType();
		emitter << YAML::EndMap;
	}

	if (renderComp->ormTexture)
	{
		emitter << YAML::Key << "ORMTexture" << YAML::Value << YAML::BeginMap;
		emitter << YAML::Key << "Path" << YAML::Value << renderComp->ormTexture->GetPath();
		emitter << YAML::Key << "FilterType" << YAML::Value << (int)renderComp->ormTexture->GetFilterType();
		emitter << YAML::Key << "WrapType" << YAML::Value << (int)renderComp->ormTexture->GetWrapType();
		emitter << YAML::EndMap;
	}

	emitter << YAML::Key << "UVOffset" << YAML::Value << renderComp->uvOffset;
	emitter << YAML::Key << "Roughness" << YAML::Value << renderComp->roughness;
	emitter << YAML::Key << "Metalness" << YAML::Value << renderComp->metalness;
	emitter << YAML::Key << "Wireframe" << YAML::Value << renderComp->isWireframe;
	emitter << YAML::Key << "IgnoreLighting" << YAML::Value << renderComp->isIgnoreLighting;
	emitter << YAML::Key << "CastShadows" << YAML::Value << renderComp->castShadows;
	emitter << YAML::Key << "CastShadowsOn" << YAML::Value << renderComp->castShadowsOn;
	emitter << YAML::Key << "SurfaceShadowSoftness" << YAML::Value << renderComp->surfaceShadowSoftness;
	emitter << YAML::Key << "CastingShadowSoftness" << YAML::Value << renderComp->castingShadownSoftness;
	emitter << YAML::Key << "ReflectRefractType" << YAML::Value << (int)renderComp->reflectRefractData.type;
	emitter << YAML::Key << "ReflectRefractMapType" << YAML::Value << (int)renderComp->reflectRefractData.mapType;
	emitter << YAML::Key << "ReflectRefractStrength" << YAML::Value << renderComp->reflectRefractData.strength;
	emitter << YAML::Key << "ReflectRefractRefractRatio" << YAML::Value << renderComp->reflectRefractData.refractRatio;
	emitter << YAML::Key << "ReflectRefractRefractPriorityType" << YAML::Value << (int)renderComp->reflectRefractMapPriority;
}

void EntityComponentSerializer::SaveRigidComponent(YAML::Emitter& emitter, RigidBodyComponent* rigidComp)
{
	RigidBody* rb = dynamic_cast<RigidBody*>(rigidComp->ptr);
	if (!rb) return;

	emitter << YAML::Key << "ComponentType" << YAML::Value << "Rigid";
	btCollisionShape* shape = rb->GetShape();
	int shapeType = shape->getShapeType();
	if (shapeType == BOX_SHAPE_PROXYTYPE)
	{
		btBoxShape* box = dynamic_cast<btBoxShape*>(shape);
		emitter << YAML::Key << "CollisionType" << YAML::Value << "Box";
		emitter << YAML::Key << "Extents" << YAML::Value << BulletUtils::BulletVec3ToGLM(box->getHalfExtentsWithoutMargin());
	}
	else if (shapeType == SPHERE_SHAPE_PROXYTYPE)
	{
		btSphereShape* sphere = dynamic_cast<btSphereShape*>(shape);
		emitter << YAML::Key << "CollisionType" << YAML::Value << "Sphere";
		emitter << YAML::Key << "Radius" << YAML::Value << sphere->getRadius();
	}
	else if (shapeType == CAPSULE_SHAPE_PROXYTYPE)
	{
		btCapsuleShape* capsule = dynamic_cast<btCapsuleShape*>(shape);
		emitter << YAML::Key << "CollisionType" << YAML::Value << "Capsule";
		emitter << YAML::Key << "Radius" << YAML::Value << capsule->getRadius();
		emitter << YAML::Key << "Height" << YAML::Value << capsule->getHalfHeight();
	}
	else if (shapeType == CYLINDER_SHAPE_PROXYTYPE)
	{
		btCylinderShape* cyl = dynamic_cast<btCylinderShape*>(shape);
		emitter << YAML::Key << "CollisionType" << YAML::Value << "Cylinder";
		emitter << YAML::Key << "Extents" << YAML::Value << cyl->getHalfExtentsWithoutMargin();
	}
	else if (shapeType == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE)
	{
		emitter << YAML::Key << "CollisionType" << YAML::Value << "ScaledTriangleMesh";
		emitter << YAML::Key << "MeshPath" << YAML::Value << entity->GetComponent<RenderComponent>()->mesh->GetPath();
	}
	else if (shapeType == TRIANGLE_MESH_SHAPE_PROXYTYPE)
	{
		emitter << YAML::Key << "CollisionType" << YAML::Value << "TriangleMesh";
		emitter << YAML::Key << "MeshPath" << YAML::Value << entity->GetComponent<RenderComponent>()->mesh->GetPath();
	}

	emitter << YAML::Key << "Mass" << YAML::Value << rb->GetBulletBody()->getMass();
	emitter << YAML::Key << "Inertia" << YAML::Value << BulletUtils::BulletVec3ToGLM(rb->GetBulletBody()->getLocalInertia());

	btTransform transform = rb->GetBulletBody()->getWorldTransform();
	
	emitter << YAML::Key << "InitialRotation" << YAML::Value << BulletUtils::BulletQuatToGLM(transform.getRotation());
	emitter << YAML::Key << "InitialPosition" << YAML::Value << BulletUtils::BulletVec3ToGLM(transform.getOrigin());
}