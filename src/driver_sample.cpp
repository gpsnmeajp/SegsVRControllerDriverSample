//============ Copyright (c) Valve Corporation, All rights reserved. ============
/*
https://github.com/gpsnmeajp/SegsVRControllerDriverSample/blob/master/LICENSE

BSD 3-Clause License

Copyright (c) 2019, gpsnmeajp
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/*
https://github.com/ValveSoftware/openvr/blob/master/LICENSE

Copyright (c) 2015, Valve Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


//Seg�ҏW�� 2019-05-02

#pragma comment(lib,"openvr_api")
#include <openvr_driver.h>
#include "driverlog.h"

#include <vector>
#include <thread>
#include <chrono>

#include <windows.h>

#include "E:\OpenVRDriverProj\conlib\ShareMem.h"
SharedMemory comm("pip1"); //�ʐM�p


using namespace vr;

#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
#define HMD_DLL_IMPORT extern "C" __declspec( dllimport )

//HMD�N�H�[�^�j�I���\���̂Ɋi�[���ĕԂ�
inline HmdQuaternion_t HmdQuaternion_Init( double w, double x, double y, double z )
{
	HmdQuaternion_t quat;
	quat.w = w;
	quat.x = x;
	quat.y = y;
	quat.z = z;
	return quat;
}

//HMD�s���P�ʍs��ɏ���������B3x3�s�񂾂��A�X�P�[������������3x4�ɂȂ��Ă���B
inline void HmdMatrix_SetIdentity( HmdMatrix34_t *pMatrix )
{
	/*
	1 0 0 0 
	0 1 0 0
	0 0 1 0
	*/
	pMatrix->m[0][0] = 1.f;
	pMatrix->m[0][1] = 0.f;
	pMatrix->m[0][2] = 0.f;
	pMatrix->m[0][3] = 0.f;

	pMatrix->m[1][0] = 0.f;
	pMatrix->m[1][1] = 1.f;
	pMatrix->m[1][2] = 0.f;
	pMatrix->m[1][3] = 0.f;

	pMatrix->m[2][0] = 0.f;
	pMatrix->m[2][1] = 0.f;
	pMatrix->m[2][2] = 1.f;
	pMatrix->m[2][3] = 0.f;
}


// �h���C�o��resources\settings\����f�[�^��ǂݍ��ޗp�̃L�[
// ���\�[�X�����łȂ�bin�̒����`�F�b�N���邱��
// driver.vrdrivermanifest��alwaysActivate��true�ɂ��邱��

//ControllerDriver
//�R���g���[���̓���B�ʒu���N���X���p�����Ă���B

class CSampleControllerDriver : public vr::ITrackedDeviceServerDriver
{
private:
	int leftorright;

	vr::TrackedDeviceIndex_t m_unObjectId;
	vr::PropertyContainerHandle_t m_ulPropertyContainer;

	vr::VRInputComponentHandle_t m_comp[9];
	vr::VRInputComponentHandle_t m_compHaptic;

	std::string m_sSerialNumber;
	std::string m_sModelNumber;


public:
	//�R���X�g���N�^
	CSampleControllerDriver()
	{
		//�����ϐ�������������

		//�f�o�C�XID: ����
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
		//�v���p�e�B�R���e�i: ����
		m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;

		m_sSerialNumber = "CTRL_1234";

		m_sModelNumber = "lh_basestation_vive";
	}

	void setleftorright(int _leftorright)
	{
		leftorright = _leftorright;
		m_sSerialNumber[0] += leftorright;
	}

	virtual ~CSampleControllerDriver()
	{
	}

