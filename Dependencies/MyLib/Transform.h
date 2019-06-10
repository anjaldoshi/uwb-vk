#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

/** @class Transform
	
	@brief An object combining position, rotation, and scale.
		Can be used to create a model matrix.
*/
class Transform {
public:
	glm::vec3 scale;		///< Scale of the object
	glm::vec3 position;		///< Position of the object
	glm::quat rotation;		///< Rotation of the object
public:
	/** @brief Transform Constructor 
		Sets scale to 1.0f in all axes,
		Sets position to origin,
		Sets 0 rotation
	*/
	Transform() :
		scale(glm::vec3(1.0f)),
		position(glm::vec3(0.0f)),
		rotation(glm::quat(0.0f, 0.0f, 0.0f, 1.0f)){}

	/** @brief Get a create a model matrix with the transform values */
	glm::mat4 getModelMatrix() const
	{
		glm::mat4 transMat = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotMat = glm::toMat4(rotation);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

		return transMat * rotMat * scaleMat;
	}
};

/** @struct MVPMatrices
	
	@brief Struct of all the matrices sent to vertex shaders
*/
struct MVPMatrices {
	glm::mat4 model;		///< Model matrix
	glm::mat4 view;			///< View matrix
	glm::mat4 projection;	///< Projection matrix from the camera
	glm::mat4 normalMat;	///< Equivalent to transpose(inverse(modelview)). Used for light calculation
};