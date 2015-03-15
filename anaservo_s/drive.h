#ifndef	DRIVE_H
#define	DRIVE_H

#define     MOTOR_CYCLE         3072    /* ���[�^PWM�̃T�C�N�����A      */
                                        /*          �^�C�}�l 1ms        */
#define     MOTOR_OFFSET        37      /* �p���X�����炷����           */
                                        /* 1=325.52[ns]                 */
/* ���݂̏�ԕۑ��p */
extern signed int   speed;          /* ���݂̃X�s�[�h      */
extern int			pid_flag;		/* �o�h�c������s���� */

/* �v���g�^�C�v�錾*/
void speed_r( int accele_l, int accele_r );
void speed2_r( int accele_l, int accele_r );
void speed_f( int accele_l, int accele_r );
void speed2_f( int accele_l, int accele_r );
void motor_mode_r( int mode_l, int mode_r );
void motor_mode_f( int mode_l, int mode_r );
void motor_mode( int mode );
                     
void motor( int accele_l, int accele_m,int accele_r );
void run( int sp, int turn );

#endif