	//�L���������BSteamVR�����ŏ��̃A�v���P�[�V�����������s���O�ɍs���f�o�C�X����������
	//�����Ńf�o�C�XID������U���A�ʒu�f�o�C�X�̏������J�n�����B
	//�܂��A��������|�[�Y���X�i�[���L���������B
	//�傫�ȕϐ���X���b�h���m�ۂ����p�J�n���邱�Ƃ��ł���B
	virtual EVRInitError Activate( vr::TrackedDeviceIndex_t unObjectId )
	{
		DriverLog("Activate\n");

		//�f�o�C�XID���i�[����
		m_unObjectId = unObjectId;
		//�f�o�C�XID����v���p�e�B�R���e�i���擾����B�����Ƀf�o�C�X�̏ڍ׏����i�[����
		m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer( m_unObjectId );

		//��`���Ă������f�o�C�X�ڍ׏����i�[����
		vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, Prop_ModelNumber_String, m_sModelNumber.c_str() );
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str());

		// return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
		//�����ɂ�0(����)���邢��1(Oculus)�ȊO������
		vr::VRProperties()->SetUint64Property( m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2 );

		// avoid "not fullscreen" warnings from vrmonitor
		//�t���X�N���[�������x���𖳌�������
		vr::VRProperties()->SetBoolProperty( m_ulPropertyContainer, Prop_IsOnDesktop_Bool, false );

		// our sample device isn't actually tracked, so set this property to avoid having the icon blink in the status window
		//�g���b�L���O�O��x����L����
		vr::VRProperties()->SetBoolProperty( m_ulPropertyContainer, Prop_NeverTracked_Bool, true );

		// even though we won't ever track we want to pretend to be the right hand so binding will work as expected
		//�E��Ɋ��蓖�ĂĂق����ƒ�`
		if (leftorright == 0){
			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_RightHand);
		}
		else{
			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_LeftHand);
		}

		// this file tells the UI what to show the user for binding this controller as well as what default bindings should
		// be for legacy or other apps
		//UI�����Ƀo�C���f�B���O���A����у��K�V�[�A�v���P�[�V���������ɁA�f�t�H���g�o�C���f�B���O��񋟂���
		vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, Prop_InputProfilePath_String, "{sample}/input/vive_controller_profile.json" );

		// create all the input components
		//���͏���񋟂���
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/system/click", &m_comp[0]);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/grip/click", &m_comp[1]);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/application_menu/click", &m_comp[2]);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trigger/click", &m_comp[3]);
		vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trigger/value", &m_comp[4],vr::EVRScalarType::VRScalarType_Absolute,EVRScalarUnits::VRScalarUnits_NormalizedOneSided);
		vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trackpad/x", &m_comp[5], vr::EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedTwoSided);
		vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trackpad/y", &m_comp[6], vr::EVRScalarType::VRScalarType_Absolute, EVRScalarUnits::VRScalarUnits_NormalizedTwoSided);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trackpad/click", &m_comp[7]);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trackpad/touch", &m_comp[8]);

		// create our haptic component
		//�U���o�͂�񋟂���
		vr::VRDriverInput()->CreateHapticComponent( m_ulPropertyContainer, "/output/haptic", &m_compHaptic );

		return VRInitError_None;
	}

	//SteamVR�ɂ��J������
	//����HMD�ɐ؂�ւ�����Ƃ��Ȃǂɍs����B
	//�傫�ȕϐ���X���b�h���J�����Ȃ��Ă͂Ȃ�Ȃ��B
	//��������|�[�Y���X�i�[�͎g�p�ł��Ȃ��Ȃ�B
	virtual void Deactivate()
	{
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
	}

	//�X�^���o�C���[�h�ڍs����
	virtual void EnterStandby()
	{
	}

	//�h���C�o�ŗL�̋@�\��Ԃ��B�ʒu�ǐՈȊO�ɓ��ɂȂ��ꍇ��NULL��Ԃ��B
	void *GetComponent( const char *pchComponentNameAndVersion )
	{
		// override this to add a component to a driver
		return NULL;
	}

	//�d��OFF����
	virtual void PowerOff()
	{
	}

	/** debug request from a client */
	//�f�o�b�O�v���B�����̌`���͎��R�ŁA�k�������ŏI������B
	//�A�v���P�[�V����������v���ł���̂��ȁH
	//�����ł͋󉞓���Ԃ��Ă���
	virtual void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize )
	{
		if ( unResponseBufferSize >= 1 )
			pchResponseBuffer[0] = 0;
	}

	//�����̎p�����
	virtual DriverPose_t GetPose()
	{
		//�p����������(�p���ϊ����Ȃ�)
		DriverPose_t pose = { 0 };
		//�p�����L����
		pose.poseIsValid = true;
		//�p���̏��
		pose.result = TrackingResult_Running_OK;
		//�f�o�C�X�ڑ����
		pose.deviceIsConnected = true;

		//�\���x���␳���ԂȂ�(�ʐM�x��Ȃ�)
		pose.poseTimeOffset = -0.016;

		
		
		//Oculus DK1�p�Ŏg���Ă��Ȃ��炵���B
		pose.willDriftInYaw = false;
		pose.shouldApplyHeadModel = false;

		if (leftorright == 0){
			double tm = 3.14*2.0*(double)(GetTickCount()) / 1000.0;
			pose.vecPosition[0] = 0;//sin(tm)-0.5;
			pose.vecPosition[1] = 0;
			pose.vecPosition[2] = 0;// cos(tm) - 0.5;
		}
		else{
			double tm = -3.14*1.3*(double)(GetTickCount()) / 1000.0;
			pose.vecPosition[0] = sin(tm) - 0.5;
			pose.vecPosition[1] = 0;
			pose.vecPosition[2] = cos(tm) - 0.5;
		}

		pose.qRotation = HmdQuaternion_Init(1, 0, 0, 0); 
		pose.vecVelocity[0] = (rand() % 1000) / 1000.0;;
		pose.vecVelocity[1] = (rand() % 1000) / 1000.0;;
		pose.vecVelocity[2] = (rand() % 1000) / 1000.0;;

		pose.vecAngularVelocity[0] = (rand() % 1000) / 1000.0;;
		pose.vecAngularVelocity[1] = (rand() % 1000) / 1000.0;;
		pose.vecAngularVelocity[2] = (rand() % 1000) / 1000.0;;

		pose.vecAcceleration[0] = (rand() % 1000) / 1000.0;;
		pose.vecAcceleration[1] = (rand() % 1000) / 1000.0;;
		pose.vecAcceleration[2] = (rand() % 1000) / 1000.0;;

		pose.vecAngularAcceleration[0] = (rand() % 1000) / 1000.0;;
		pose.vecAngularAcceleration[1] = (rand() % 1000) / 1000.0;;
		pose.vecAngularAcceleration[2] = (rand() % 1000) / 1000.0;;

		//�ϊ��Ȃ�
		//���[���h���W�n����g�̃��[�J�����W�n�̉�]���
		pose.vecWorldFromDriverTranslation[0] = 0;
		pose.vecWorldFromDriverTranslation[1] = 0;
		pose.vecWorldFromDriverTranslation[2] = 0;
		pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
		//�g�̃��[�J�����W�n����f�o�C�X���W�n�̉�]���
		pose.vecDriverFromHeadTranslation[0] = 0;
		pose.vecDriverFromHeadTranslation[1] = 0;
		pose.vecDriverFromHeadTranslation[2] = 0;
		pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

		return pose;
	}

	//���t���[���X�V����
	void RunFrame()
	{
		// Your driver would read whatever hardware state is associated with its input components and pass that
		// in to UpdateBooleanComponent. This could happen in RunFrame or on a thread of your own that's reading USB
		// state. There's no need to update input state unless it changes, but it doesn't do any harm to do so.

		/*
		vr::VRDriverInput()->UpdateBooleanComponent(m_comp[0], (0x8000 & GetAsyncKeyState('Q')) != 0, 0);
		vr::VRDriverInput()->UpdateBooleanComponent(m_comp[1], (0x8000 & GetAsyncKeyState('W')) != 0, 0);
		vr::VRDriverInput()->UpdateBooleanComponent(m_comp[2], (0x8000 & GetAsyncKeyState('E')) != 0, 0);
		vr::VRDriverInput()->UpdateBooleanComponent(m_comp[3], (0x8000 & GetAsyncKeyState('R')) != 0, 0);
		vr::VRDriverInput()->UpdateScalarComponent(m_comp[4], (0x8000 & GetAsyncKeyState('T')) != 0, 0);
		vr::VRDriverInput()->UpdateScalarComponent(m_comp[5], (0x8000 & GetAsyncKeyState('Y')) != 0, 0);
		vr::VRDriverInput()->UpdateScalarComponent(m_comp[6], (0x8000 & GetAsyncKeyState('U')) != 0, 0);
		vr::VRDriverInput()->UpdateBooleanComponent(m_comp[7], (0x8000 & GetAsyncKeyState('I')) != 0, 0);
		vr::VRDriverInput()->UpdateBooleanComponent(m_comp[8], (0x8000 & GetAsyncKeyState('O')) != 0, 0);
		*/
		// In a real driver, this should happen from some pose tracking thread.
		// The RunFrame interval is unspecified and can be very irregular if some other
		// driver blocks it for some periodic task.
		/*
		if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid)
		{
			vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(DriverPose_t));
		}
		*/
	}

	//�C�x���g����
	void ProcessEvent( const vr::VREvent_t & vrEvent )
	{
		switch ( vrEvent.eventType )
		{
			//�U�����󂯎������
		case vr::VREvent_Input_HapticVibration:
		{
			//�U���n���h�������ݎ����Ă���U���n���h���ƈ�v����Ȃ�
			if (vrEvent.data.hapticVibration.componentHandle == m_compHaptic)
			{
				// This is where you would send a signal to your hardware to trigger actual haptic feedback
				//�u�[�b�I�I
				DriverLog("BUZZ!\n");
			}
		}
		break;
		}
	}

	void Communication()
	{
		char *SharedRam = (char*)comm.get_pointer();

		if (SharedRam[0] == 'x'){
			return;
		}
	
		//json���
		std::string json = SharedRam;
		picojson::value j;
		std::string err = picojson::parse(j, json);
		if (!err.empty()){
			printf("%s\n", err.c_str());
			return;
		}

		bool valid;
		if (GetBoolValue(valid, j, "Valid") != 0){ return; }

		double id;
		if (GetDoubleValue(id, j, "id") != 0){ return; }

		double v[3], vd[3], vdd[3];
		if (GetDoubleArry(v, 3, j, "v") != 0){ return; }
		if (GetDoubleArry(vd, 3, j, "vd") != 0){ return; }
		if (GetDoubleArry(vdd, 3, j, "vdd") != 0){ return; }
		double r[4], rd[3], rdd[3];
		if (GetDoubleArry(r, 4, j, "r") != 0){ return; }
		if (GetDoubleArry(rd, 3, j, "rd") != 0){ return; }
		if (GetDoubleArry(rdd, 3, j, "rdd") != 0){ return; }

		if ((int)id == leftorright)
		{
			if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid)
			{
				//�p����������(�p���ϊ����Ȃ�)
				DriverPose_t pose = { 0 };
				//�p�����L����
				pose.poseIsValid = valid;
				//�p���̏��
				pose.result = TrackingResult_Running_OK;
				//�f�o�C�X�ڑ����
				pose.deviceIsConnected = true;

				//�\���x���␳���ԂȂ�(�ʐM�x��Ȃ�)
				pose.poseTimeOffset = -0.016;

				//Oculus DK1�p�Ŏg���Ă��Ȃ��炵���B
				pose.willDriftInYaw = false;
				pose.shouldApplyHeadModel = false;

				pose.vecPosition[0] = v[0];
				pose.vecPosition[1] = v[1];
				pose.vecPosition[2] = v[2];

				pose.qRotation = HmdQuaternion_Init(r[0], r[1], r[2], r[3]);
				pose.vecVelocity[0] = vd[0];
				pose.vecVelocity[1] = vd[1];
				pose.vecVelocity[2] = vd[2];

				pose.vecAngularVelocity[0] = rd[0];
				pose.vecAngularVelocity[1] = rd[1];
				pose.vecAngularVelocity[2] = rd[2];

				pose.vecAcceleration[0] = vdd[0];
				pose.vecAcceleration[1] = vdd[1];
				pose.vecAcceleration[2] = vdd[2];

				pose.vecAngularAcceleration[0] = rdd[0];
				pose.vecAngularAcceleration[1] = rdd[1];
				pose.vecAngularAcceleration[2] = rdd[2];

				//�ϊ��Ȃ�
				//���[���h���W�n����g�̃��[�J�����W�n�̉�]���
				pose.vecWorldFromDriverTranslation[0] = 0;
				pose.vecWorldFromDriverTranslation[1] = 0;
				pose.vecWorldFromDriverTranslation[2] = 0;
				pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
				//�g�̃��[�J�����W�n����f�o�C�X���W�n�̉�]���
				pose.vecDriverFromHeadTranslation[0] = 0;
				pose.vecDriverFromHeadTranslation[1] = 0;
				pose.vecDriverFromHeadTranslation[2] = 0;
				pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);


				vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, pose, sizeof(DriverPose_t));
			}
		}
	}

	//�V���A���i���o�[��Ԃ�
	std::string GetSerialNumber() const { return m_sSerialNumber; }
};

