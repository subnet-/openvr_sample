#ifndef HMD_H
#define HMD_H
#include <openvr.h>
#include <string>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/gtx/transform.hpp>
#include "vr/CGLRenderModel.hpp"
//#include "vr/distortion.hpp" // buggy

/**
* @brief HMD Helper Functions
*
* This class contains helper functions
* that are used all over the program.
*
*/
struct FramebufferDesc
{
	GLuint m_nDepthBufferId;
	GLuint m_nRenderTextureId;
	GLuint m_nRenderFramebufferId;
	GLuint m_nResolveTextureId;
	GLuint m_nResolveFramebufferId;
};

struct VRControllerButton
{
	bool isPressed = false;
	bool isTouched = false;
	bool isToggled = false;
	// check for toggle. Do not edit!
	bool unlockToggle = false;
};

struct ViveWand
{
	int i = -1;
	VRControllerButton menu;											// Menu Button
	VRControllerButton touch;											// Touchpad
	VRControllerButton grip;											// Grip Button
	VRControllerButton trigger;											// Trigger
	vr::VRControllerAxis_t axis[vr::k_unControllerStateAxisCount];	// axis
};

struct VRController
{
	ViveWand l;
	ViveWand r;

	// any controller configuration

	VRControllerButton menu;											// Menu Button
	VRControllerButton touch;											// Touchpad
	VRControllerButton grip;											// Grip Button
	VRControllerButton trigger;											// Trigger
	vr::VRControllerAxis_t axis[vr::k_unControllerStateAxisCount];	// axis
};

class hmd {

	
public:

	static void exit(std::string description, int exitcode);

	static void init();

	static void SetupRenderModels();

	static glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t & matPose);

	static glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix44_t & matPose);

	static FramebufferDesc getFramebuffer(int eyeIndex);

	static glm::mat4 getProjectionMatrix(int eyeIndex);

	static glm::mat4 getEye(int eyeIndex);

	static void preRender(int eyeIndex);
	static glm::mat4 getViewMatrix(int eyeIndex);
	static void postRender(int eyeIndex);

	static vr::TrackedDevicePose_t rawPoses[vr::k_unMaxTrackedDeviceCount];
	
	static glm::mat4 getPose(int index = vr::k_unTrackedDeviceIndex_Hmd);
	static glm::mat4 getPose(ViveWand controller);

	static void setController();

	static void WaitGetPoses();

	static std::string GetTrackedDeviceString(vr::IVRSystem * pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError * peError);

	static void SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);

	static void ProcessVREvents();

	static bool IsInputFocusCapturedByAnotherProcess();

	static void drawRenderModels(glm::mat4 projection_matrix, glm::mat4 view_matrix);

	static void GetControllerState();

	static VRController controller;

private:

	static vr::IVRSystem *_hmd;

	static bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc & framebufferDesc);

	static void ControllerToggleButton(VRControllerButton* button);

	static uint32_t r_customwidth;		// Recommended Render Width
	static uint32_t r_customheight;	// Recommended Render Height

	static glm::mat4 projection[2];
	static glm::mat4 eye[2];

	static FramebufferDesc framebuffer[2];

	static glm::mat4 poses[vr::k_unMaxTrackedDeviceCount];

	static CGLRenderModel *rendermodels[vr::k_unMaxTrackedDeviceCount];

	static GLint defaultFBO;
};

#endif // HMD_H