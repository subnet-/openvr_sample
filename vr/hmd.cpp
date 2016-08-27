#include <GL/glew.h>
#include "hmd.h"

#include <QImage>
#include <QTextStream>

vr::IVRSystem* hmd::_hmd = NULL;

uint32_t hmd::r_customwidth = 0;
uint32_t hmd::r_customheight = 0;

glm::mat4 hmd::projection[2];
glm::mat4 hmd::eye[2];
VRController hmd::controller;

vr::TrackedDevicePose_t hmd::rawPoses[vr::k_unMaxTrackedDeviceCount];

FramebufferDesc hmd::framebuffer[2];

GLint hmd::defaultFBO;
CGLRenderModel* hmd::rendermodels[vr::k_unMaxTrackedDeviceCount];

bool hmd::CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc &framebufferDesc)
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void hmd::exit(std::string description, int exitcode)
{
	fprintf(stderr, "\nExit: %s\n", description.c_str());
	vr::VR_Shutdown();
	std::exit(exitcode);
}

void hmd::init()
{
	vr::EVRInitError eError = vr::VRInitError_None;
	_hmd = vr::VR_Init(&eError, vr::VRApplication_Scene);

	if (eError != vr::VRInitError_None)
		hmd::exit("Could not initialize VRSystem (vr::VR_Init)", 6423); // 0x1917 -> init

	if (!vr::VRCompositor())
		hmd::exit("Could not initialize VRCompositor (vr::VRCompositor)", 192); // 0xc0 -> compositor

	_hmd->GetRecommendedRenderTargetSize(&r_customwidth, &r_customheight);

	/// DEBUG: supersampling test?
	//r_customwidth *= 2;
	//r_customheight *= 2;
	/// END DEBUG

	CreateFrameBuffer(r_customwidth, r_customheight, framebuffer[0]);
	CreateFrameBuffer(r_customwidth, r_customheight, framebuffer[1]);

	projection[0] = ConvertSteamVRMatrixToMatrix4(_hmd->GetProjectionMatrix(vr::Eye_Left, 0.1, 100, vr::API_OpenGL));
	projection[1] = ConvertSteamVRMatrixToMatrix4(_hmd->GetProjectionMatrix(vr::Eye_Right, 0.1, 100, vr::API_OpenGL));
	eye[0] = glm::inverse(ConvertSteamVRMatrixToMatrix4(_hmd->GetEyeToHeadTransform(vr::Eye_Left)));
	eye[1] = glm::inverse(ConvertSteamVRMatrixToMatrix4(_hmd->GetEyeToHeadTransform(vr::Eye_Right)));

	SetupRenderModels();
	setController();
	//distortion::init(_hmd);
}

void hmd::SetupRenderModels()
{
	memset(rendermodels, 0, sizeof(rendermodels));

	if (!_hmd)
		return;

	// 0 for HMD? // 0 for HMD!
	for (uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!_hmd->IsTrackedDeviceConnected(unTrackedDevice))
			continue;

		SetupRenderModelForTrackedDevice(unTrackedDevice);
	}
}

glm::mat4 hmd::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose)
{
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}

glm::mat4 hmd::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix44_t &matPose)
{
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], matPose.m[3][0],
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], matPose.m[3][1],
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], matPose.m[3][2],
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], matPose.m[3][3]
	);
	return matrixObj;
}

FramebufferDesc hmd::getFramebuffer(int eyeIndex)
{
	return (eyeIndex == 0) ? framebuffer[0] : framebuffer[1];
}

glm::mat4 hmd::getProjectionMatrix(int eyeIndex)
{
	return (eyeIndex == 0) ? projection[0] : projection[1];
}

glm::mat4 hmd::getEye(int eyeIndex)
{
	return (eyeIndex == 0) ? hmd::eye[0] : hmd::eye[1];
}

void hmd::drawRenderModels(glm::mat4 projection_matrix, glm::mat4 view_matrix)
{
	GLboolean culling;
	GLint cull_mode;
	glGetBooleanv(GL_CULL_FACE, &culling);
	glGetIntegerv(GL_CULL_FACE_MODE, &cull_mode);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!rendermodels[unTrackedDevice])
			continue;

		glm::mat4 model_matrix;
		// special check for hmd
		if (unTrackedDevice == vr::k_unTrackedDeviceIndex_Hmd)
			model_matrix = glm::inverse(getPose());
		else
			model_matrix = getPose(unTrackedDevice);

		// check if pose is valid
		if (model_matrix == glm::mat4(0))
			continue;



		rendermodels[unTrackedDevice]->render(projection_matrix, view_matrix, model_matrix);
	}

	if (!culling)
		glDisable(GL_CULL_FACE);
	glCullFace(cull_mode);
}

