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


//Seg編集版 2019-05-02

#pragma comment(lib,"openvr_api")
#include <openvr_driver.h>
#include "driverlog.h"

#include <vector>
#include <thread>
#include <chrono>

#include <windows.h>

#include "E:\OpenVRDriverProj\conlib\ShareMem.h"
SharedMemory comm("pip1"); //通信用


using namespace vr;

#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
#define HMD_DLL_IMPORT extern "C" __declspec( dllimport )

//HMDクォータニオン構造体に格納して返す
inline HmdQuaternion_t HmdQuaternion_Init( double w, double x, double y, double z )
{
	HmdQuaternion_t quat;
	quat.w = w;
	quat.x = x;
	quat.y = y;
	quat.z = z;
	return quat;
}

//HMD行列を単位行列に初期化する。3x3行列だが、スケール成分がついて3x4になっている。
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


// ドライバのresources\settings\からデータを読み込む用のキー
// ※ソースだけでなくbinの中もチェックすること
// driver.vrdrivermanifestのalwaysActivateはtrueにすること

//ControllerDriver
//コントローラの動作。位置情報クラスを継承している。

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
	//コンストラクタ
	CSampleControllerDriver()
	{
		//内部変数を初期化する

		//デバイスID: 無効
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
		//プロパティコンテナ: 無効
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

	//有効化処理。SteamVR側が最初のアプリケーション処理を行う前に行うデバイス初期化処理
	//ここでデバイスIDが割り振られ、位置デバイスの処理が開始される。
	//また、ここからポーズリスナーが有効化される。
	//大きな変数やスレッドを確保し利用開始することができる。
	virtual EVRInitError Activate( vr::TrackedDeviceIndex_t unObjectId )
	{
		DriverLog("Activate\n");

		//デバイスIDを格納する
		m_unObjectId = unObjectId;
		//デバイスIDからプロパティコンテナを取得する。ここにデバイスの詳細情報を格納する
		m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer( m_unObjectId );

		//定義しておいたデバイス詳細情報を格納する
		vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, Prop_ModelNumber_String, m_sModelNumber.c_str() );
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str());

		// return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
		//ここには0(無効)あるいは1(Oculus)以外を入れる
		vr::VRProperties()->SetUint64Property( m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2 );

		// avoid "not fullscreen" warnings from vrmonitor
		//フルスクリーン無効警告を無効化する
		vr::VRProperties()->SetBoolProperty( m_ulPropertyContainer, Prop_IsOnDesktop_Bool, false );

		// our sample device isn't actually tracked, so set this property to avoid having the icon blink in the status window
		//トラッキング外れ警告を有効化
		vr::VRProperties()->SetBoolProperty( m_ulPropertyContainer, Prop_NeverTracked_Bool, true );

		// even though we won't ever track we want to pretend to be the right hand so binding will work as expected
		//右手に割り当ててほしいと定義
		if (leftorright == 0){
			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_RightHand);
		}
		else{
			vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_LeftHand);
		}

		// this file tells the UI what to show the user for binding this controller as well as what default bindings should
		// be for legacy or other apps
		//UI向けにバインディング情報、およびレガシーアプリケーション向けに、デフォルトバインディングを提供する
		vr::VRProperties()->SetStringProperty( m_ulPropertyContainer, Prop_InputProfilePath_String, "{sample}/input/vive_controller_profile.json" );

		// create all the input components
		//入力情報を提供する
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
		//振動出力を提供する
		vr::VRDriverInput()->CreateHapticComponent( m_ulPropertyContainer, "/output/haptic", &m_compHaptic );

		return VRInitError_None;
	}

	//SteamVRによる開放処理
	//他のHMDに切り替わったときなどに行われる。
	//大きな変数やスレッドを開放しなくてはならない。
	//ここからポーズリスナーは使用できなくなる。
	virtual void Deactivate()
	{
		m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
	}

	//スタンバイモード移行処理
	virtual void EnterStandby()
	{
	}

	//ドライバ固有の機能を返す。位置追跡以外に特にない場合はNULLを返す。
	void *GetComponent( const char *pchComponentNameAndVersion )
	{
		// override this to add a component to a driver
		return NULL;
	}

	//電源OFF処理
	virtual void PowerOff()
	{
	}

	/** debug request from a client */
	//デバッグ要求。応答の形式は自由で、ヌル文字で終了する。
	//アプリケーション側から要求できるのかな？
	//ここでは空応答を返している
	virtual void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize )
	{
		if ( unResponseBufferSize >= 1 )
			pchResponseBuffer[0] = 0;
	}

	//初期の姿勢情報
	virtual DriverPose_t GetPose()
	{
		//姿勢を初期化(姿勢変換情報など)
		DriverPose_t pose = { 0 };
		//姿勢が有効か
		pose.poseIsValid = true;
		//姿勢の状態
		pose.result = TrackingResult_Running_OK;
		//デバイス接続情報
		pose.deviceIsConnected = true;

		//予測遅延補正時間なし(通信遅れなど)
		pose.poseTimeOffset = -0.016;

		
		
		//Oculus DK1用で使われていないらしい。
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

		//変換なし
		//ワールド座標系から身体ローカル座標系の回転情報
		pose.vecWorldFromDriverTranslation[0] = 0;
		pose.vecWorldFromDriverTranslation[1] = 0;
		pose.vecWorldFromDriverTranslation[2] = 0;
		pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
		//身体ローカル座標系からデバイス座標系の回転情報
		pose.vecDriverFromHeadTranslation[0] = 0;
		pose.vecDriverFromHeadTranslation[1] = 0;
		pose.vecDriverFromHeadTranslation[2] = 0;
		pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

		return pose;
	}

	//毎フレーム更新処理
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

	//イベント処理
	void ProcessEvent( const vr::VREvent_t & vrEvent )
	{
		switch ( vrEvent.eventType )
		{
			//振動を受け取った時
		case vr::VREvent_Input_HapticVibration:
		{
			//振動ハンドルが現在持っている振動ハンドルと一致するなら
			if (vrEvent.data.hapticVibration.componentHandle == m_compHaptic)
			{
				// This is where you would send a signal to your hardware to trigger actual haptic feedback
				//ブーッ！！
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
	
		//json解析
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
				//姿勢を初期化(姿勢変換情報など)
				DriverPose_t pose = { 0 };
				//姿勢が有効か
				pose.poseIsValid = valid;
				//姿勢の状態
				pose.result = TrackingResult_Running_OK;
				//デバイス接続情報
				pose.deviceIsConnected = true;

				//予測遅延補正時間なし(通信遅れなど)
				pose.poseTimeOffset = -0.016;

				//Oculus DK1用で使われていないらしい。
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

				//変換なし
				//ワールド座標系から身体ローカル座標系の回転情報
				pose.vecWorldFromDriverTranslation[0] = 0;
				pose.vecWorldFromDriverTranslation[1] = 0;
				pose.vecWorldFromDriverTranslation[2] = 0;
				pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
				//身体ローカル座標系からデバイス座標系の回転情報
				pose.vecDriverFromHeadTranslation[0] = 0;
				pose.vecDriverFromHeadTranslation[1] = 0;
				pose.vecDriverFromHeadTranslation[2] = 0;
				pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);


				vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, pose, sizeof(DriverPose_t));
			}
		}
	}

	//シリアルナンバーを返す
	std::string GetSerialNumber() const { return m_sSerialNumber; }
};

