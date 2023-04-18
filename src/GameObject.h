#pragma once

#include "Model.h"

// libs
#include <glm/gtc/matrix_transform.hpp>


#include <memory>
#include <unordered_map>


struct TransformComponent
{
	glm::vec3 translation {};
	glm::vec3 scale {1.f, 1.f, 1.f};
	glm::vec3 rotation {};

	// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
	// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
	// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	glm::mat4 mat4 ();

	glm::mat3 normalMatrix ();
};

class CGameObject
{
public:
	using id_t = unsigned int;
	using Map = std::unordered_map<id_t, CGameObject>;

	static CGameObject createGameObject ()
	{
		static id_t currentId = 0;
		return CGameObject {currentId++};
	}

	id_t getId ()
	{
		return id;
	}

	glm::vec3 color {};
	TransformComponent transform {};

	// Optional pointer components
	std::shared_ptr<CModel> model {};

private:
	CGameObject(id_t
	objId) : id{
		objId
	} {
	}

	id_t id;
};