//-------------------------------------------------------
//ServerDriver
//�h���C�o�̊�b�I�ȏ����󂯓n�����߂̃C���^�[�t�F�[�X
//����ɂ��h���C�o������Ă���f�o�C�X��񂪒ʒm�����B
//�h���C�o�̊�b�I�ȏ�������Ƃ�A�I�������A���t���[�������������ōs���B

class CServerDriver_Sample: public IServerTrackedDeviceProvider
{
private:
	//�R���|�[�l���g�n���h��
	//CSampleDeviceDriver *m_pNullHmdLatest = nullptr;
	CSampleControllerDriver *m_pController[10];
	int m_pControllers = 2;

public:
	//����������
	EVRInitError CServerDriver_Sample::Init(vr::IVRDriverContext *pDriverContext)
	{
		//�R���e�L�X�g�̏�����
		VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
		//Log�o�͏����̏�����
		InitDriverLog(vr::VRDriverLog());
		DriverLog("CServerDriver_Sample::Init\n");

		//�R���g���[���R���|�[�l���g�̏�����
		for (int i = 0; i < m_pControllers; i++)
		{
			m_pController[i] = new CSampleControllerDriver();
			m_pController[i]->setleftorright(i);
			//�f�o�C�X�̒ǉ���SteamVR�ɒʒm
			vr::VRServerDriverHost()->TrackedDeviceAdded(m_pController[i]->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_pController[i]);
		}

		return VRInitError_None;
	}