void hmd::ControllerToggleButton(VRControllerButton* button)
{
	// toggle configuration
	if (button->isPressed && !button->unlockToggle && !button->isToggled)
	{
		button->unlockToggle = true;
		button->isToggled = true;
	}
	else if (!button->isPressed && button->unlockToggle)
	{
		button->unlockToggle = false;
	}
	else if (button->isPressed && !button->unlockToggle && button->isToggled)
	{
		button->unlockToggle = true;
		button->isToggled = false;
	}
}

void hmd::GetControllerState()
{
	vr::VRControllerState_t state;
	if (_hmd->GetControllerState(controller.l.i, &state))
	{
		controller.l.menu.isPressed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu));
		controller.l.touch.isPressed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad));
		controller.l.grip.isPressed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_Grip));
		controller.l.trigger.isPressed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger));

		controller.l.touch.isTouched = (state.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad));
		controller.l.trigger.isTouched = (state.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger));

		for (int i = 0; i < vr::k_unControllerStateAxisCount; i++)
			controller.l.axis[i] = state.rAxis[i];

		ControllerToggleButton(&controller.l.menu);
		ControllerToggleButton(&controller.l.touch);
		ControllerToggleButton(&controller.l.grip);
		ControllerToggleButton(&controller.l.trigger);
	}
	if (_hmd->GetControllerState(controller.r.i, &state))
	{
		controller.r.menu.isPressed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu));
		controller.r.touch.isPressed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad));
		controller.r.grip.isPressed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_Grip));
		controller.r.trigger.isPressed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger));

		controller.r.touch.isTouched = (state.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad));
		controller.r.trigger.isTouched = (state.ulButtonTouched & vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger));

		for (int i = 0; i < vr::k_unControllerStateAxisCount; i++)
			controller.r.axis[i] = state.rAxis[i];

		ControllerToggleButton(&controller.r.menu);
		ControllerToggleButton(&controller.r.touch);
		ControllerToggleButton(&controller.r.grip);
		ControllerToggleButton(&controller.r.trigger);
	}

	// any controller configuration
	controller.menu.isPressed = (controller.r.menu.isPressed || controller.l.menu.isPressed);
	controller.touch.isPressed = (controller.r.touch.isPressed || controller.l.touch.isPressed);
	controller.grip.isPressed = (controller.r.grip.isPressed || controller.l.grip.isPressed);
	controller.trigger.isPressed = (controller.r.trigger.isPressed || controller.l.trigger.isPressed);
	for (int i = 0; i < vr::k_unControllerStateAxisCount; i++)
	{
		controller.axis[i].x = (controller.r.axis[i].x + controller.l.axis[i].x);
		controller.axis[i].y = (controller.r.axis[i].y + controller.l.axis[i].y);
	}
	controller.menu.isToggled = (controller.r.menu.isToggled != controller.l.menu.isToggled);
	controller.touch.isToggled = (controller.r.touch.isToggled != controller.l.touch.isToggled);
	controller.grip.isToggled = (controller.r.grip.isToggled != controller.l.grip.isToggled);
	controller.trigger.isToggled = (controller.r.trigger.isToggled != controller.l.trigger.isToggled);
}

glm::mat4 hmd::getViewMatrix(int eyeIndex)
{
	return getEye(eyeIndex) * getPose();
}

