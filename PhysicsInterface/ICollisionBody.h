#pragma once

namespace Physics
{
	enum class CollisionBodyType
	{
		Rigid,
		Soft
	};

	class ICollisionBody
	{
	public:
		virtual ~ICollisionBody() = default;

		CollisionBodyType GetBodyType() const { return type; }

	protected:
		ICollisionBody(CollisionBodyType type) : type(type) {}

	private:
		CollisionBodyType type;

		ICollisionBody(const ICollisionBody&) { }
		ICollisionBody& operator=(const ICollisionBody&) { return *this; }
	};
}