	//�I������
	void CServerDriver_Sample::Cleanup()
	{
		DriverLog("CServerDriver_Sample::Cleanup\n");

		//Log�o�͏����̃N���[���A�b�v
		CleanupDriverLog();

		//HMD�R���|�[�l���g�̊J��
		//delete m_pNullHmdLatest;
		//m_pNullHmdLatest = NULL;

		//�R���g���[���R���|�[�l���g�̊J��
		for (int i = 0; i < m_pControllers; i++)
		{
			delete m_pController[i];
			m_pController[i] = NULL;
		}
	}

	//�C���^�[�t�F�[�X�o�[�W�����̒ʒm
	virtual const char * const *GetInterfaceVersions() { 
		//OpenVR���C�u�����̂�Ԃ��B(����ŗǂ�)
		return vr::k_InterfaceVersions; 
	}

	//�t���[�������̎��s
	void CServerDriver_Sample::RunFrame()
	{
		//�ʐM�`�F�b�N
		char *SharedRam = (char*)comm.get_pointer();
		if (SharedRam[0] != 'x'){
			for (int i = 0; i < m_pControllers; i++)
			{
				if (m_pController[i])
				{
					m_pController[i]->Communication();
				}
			}
			//�f�[�^�҂��t���O�𗧂Ă�
			SharedRam[1] = '\0';
			SharedRam[0] = 'x';
		}

		for (int i = 0; i < m_pControllers; i++)
		{

			if (m_pController[i])
			{
				m_pController[i]->RunFrame();
			}
		}
		//VR�C�x���g���擾(�I�����̑�)
		vr::VREvent_t vrEvent;
		while (vr::VRServerDriverHost()->PollNextEvent(&vrEvent, sizeof(vrEvent)))
		{
			//�R���g���[���֓n���ď���
			for (int i = 0; i < m_pControllers; i++)
			{
				if (m_pController[i])
				{
					m_pController[i]->ProcessEvent(vrEvent);
				}
			}
		}
	}
	//�X�^���o�C���[�h�ڍs�j�Q
	virtual bool ShouldBlockStandbyMode()  { 
		return false; 
	}
	//�X�^���o�C���[�h�ֈڍs
	virtual void EnterStandby()  {}
	//�X�^���o�C���[�h����̕��A
	virtual void LeaveStandby()  {}
};

