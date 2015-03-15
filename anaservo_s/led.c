/*======================================*/
/* �C���N���[�h                         */
/*======================================*/
#include <stdio.h>
#include "h8_3048.h"                  /* R8C/38A SFR�̒�`�t�@�C��    */
#include <stdio.h>                 /* printf�g�p���C�u����         */
#include "i2c_eeprom.h"              /* EEP-ROM�ǉ�(�f�[�^�L�^)      */
#include "isCamera.h"
#include "drive.h"
#include "led.h"						/* �X�C�b�`���C�u����			*/


/************************************************************************/
/* ���[�^�h���C�u���TypeS��LED����                                     */
/* �����@4��LED���� 0:OFF 1:ON                                        */
/************************************************************************/
void led_out( unsigned char led )
{
    unsigned char data;

    led <<= 4;
    data = P1DR & 0x0f;
    P1DR = data | led;
}

/************************************************************************/
/* �J�����p�k�d�c���C�g����                                             */
/* �����@ 0�`30 �������u-�v�͋ɐ��Ŋ֌W�Ȃ�    					        */
/************************************************************************/
void LED_Right( int pwm )
{
    unsigned int    work;

    saveData[5] = pwm;                  /* ���O�ۑ�                     */

    if( pwm > 0 ) {
        PADR |= 0x80;
		if( pwm > 30 ) pwm = 30;
    } else if( pwm < 0 ) {
        PADR |= 0x80;
        pwm = -pwm;
		if( pwm > 30 ) pwm = 30;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET - 2;
    work   = (long)work * pwm / 100;
    ITU4_BRB = MOTOR_CYCLE / 2 - 2 - work;
}

/************************************************************************/
/* �J�����p�k�d�c���C�g����i�t���[�j                             */
/* �����@ FREE or BRAKE                                                 */
/************************************************************************/
void right_mode( int mode )
{
    if( mode ) {
        P3DR |= 0x80;
    } else {
        P3DR &= 0x7f;
    }
}
/************************************************************************/
/* end of file                                                          */
/************************************************************************/