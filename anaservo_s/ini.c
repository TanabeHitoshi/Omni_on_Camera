/*======================================*/
/* �C���N���[�h                         */
/*======================================*/
#include <stdio.h>
#include "h8_3048.h"                  /* R8C/38A SFR�̒�`�t�@�C��    */
#include <stdio.h>                 	/* printf�g�p���C�u����         */
#include "i2c_eeprom.h"       	   /* EEP-ROM�ǉ�(�f�[�^�L�^)      */
#include "isCamera.h"
#include "drive.h"
#include "sw.h"						/* �X�C�b�`���C�u����			*/
#include "ini.h"

unsigned long   cnt0;                   /* timer�֐��p                  */
unsigned long   cnt1;                   /* main���Ŏg�p                 */
unsigned long   cnt_lcd;                /* LCD�����Ŏg�p                */
unsigned long   stop_timer;				/* ���s�^�C�}�[					*/

int             pattern;                /* �p�^�[���ԍ�                 */
int             iTimer10;               /* 10ms�J�E���g�p               */
int				Cycle;					/* ����							*/

unsigned long	cnt_wdt;				/* WDT�p						*/
int				cnt_wdt2;				/* WDT���������p				*/
/************************************************************************/
/* H8/3048F-ONE �������Ӌ@�\�̏�����                                    */
/************************************************************************/
void init( void )
{
    /* �|�[�g�̓��o�͐ݒ� */
    P1DDR = 0xff;
    P2DDR = 0x00;
    P3DDR = 0xff;
    P4DDR = 0xff;
    P5DDR = 0xff;
    P6DDR = 0x90;                       /* CPU����DIP SW            */
    P8DDR = 0xe3;						/* P80 -> CLK   P81 -> SI		*/
    P9DDR = 0xf7;
    PADDR = 0xd6;
    PBDDR = 0xff;

    /* A/D�̏����ݒ� */
    AD_CSR = 0x0a;                      /* �X�L�������[�h�g�pAN2    CKS -> 134 */
//    AD_CSR |= 0x20;                     /* AD�X�^�[�g                   */

    /* ITU0 ���O���[�^�pPWM */
    ITU0_TCR = 0x23;                    /* �J�E���^�A�N���A�̐ݒ�       */
    ITU0_GRA = MOTOR_CYCLE;             /* PWM����                      */
    ITU0_GRB = 0;                       /* �f���[�e�B��ݒ�             */

    /* ITU1 �E�O���[�^�pPWM */
    ITU1_TCR = 0x23;                    /* �J�E���^�A�N���A�̐ݒ�       */
    ITU1_GRA = MOTOR_CYCLE;             /* PWM����                      */
    ITU1_GRB = 0;                       /* �f���[�e�B��ݒ�             */

    /* ITU2 �G���R�[�_ */
    ITU2_TCR = 0x14;                    /* PA0�p���X���͒[�q            */

    /* ITU3,4 �E��A����A�T�[�{���[�^�pPWM���A���荞�� */
    ITU3_TCR = 0x03;
    ITU4_TCR = 0x03;
    ITU_FCR  = 0x2e;                    /* ITU3,4�ő���PWM���[�h        */
    ITU_TOCR = 0x12;                    /* TIOCB3,TOCXA4,TOCXB4�͔��]   */
    ITU3_IER = 0x01;                    /* TCNT = GRA�ɂ�銄���ݐ���   */
    ITU3_CNT = MOTOR_OFFSET;
    ITU4_CNT = 0;
    ITU3_GRA = MOTOR_CYCLE / 2 + MOTOR_OFFSET - 2;  /* PWM�����ݒ�      */
    ITU3_BRB = ITU3_GRB = MOTOR_CYCLE / 2 - 2;
    ITU4_BRA = ITU4_GRA = MOTOR_CYCLE / 2 - 2;
    ITU4_BRB = ITU4_GRB = MOTOR_CYCLE / 2 - 2;

    ITU_MDR = 0x03;                     /* PWM���[�h�ݒ�                */
    ITU_STR = 0x1f;                     /* ITU�̃J�E���g�X�^�[�g        */
	
	/* WDT�ݒ�	*/
	TCSR_W = 0xa521;					/* WDT=�C���^�[�o���^�C�} */
	cnt_wdt = 0;
	cnt_wdt2 =0;

}

/************************************************************************/
/* ITU3 ���荞�ݏ���                                                    */
/************************************************************************/
#pragma interrupt( interrupt_timer3 )
void interrupt_timer3( void )
{
    unsigned int    i;

    ITU3_TSR &= 0xfe;                   /* �t���O�N���A                 */
    cnt1++;
	cnt0++;

    /* �u�U�[���� */
    beepProcessS();

	/* PID���s */
	Cycle--;
	if( (pid_flag == 1) && (Cycle < 0) ){
		run(speed,pid_turn);
		Cycle = 4;
	}

	/* �f�[�^�ۑ��֘A */
    iTimer10++;
    if( iTimer10 >= 10 ) {
        iTimer10 = 0;
        if( saveFlag ) {
            saveData[0] = pattern;      /* �p�^�[��                     */
            saveData[1] = pid_turn;		/* PID�l   */
            saveData[2] = Center;		/* ���S�l */
            saveData[3] = Wide01;		/* ���C����01 */
            saveData[4] = Wide02;		/* ���C����02 */
            saveData[5] = Max;			/* �ő�l */
            saveData[6] = Min;			/* �ŏ��l */
            saveData[7] = L_maker << 4 | R_maker;	/* �}�[�J�[ */
            /*  8�̓��[�^�֐����ŃX�s�[�h�ۑ�    */
            /*  9�̓��[�^�֐����ŉ�]�l�ۑ�    */
            saveData[10] = 0;    
            saveData[11] = 0;    
            saveData[12] = 0;
            saveData[13] = 0;
            saveData[14] = 0;
            saveData[15] = 0;
            setPageWriteI2CEeprom( saveIndex, 16, saveData );
            saveIndex += 16;
            if( saveIndex >= 0x8000 ) saveFlag = 0;
        }
    }
}
/************************************************************************/
/* WDT ���荞�ݏ���                                                    */
/************************************************************************/
#pragma interrupt( interrupt_wdt )
void interrupt_wdt( void )
{
	unsigned char work;

	work = TCSR & 0x7f;			/* �t���O���[�h */
	TCSR_W = 0xa500 | work;		/* �t���O�N���A */

	cnt_wdt++;
}
/************************************************************************/
/* �^�C�}�{��                                                           */
/* �����@ �^�C�}�l 1=1ms                                                */
/************************************************************************/
void timer( unsigned long timer_set )
{
    cnt0 = 0;
    while( cnt0 < timer_set );
}
/************************************************************************/
/* end of file                                                          */
/************************************************************************/