//-------------------------------------------------------
//WatchdogDriver
//SteamVR���N����������A�X�^���o�C���[�h�ɓ����Ă���f�o�C�X����SteamVR���N�������Ɏg��(�H)
//�d�g�ݏ�A�ʃX���b�h�œ��삳����B

bool g_bExiting = false;

void WatchdogThreadFunction()
{
	while (!g_bExiting)
	{
		// on windows send the event when the Y key is pressed.
		if ((0x01 & GetAsyncKeyState('Y')) != 0)
		{
			// Y key was pressed. 
			vr::VRWatchdogHost()->WatchdogWakeUp();
		}
		std::this_thread::sleep_for(std::chrono::microseconds(500));
	}
}

class CWatchdogDriver_Sample : public IVRWatchdogProvider
{

private:
	std::thread *m_pWatchdogThread;

public:
	CWatchdogDriver_Sample()
	{
		m_pWatchdogThread = nullptr;
	}

	EVRInitError CWatchdogDriver_Sample::Init(vr::IVRDriverContext *pDriverContext)
	{
		VR_INIT_WATCHDOG_DRIVER_CONTEXT(pDriverContext);
		InitDriverLog(vr::VRDriverLog());
		DriverLog("CWatchdogDriver_Sample::Init\n");

		// Watchdog mode on Windows starts a thread that listens for the 'Y' key on the keyboard to 
		// be pressed. A real driver should wait for a system button event or something else from the 
		// the hardware that signals that the VR system should start up.
		g_bExiting = false;
		m_pWatchdogThread = new std::thread(WatchdogThreadFunction);
		if (!m_pWatchdogThread)
		{
			DriverLog("Unable to create watchdog thread\n");
			return VRInitError_Driver_Failed;
		}

		if (!comm.is_open())
		{
			return VRInitError_Driver_Failed;
		}

		return VRInitError_None;
	}
	void CWatchdogDriver_Sample::Cleanup()
	{
		DriverLog("CWatchdogDriver_Sample::Cleanup\n");

		g_bExiting = true;
		if (m_pWatchdogThread)
		{
			m_pWatchdogThread->join();
			delete m_pWatchdogThread;
			m_pWatchdogThread = nullptr;
		}

		CleanupDriverLog();
	}
};



//-------------------------------------------------------
//�T�[�o�[�A�E�H�b�`�h�b�O�͔j������Ȃ��C���X�^���X���K�v

CServerDriver_Sample g_serverDriverNull;
CWatchdogDriver_Sample g_watchdogDriverNull;

//-------------------------------------------------------
//�h���C�o�t�@�N�g���[
//SteamVR���ɕK�v�ȏ���`���邽�߂̈�ԊO���̊֐�
//�����Ńh���C�o�̊�b�I�ȏ��(�Ȃɂ���������Ă��邩)���F�������
//...�͂������A����Log�����銴����������Ă���悤�Ɍ�����

HMD_DLL_EXPORT void *HmdDriverFactory( const char *pInterfaceName, int *pReturnCode )
{
	if( 0 == strcmp( IServerTrackedDeviceProvider_Version, pInterfaceName ) )
	{
		return &g_serverDriverNull;
	}
	if( 0 == strcmp( IVRWatchdogProvider_Version, pInterfaceName ) )
	{
		return &g_watchdogDriverNull;
	}

	if( pReturnCode )
		*pReturnCode = VRInitError_Init_InterfaceNotFound;

	return NULL;
}
