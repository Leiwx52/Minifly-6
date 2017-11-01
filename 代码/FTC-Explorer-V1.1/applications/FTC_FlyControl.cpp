/******************** (C) COPYRIGHT 2015 FTC ***************************
 * ����		 ��FTC
 * �ļ���  ��FTC_FlyControl.cpp
 * ����    �����п���
**********************************************************************************/
#include "FTC_FlyControl.h"

FTC_FlyControl fc;

FTC_FlyControl::FTC_FlyControl()
{
	rollPitchRate = 150;
	yawRate = 50;
	
	altHoldDeadband = 100;
	
	//����PID����
	PID_Reset();
}

//����PID����
void FTC_FlyControl::PID_Reset(void)
{
	pid[PIDROLL].set_pid(0.15, 0.15, 0.02, 200);
	pid[PIDPITCH].set_pid(0.15, 0.15, 0.02, 200);
	pid[PIDYAW].set_pid(0.8, 0.45, 0, 200);
	pid[PIDANGLE].set_pid(5, 0, 0, 0);
	pid[PIDMAG].set_pid(2, 0, 0, 0);
 	pid[PIDVELZ].set_pid(1.5, 0.5, 0.002, 150);
 	pid[PIDALT].set_pid(1.2, 0, 0, 200);
}

/*
//��������̬�⻷����
int32_t roll_out, pitch_out, yaw_out;
void FTC_FlyControl::Attitude_Outter_Loop(void)
{
	int32_t e_roll = rc.Command[ROLL] - imu.angle.x;   //���
	int32_t e_pitch = rc.Command[PITCH] - imu.angle.y;
	int32_t e_yaw = rc.Command[YAW] - imu.angle.z;
	
	roll_out = pid[PIDROLL].get_p(e_roll);		//���
	pitch_out = pid[PIDPITCH].get_p(e_pitch);
	yaw_out = pid[PIDYAW].get_p(e_yaw);
}

//��������̬�ڻ�����
void FTC_FlyControl::Attitude_Inner_Loop(void)
{
	Vector3f gyro = mpu6050.Get_Gyro_in_dps(imu.Gyro_lpf);
	
	int16_t e_roll = roll_out - gyro.x;  //���
	int16_t e_pitch = pitch_out - gyro.y;
	int16_t e_yaw = yaw_out - gyro.z;
	
	static uint32_t lastTime;  //ʱ��dt
	float deltaT;
	deltaT = (GetSysTime_us() - lastTime) * 1e-6;
	lastTime = GetSysTime_us();
	
	if(rc.Command[THROTTLE] < RC_MINCHECK) //����
	{
		pid[PIDROLL].reset_I();
		pid[PIDPITCH].reset_I();
		pid[PIDYAW].reset_I();
	}
	
	roll_out = pid[PIDROLL].get_pid(e_roll, deltaT);	//���
	pitch_out = pid[PIDPITCH].get_pid(e_pitch, deltaT);
	yaw_out = pid[PIDYAW].get_pid(e_yaw, deltaT);
	
	motor.writeMotor(rc.Command[THROTTLE], roll_out, pitch_out, yaw_out);
}
*/

//��������̬�⻷����
void FTC_FlyControl::Attitude_Outter_Loop(void)
{
	int32_t	errorAngle[2];
	Vector3f Gyro_ADC;
	
	//����Ƕ����ֵ
	errorAngle[ROLL] = constrain_int32((rc.Command[ROLL] * 2) , -((int)FLYANGLE_MAX), +FLYANGLE_MAX) - imu.angle.x * 10; 
	errorAngle[PITCH] = constrain_int32((rc.Command[PITCH] * 2) , -((int)FLYANGLE_MAX), +FLYANGLE_MAX) - imu.angle.y * 10; 
	errorAngle[ROLL] = applyDeadband(errorAngle[ROLL], 2);
	errorAngle[PITCH] = applyDeadband(errorAngle[PITCH], 2);
	
	//��ȡ���ٶ�
	Gyro_ADC = imu.Gyro_lpf / 4.0f;
	
	//�õ��⻷PID���
	RateError[ROLL] = pid[PIDANGLE].get_p(errorAngle[ROLL]) - Gyro_ADC.x;
	RateError[PITCH] = pid[PIDANGLE].get_p(errorAngle[PITCH]) - Gyro_ADC.y;
	RateError[YAW] = ((int32_t)(yawRate) * rc.Command[YAW]) / 32 - Gyro_ADC.z;		
}

//��������̬�ڻ�����
void FTC_FlyControl::Attitude_Inner_Loop(void)
{
	int32_t PIDTerm[3];
	float tiltAngle = constrain_float( max(abs(imu.angle.x), abs(imu.angle.y)), 0 ,20);
	
	for(u8 i=0; i<3;i++)
	{
		//�����ŵ��ڼ��ֵʱ��������
		if ((rc.rawData[THROTTLE]) < RC_MINCHECK)	
			pid[i].reset_I();
		
		//�õ��ڻ�PID���
		PIDTerm[i] = pid[i].get_pid(RateError[i], PID_INNER_LOOP_TIME*1e-6);
	}
	
	PIDTerm[YAW] = -constrain_int32(PIDTerm[YAW], -300 - abs(rc.Command[YAW]), +300 + abs(rc.Command[YAW]));	
		
	//������б����
	if(!ftc.f.ALTHOLD)
		rc.Command[THROTTLE] = (rc.Command[THROTTLE] - 1000) / cosf(radians(tiltAngle)) + 1000;
	
	//PID���תΪ���������
	motor.writeMotor(rc.Command[THROTTLE], PIDTerm[ROLL], PIDTerm[PITCH], PIDTerm[YAW]);
}


//�������߶��⻷����
void FTC_FlyControl::Altitude_Outter_Loop(void)
{
	//to do
}

//�������߶��ڻ�����
void FTC_FlyControl::Altitude_Inner_Loop(void)
{
	//to do
}

void FTC_FlyControl::AltHoldReset(void)
{
	AltHold = nav.position.z;
}

/************************ (C) COPYRIGHT 2015 FTC *****END OF FILE**********************/