//-------------------------------------------------------
//ServerDriver
//ドライバの基礎的な情報を受け渡すためのインターフェース
//これによりドライバが内包しているデバイス情報が通知される。
//ドライバの基礎的な初期化作業や、終了処理、毎フレーム処理をここで行う。

class CServerDriver_Sample: public IServerTrackedDeviceProvider
{
private:
	//コンポーネントハンドル
	//CSampleDeviceDriver *m_pNullHmdLatest = nullptr;
	CSampleControllerDriver *m_pController[10];
	int m_pControllers = 2;

public:
	//初期化処理
	EVRInitError CServerDriver_Sample::Init(vr::IVRDriverContext *pDriverContext)
	{
		//コンテキストの初期化
		VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
		//Log出力処理の初期化
		InitDriverLog(vr::VRDriverLog());
		DriverLog("CServerDriver_Sample::Init\n");

		//コントローラコンポーネントの初期化
		for (int i = 0; i < m_pControllers; i++)
		{
			m_pController[i] = new CSampleControllerDriver();
			m_pController[i]->setleftorright(i);
			//デバイスの追加をSteamVRに通知
			vr::VRServerDriverHost()->TrackedDeviceAdded(m_pController[i]->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_pController[i]);
		}

		return VRInitError_None;
	}

	//終了処理
	void CServerDriver_Sample::Cleanup()
	{
		DriverLog("CServerDriver_Sample::Cleanup\n");

		//Log出力処理のクリーンアップ
		CleanupDriverLog();

		//HMDコンポーネントの開放
		//delete m_pNullHmdLatest;
		//m_pNullHmdLatest = NULL;

		//コントローラコンポーネントの開放
		for (int i = 0; i < m_pControllers; i++)
		{
			delete m_pController[i];
			m_pController[i] = NULL;
		}
	}

	//インターフェースバージョンの通知
	virtual const char * const *GetInterfaceVersions() { 
		//OpenVRライブラリのを返す。(これで良い)
		return vr::k_InterfaceVersions; 
	}

	//フレーム処理の実行
	void CServerDriver_Sample::RunFrame()
	{
		//通信チェック
		char *SharedRam = (char*)comm.get_pointer();
		if (SharedRam[0] != 'x'){
			for (int i = 0; i < m_pControllers; i++)
			{
				if (m_pController[i])
				{
					m_pController[i]->Communication();
				}
			}
			//データ待ちフラグを立てる
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
		//VRイベントを取得(終了その他)
		vr::VREvent_t vrEvent;
		while (vr::VRServerDriverHost()->PollNextEvent(&vrEvent, sizeof(vrEvent)))
		{
			//コントローラへ渡して処理
			for (int i = 0; i < m_pControllers; i++)
			{
				if (m_pController[i])
				{
					m_pController[i]->ProcessEvent(vrEvent);
				}
			}
		}
	}
	//スタンバイモード移行阻害
	virtual bool ShouldBlockStandbyMode()  { 
		return false; 
	}
	//スタンバイモードへ移行
	virtual void EnterStandby()  {}
	//スタンバイモードからの復帰
	virtual void LeaveStandby()  {}
};

//-------------------------------------------------------
//WatchdogDriver
//SteamVRを起動させたり、スタンバイモードに入っているデバイスからSteamVRを起こす時に使う(？)
//仕組み上、別スレッドで動作させる。

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
//サーバー、ウォッチドッグは破棄されないインスタンスが必要

CServerDriver_Sample g_serverDriverNull;
CWatchdogDriver_Sample g_watchdogDriverNull;

//-------------------------------------------------------
//ドライバファクトリー
//SteamVR側に必要な情報を伝えるための一番外側の関数
//ここでドライバの基礎的な情報(なにが実装されているか)が認識される
//...はずだが、現状Logを見る感じ無視されているように見える

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
