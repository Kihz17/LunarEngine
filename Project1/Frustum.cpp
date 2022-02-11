#include "Frustum.h"

namespace FrustumUtils
{
	Frustum CreateFrustumFromCamera(const Camera& camera, const float aspect, const float far, const float near)
	{
		Frustum frustum;

		// Compute edges of the frustum https://gyazo.com/05f6bdb2693d048cf11caaf7e0c68992
		const float halfVerticalSide = far * tanf(camera.fov * 0.5f);
		const float halfHorizontalSide = halfVerticalSide * aspect;
		const glm::vec3 frontMultFar = far * camera.front;

		// Generate all edges of the frustum
		frustum.near = { camera.position + near * camera.front, camera.front };
		frustum.far = { camera.position + frontMultFar, -camera.front };
		frustum.right = { camera.position, glm::cross(camera.up, frontMultFar + camera.right * halfHorizontalSide) };
		frustum.left = { camera.position, glm::cross(frontMultFar - camera.right * halfHorizontalSide, camera.up) };
		frustum.top = { camera.position, glm::cross(camera.right, frontMultFar - camera.up * halfVerticalSide) };
		frustum.bottom = { camera.position, glm::cross(frontMultFar + camera.up * halfVerticalSide, camera.right) };

		return frustum;
	}
}