#include "EntityComponentSerializer.h"
#include "RigidBody.h"
#include "BulletUtils.h"
#include "MeshManager.h"
#include "IMesh.h"
#include "TextureManager.h"
#include "Texture2D.h"
#include "BulletUtils.h"
#include "PhysicsFactory.h"

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

void EntityComponentSerializer::Deserialize(const YAML::Node& node)
{
	if (!node["ComponentType"]) return;

	std::string componentType = node["ComponentType"].as<std::string>();
	if (componentType == "Position")
	{
		entity->AddComponent<PositionComponent>(node["Position"].as<glm::vec3>());
	}
	else if (componentType == "Rotation")
	{
		entity->AddComponent<RotationComponent>(node["Rotation"].as<glm::quat>());
	}
	else if (componentType == "Scale")
	{
		entity->AddComponent<ScaleComponent>(node["Scale"].as<glm::vec3>());
	}
	else if (componentType == "Render")
	{
		bool isAnimated = node["Animated"].as<bool>();
		IMesh* loadedMesh = nullptr;
		if (isAnimated)
		{
			loadedMesh = MeshManager::GetAnimatedMesh(node["Mesh"].as<std::string>());
		}
		else
		{
			loadedMesh = MeshManager::GetMesh(node["Mesh"].as<std::string>());
		}

		RenderComponent renderInfo;
		renderInfo.mesh = loadedMesh;
		renderInfo.isColorOverride = node["IsColorOverride"].as<bool>();
		renderInfo.colorOverride = node["ColorOverride"].as<glm::vec3>();

		// Load albedo textures
		const YAML::Node& albedoTextures = node["AlbedoTextures"];
		if (albedoTextures)
		{
			YAML::const_iterator it;
			for (it = albedoTextures.begin(); it != albedoTextures.end(); it++)
			{
				YAML::Node childNode = (*it);
				std::string albedoPath = childNode["Path"].as<std::string>();
				TextureFilterType filterType = (TextureFilterType) childNode["FilterType"].as<int>();
				TextureWrapType wrapType = (TextureWrapType)childNode["WrapType"].as<int>();
				float strength = childNode["Strength"].as<float>();
				Texture2D* texture = TextureManager::CreateTexture2D(albedoPath, filterType, wrapType);
				renderInfo.albedoTextures.push_back({ texture, strength });
			}
		}

		// Normal texture
		const YAML::Node& normalTexture = node["NormalTexture"];
		if (normalTexture)
		{
			std::string normalPath = normalTexture["Path"].as<std::string>();
			TextureFilterType filterType = (TextureFilterType) normalTexture["FilterType"].as<int>();
			TextureWrapType wrapType = (TextureWrapType) normalTexture["WrapType"].as<int>();
			Texture2D* texture = TextureManager::CreateTexture2D(normalPath, filterType, wrapType);
			renderInfo.normalTexture = texture;
		}

		// ORM texture
		const YAML::Node& ormTexture = node["ORMTexture"];
		if (ormTexture)
		{
			std::string ormPath = ormTexture["Path"].as<std::string>();
			TextureFilterType filterType = (TextureFilterType)ormTexture["FilterType"].as<int>();
			TextureWrapType wrapType = (TextureWrapType)ormTexture["WrapType"].as<int>();
			Texture2D* texture = TextureManager::CreateTexture2D(ormPath, filterType, wrapType);
			renderInfo.ormTexture = texture;
		}

		renderInfo.uvOffset = node["UVOffset"].as<glm::vec2>();
		renderInfo.roughness = node["Roughness"].as<float>();
		renderInfo.metalness = node["Metalness"].as<float>();
		renderInfo.isWireframe = node["Wireframe"].as<bool>();
		renderInfo.isIgnoreLighting = node["IgnoreLighting"].as<bool>();
		renderInfo.castShadows = node["CastShadows"].as<bool>();
		renderInfo.castShadowsOn = node["CastShadowsOn"].as<bool>();
		renderInfo.surfaceShadowSoftness = node["SurfaceShadowSoftness"].as<float>();
		renderInfo.castingShadownSoftness = node["CastingShadowSoftness"].as<float>();

		ReflectRefractData rrData;
		rrData.type = (ReflectRefractType) node["ReflectRefractType"].as<int>();
		rrData.mapType = (ReflectRefractMapType)node["ReflectRefractMapType"].as<int>();
		rrData.strength = node["ReflectRefractStrength"].as<float>();
		rrData.refractRatio = node["RefractRatio"].as<float>();
		renderInfo.reflectRefractData = rrData;
		renderInfo.reflectRefractMapPriority = (ReflectRefractMapPriorityType) node["ReflectRefractPriorityType"].as<int>();

		if (node["FaceCullType"])
		{
			renderInfo.faceCullType = (FaceCullType)node["FaceCullType"].as<int>();
		}
		else
		{
			renderInfo.faceCullType = FaceCullType::Back;
		}

		entity->AddComponent<RenderComponent>(renderInfo);
	}
	else if (componentType == "Rigid")
	{
		std::string colliderType = node["CollisionType"].as<std::string>();
		btCollisionShape* shape = nullptr;
		if (colliderType == "Box")
		{
			shape = new btBoxShape(BulletUtils::GLMVec3ToBullet(node["Extents"].as<glm::vec3>()));
		}
		else if (colliderType == "Sphere")
		{
			shape = new btSphereShape(node["Radius"].as<float>());
		}
		else if (colliderType == "Capsule")
		{
			shape = new btCapsuleShape(node["Radius"].as<float>(), node["Height"].as<float>());
		}
		else if (colliderType == "Cylinder")
		{
			shape = new btCylinderShape(BulletUtils::GLMVec3ToBullet(node["Extents"].as<glm::vec3>()));
		}
		else if (colliderType == "ScaledTriangleMesh")
		{
			shape = PhysicsFactory::GetMeshScaledShape(MeshManager::GetMesh(node["MeshPath"].as<std::string>()), node["Scale"].as<glm::vec3>());
		}
		else if (colliderType == "TriangleMesh")
		{
			shape = PhysicsFactory::GetMeshShape(MeshManager::GetMesh(node["MeshPath"].as<std::string>()), MeshColliderType::BvhTriangle);
		}

		Physics::RigidBodyInfo info;
		info.mass = node["Mass"].as<float>();
		info.intertia = node["Inertia"].as<glm::vec3>();
		glm::mat4 transform = glm::toMat4(node["InitialRotation"].as<glm::quat>());
		transform[3] = glm::vec4(node["InitialPosition"].as<glm::vec3>(), 1.0f);
		info.initialTransform = transform;
		RigidBody* rb = new RigidBody(info, shape);
		rb->GetBulletBody()->setRestitution(node["Restitution"].as<float>());
		if (node["IsStatic"].as<bool>())
		{
			rb->GetBulletBody()->setCollisionFlags(btCollisionObject::CollisionFlags::CF_STATIC_OBJECT);
		}

		entity->AddComponent<RigidBodyComponent>(rb);
	}
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
		emitter << YAML::Key << "Strength" << YAML::Value << p.second;
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
	emitter << YAML::Key << "RefractRatio" << YAML::Value << renderComp->reflectRefractData.refractRatio;
	emitter << YAML::Key << "ReflectRefractPriorityType" << YAML::Value << (int)renderComp->reflectRefractMapPriority;
	emitter << YAML::Key << "FaceCullType" << YAML::Value << (int)renderComp->faceCullType;
}

