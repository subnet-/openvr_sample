#ifndef CONFIG_H
#define CONFIG_H

#include <glm/vec2.hpp>
#include <string>

/**
 * @brief The global configuration
 *
 * This class contains global configuration parameters
 * that are used all over the program.
 *
 * Hint: Add as many parameters as you need.
 */
class Config
{
public:

	static float animationSpeed;		/**< the number of steps per ms */

	static bool localRotation;			/**< rotate the planets locally */

	static glm::vec2 resolution;		// resolution of the mirror window (gets overwritten by glResize())

	static std::string windowTitle;		// QT Window Title

	static bool submitSkybox;
};

#endif // CONFIG_H

