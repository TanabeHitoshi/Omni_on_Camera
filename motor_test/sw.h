#ifndef	SW_H
#define	SW_H


/* ���݂̏�ԕۑ��p */
extern signed int	handleBuff;             /* ���݂̃n���h���p�x�L�^       */
extern signed int   leftMotorBuff;          /* ���݂̍����[�^PWM�l�L�^      */
extern signed int   rightMotorBuff;         /* ���݂̉E���[�^PWM�l�L�^      */
extern int			angle_buff;             /* ���݃n���h���p�x�ێ��p   */
extern int          servo_center;           /* �T�[�{�Z���^�l               */
/* �G���R�[�_�֘A */
extern int				iTimer10;			   	/* 10ms�J�E���g�p		  	*/
extern int				iEncoder;			   	/* 10ms���̍ŐV�l		   	*/
extern unsigned long	lEncoderTotal;		  	/* �ώZ�l�ۑ��p				*/
extern unsigned int		uEncoderBuff;		   	/* �v�Z�p�@���荞�ݓ��Ŏg�p */
extern unsigned long	lEncoderCrank;		  	/* �N���X���C�����o���̐ώZ�l 	*/
extern unsigned long 	lEncoderCrank2;			/* �N���X���C�����o���̐ώZ�l2 	*/
extern unsigned long	lEncoderHarf;		  	/* �n�[�t���C�����o���̐ώZ�l 	*/

/* �v���g�^�C�v�錾*/
void speed_r( int accele_l, int accele_r );
void speed2_r( int accele_l, int accele_r );
void speed_f( int accele_l, int accele_r );
void speed2_f( int accele_l, int accele_r );
void motor_mode_r( int mode_l, int mode_r );
void motor_mode_f( int mode_l, int mode_r );
                     
void motor( int accele_l, int accele_m,int accele_r );
void handle( int angle );

#endif