void hmd::preRender(int eyeIndex)
{
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);

	glEnable(GL_MULTISAMPLE);
	// bind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, getFramebuffer(eyeIndex).m_nRenderFramebufferId);
	glViewport(0, 0, r_customwidth, r_customheight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void hmd::postRender(int eyeIndex)
{

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_MULTISAMPLE);
	// BlitFramebuffer
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, getFramebuffer(eyeIndex).m_nRenderFramebufferId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, getFramebuffer(eyeIndex).m_nResolveFramebufferId);

	glBlitFramebuffer(0, 0, r_customwidth, r_customheight, 0, 0, r_customwidth, r_customheight,
					  GL_COLOR_BUFFER_BIT,
					  GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// calculate distortion (buggy for now)
	//distortion::render(eyeIndex, r_customwidth, r_customheight);

	// submit to compositor
	vr::Texture_t EyeTexture = { (void*)getFramebuffer(eyeIndex).m_nResolveTextureId, vr::API_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit((eyeIndex == 0) ? vr::Eye_Left : vr::Eye_Right, &EyeTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
}

glm::mat4 hmd::getPose(ViveWand controller)
{
	return getPose(controller.i);
}

glm::mat4 hmd::getPose(int index)
{
	if (index < 0)
	{
		return glm::mat4(0);
	}
	if (rawPoses[index].bPoseIsValid)
	{
		if (index == vr::k_unTrackedDeviceIndex_Hmd)
		{
			return glm::inverse(ConvertSteamVRMatrixToMatrix4(rawPoses[index].mDeviceToAbsoluteTracking));
		}
		else
		{
			return ConvertSteamVRMatrixToMatrix4(rawPoses[index].mDeviceToAbsoluteTracking);
		}
	}
	else
	{
		return glm::mat4(0);
	}
}

void hmd::setController()
{

	for (int unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (_hmd->GetTrackedDeviceClass(unTrackedDevice) != vr::TrackedDeviceClass_Controller)
		{
			if (controller.l.i == unTrackedDevice)
			{
				controller.l.i = -1;
			}
			else if (controller.r.i == unTrackedDevice)
			{
				controller.r.i = -1;
			}
		}
		else
		{
			if (controller.l.i == -1)
			{
				controller.l.i = unTrackedDevice;
			}
			else if (controller.r.i == -1)
			{
				controller.r.i = unTrackedDevice;
			}

			// swap hands if the right hand is supposed to be the left
			if (controller.l.i != -1 && controller.r.i != -1)
			{
				vr::ETrackedControllerRole Role = _hmd->GetControllerRoleForTrackedDeviceIndex(unTrackedDevice);
				if (Role != vr::TrackedControllerRole_Invalid)
				{
					if (Role == vr::TrackedControllerRole_LeftHand)
					{
						controller.r.i = controller.l.i;
						controller.l.i = unTrackedDevice;
					}
					else if (Role == vr::TrackedControllerRole_RightHand)
					{
						controller.l.i = controller.r.i;
						controller.r.i = unTrackedDevice;
					}
				}
			}
		}
	}
}
void hmd::WaitGetPoses()
{
	vr::VRCompositor()->WaitGetPoses(hmd::rawPoses, vr::k_unMaxTrackedDeviceCount, NULL, 0);
}

bool hmd::IsInputFocusCapturedByAnotherProcess()
{
	return _hmd->IsInputFocusCapturedByAnotherProcess();
}

std::string hmd::GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

void hmd::SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex)
{
	if (unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount)
		return;

	// try to find a model we've already set up
	std::string sRenderModelName = GetTrackedDeviceString(_hmd, unTrackedDeviceIndex, vr::Prop_RenderModelName_String);
	CGLRenderModel *pRenderModel = CGLRenderModel::FindOrLoadRenderModel(sRenderModelName.c_str());
	if (!pRenderModel)
	{
		std::string sTrackingSystemName = GetTrackedDeviceString(_hmd, unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String);
		fprintf(stderr, "Unable to load render model for tracked device %d (%s.%s)", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str());
	}
	else
	{
		rendermodels[unTrackedDeviceIndex] = pRenderModel;
	}
}

void hmd::ProcessVREvents()
{
	vr::VREvent_t event;
	while (_hmd->PollNextEvent(&event, sizeof(event)))
	{
		switch (event.eventType)
		{
			case vr::VREvent_TrackedDeviceActivated:
			{
				SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
				if (_hmd->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
				{
					setController();
				}
			}
			break;
			case vr::VREvent_TrackedDeviceDeactivated:
			{
				if (_hmd->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
				{
					setController();
				}
			}
			break;
			case vr::VREvent_TrackedDeviceUpdated:
			{
				if (_hmd->GetTrackedDeviceClass(event.trackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
				{
					/// DEBUG
					fprintf(stderr, "Tracked Device updated: %u\n", event.trackedDeviceIndex);
					setController();
				}
			}
			break;
		}
	}
}