void EntityComponentSerializer::SaveRigidComponent(YAML::Emitter& emitter, RigidBodyComponent* rigidComp)
{
	RigidBody* rb = static_cast<RigidBody*>(rigidComp->ptr);
	if (!rb) return;

	emitter << YAML::Key << "ComponentType" << YAML::Value << "Rigid";
	btCollisionShape* shape = rb->GetShape();
	int shapeType = shape->getShapeType();
	if (shapeType == BOX_SHAPE_PROXYTYPE)
	{
		btBoxShape* box = static_cast<btBoxShape*>(shape);
		emitter << YAML::Key << "CollisionType" << YAML::Value << "Box";
		emitter << YAML::Key << "Extents" << YAML::Value << BulletUtils::BulletVec3ToGLM(box->getHalfExtentsWithoutMargin());
	}
	else if (shapeType == SPHERE_SHAPE_PROXYTYPE)
	{
		btSphereShape* sphere = static_cast<btSphereShape*>(shape);
		emitter << YAML::Key << "CollisionType" << YAML::Value << "Sphere";
		emitter << YAML::Key << "Radius" << YAML::Value << sphere->getRadius();
	}
	else if (shapeType == CAPSULE_SHAPE_PROXYTYPE)
	{
		btCapsuleShape* capsule = static_cast<btCapsuleShape*>(shape);
		emitter << YAML::Key << "CollisionType" << YAML::Value << "Capsule";
		emitter << YAML::Key << "Radius" << YAML::Value << capsule->getRadius();
		emitter << YAML::Key << "Height" << YAML::Value << capsule->getHalfHeight();
	}
	else if (shapeType == CYLINDER_SHAPE_PROXYTYPE)
	{
		btCylinderShape* cyl = static_cast<btCylinderShape*>(shape);
		emitter << YAML::Key << "CollisionType" << YAML::Value << "Cylinder";
		emitter << YAML::Key << "Extents" << YAML::Value << cyl->getHalfExtentsWithoutMargin();
	}
	else if (shapeType == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE)
	{
		btScaledBvhTriangleMeshShape* tShape = static_cast<btScaledBvhTriangleMeshShape*>(shape);
		emitter << YAML::Key << "CollisionType" << YAML::Value << "ScaledTriangleMesh";
		emitter << YAML::Key << "MeshPath" << YAML::Value << entity->GetComponent<RenderComponent>()->mesh->GetPath();
		emitter << YAML::Key << "Scale" << YAML::Value << BulletUtils::BulletVec3ToGLM(tShape->getLocalScaling());
	}
	else if (shapeType == TRIANGLE_MESH_SHAPE_PROXYTYPE)
	{
		emitter << YAML::Key << "CollisionType" << YAML::Value << "TriangleMesh";
		emitter << YAML::Key << "MeshPath" << YAML::Value << entity->GetComponent<RenderComponent>()->mesh->GetPath();
	}

	emitter << YAML::Key << "IsStatic" << YAML::Value << rb->IsStatic();
	emitter << YAML::Key << "Mass" << YAML::Value << rb->GetBulletBody()->getMass();
	emitter << YAML::Key << "Inertia" << YAML::Value << BulletUtils::BulletVec3ToGLM(rb->GetBulletBody()->getLocalInertia());
	emitter << YAML::Key << "Restitution" << YAML::Value << rb->GetBulletBody()->getRestitution();

	btTransform transform = rb->GetBulletBody()->getWorldTransform();
	
	emitter << YAML::Key << "InitialRotation" << YAML::Value << BulletUtils::BulletQuatToGLM(transform.getRotation());
	emitter << YAML::Key << "InitialPosition" << YAML::Value << BulletUtils::BulletVec3ToGLM(transform.getOrigin());
}