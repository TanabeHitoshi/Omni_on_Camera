/*======================================*/
/* �C���N���[�h                         */
/*======================================*/
#include <stdio.h>
#include "h8_3048.h"                  /* R8C/38A SFR�̒�`�t�@�C��    */
#include <stdio.h>                 /* printf�g�p���C�u����         */
#include "i2c_eeprom.h"              /* EEP-ROM�ǉ�(�f�[�^�L�^)      */
#include "isCamera.h"
#include "drive.h"
#include "sw.h"						/* �X�C�b�`���C�u����			*/


/* ���݂̏�ԕۑ��p */
int         speed;          /* ���݂̃X�s�[�h      */
int			pid_flag;		/* �o�h�c������s���� */

/************************************************************************/
/* ��ւ̑��x����                                                       */
/* �����@ �����[�^:-100�`100 , �E���[�^:-100�`100                       */
/*        0�Œ�~�A100�Ő��]100%�A-100�ŋt�]100%                        */
/************************************************************************/
void speed_r( int accele_l, int accele_r )
{
    unsigned int    sw_data;
    unsigned int    work;

    sw_data  = dipsw_get() + 5;         /* �f�B�b�v�X�C�b�`�ǂݍ���     */
    sw_data *= 5;                       /* 5�`20 �� 25�`100�ɕϊ�       */

    /* �����[�^ */
    if( accele_l > 0 ) {
        PBDR &= 0xbf;
    } else if( accele_l < 0 ) {
        PBDR |= 0x40;
        accele_l = -accele_l;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET - 2;
    work   = (long)work * accele_l * sw_data / 10000;
    ITU4_BRA = MOTOR_CYCLE / 2 - 2 - work;

    /* �E���[�^ */
    if( accele_r > 0 ) {
        PBDR &= 0x7f;
    } else if( accele_r < 0 ) {
        PBDR |= 0x80;
        accele_r = -accele_r;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET - 2;
    work   = (long)work * accele_r * sw_data / 10000;
    ITU3_BRB = MOTOR_CYCLE / 2 - 2 - work;
}

/************************************************************************/
/* ��ւ̑��x����2 �f�B�b�v�X�C�b�`�ɂ͊֌W���Ȃ�speed�֐�              */
/* �����@ �����[�^:-100�`100 , �E���[�^:-100�`100                       */
/*        0�Œ�~�A100�Ő��]100%�A-100�ŋt�]100%                        */
/************************************************************************/
void speed2_r( int accele_l, int accele_r )
{
    unsigned int    sw_data;
    unsigned int    work;

    /* �����[�^ */
    if( accele_l > 0 ) {
        PBDR &= 0xbf;
    } else if( accele_l < 0 ) {
        PBDR |= 0x40;
        accele_l = -accele_l;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET - 2;
    work   = (long)work * accele_l / 100;
    ITU4_BRA = MOTOR_CYCLE / 2 - 2 - work;

    /* �E���[�^ */
    if( accele_r > 0 ) {
        PBDR &= 0x7f;
    } else if( accele_r < 0 ) {
        PBDR |= 0x80;
        accele_r = -accele_r;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET - 2;
    work   = (long)work * accele_r / 100;
    ITU3_BRB = MOTOR_CYCLE / 2 - 2 - work;
}

/************************************************************************/
/* �O�ւ̑��x����                                                       */
/* �����@ �����[�^:-100�`100 , �E���[�^:-100�`100                       */
/*        0�Œ�~�A100�Ő��]100%�A-100�ŋt�]100%                        */
/************************************************************************/
void speed_f( int accele_l, int accele_r )
{
    unsigned char   sw_data;
    unsigned long   speed_max;

    sw_data  = dipsw_get() + 5;         /* �f�B�b�v�X�C�b�`�ǂݍ���     */
    speed_max = (unsigned long)(MOTOR_CYCLE-1) * sw_data / 20;

    /* ���O���[�^ */
    if( accele_l > 0 ) {
        PADR &= 0xfd;
    } else if( accele_l < 0 ) {
        PADR |= 0x02;
        accele_l = -accele_l;
    }
    /* GRB��CNT���20�ȏ㏬�����l���ǂ����̃`�F�b�N */
    if( ITU0_GRB > 20 ) {
        /* GRB��20�ȏ�Ȃ�@�P���ɔ�r */
        while( (ITU0_CNT >= ITU0_GRB-20) && (ITU0_CNT <= ITU0_GRB) );
    } else {
        /* GRB��20�ȉ��Ȃ�@����l����̒l���Q�Ƃ��� */
        while( (ITU0_CNT >= ITU0_GRA-20) || (ITU0_CNT <= ITU0_GRB) );
    }
    ITU0_GRB = speed_max * accele_l / 100;

    /* �E�O���[�^ */
    if( accele_r > 0 ) {
        PADR &= 0xbf;
    } else if( accele_r < 0 ) {
        PADR |= 0x40;
        accele_r = -accele_r;
    }
    /* GRB��CNT���20�ȏ㏬�����l���ǂ����̃`�F�b�N */
    if( ITU1_GRB > 20 ) {
        /* GRB��20�ȏ�Ȃ�@�P���ɔ�r */
        while( (ITU1_CNT >= ITU1_GRB-20) && (ITU1_CNT <= ITU1_GRB) );
    } else {
        /* GRB��20�ȉ��Ȃ�@����l����̒l���Q�Ƃ��� */
        while( (ITU1_CNT >= ITU1_GRA-20) || (ITU1_CNT <= ITU1_GRB) );
    }
    ITU1_GRB = speed_max * accele_r / 100;
}

/************************************************************************/
/* �O�ւ̑��x����2 �f�B�b�v�X�C�b�`�ɂ͊֌W���Ȃ�speed�֐�              */
/* �����@ �����[�^:-100�`100 , �E���[�^:-100�`100                       */
/*        0�Œ�~�A100�Ő��]100%�A-100�ŋt�]100%                        */
/************************************************************************/
void speed2_f( int accele_l, int accele_r )
{
    unsigned long   speed_max;


    speed_max = MOTOR_CYCLE - 1;

    /* ���O���[�^ */
    if( accele_l > 0 ) {
        PADR &= 0xfd;
    } else if( accele_l < 0 ) {
        PADR |= 0x02;
        accele_l = -accele_l;
    }
    /* GRB��CNT���20�ȏ㏬�����l���ǂ����̃`�F�b�N */
    if( ITU0_GRB > 20 ) {
        /* GRB��20�ȏ�Ȃ�@�P���ɔ�r */
        while( (ITU0_CNT >= ITU0_GRB-20) && (ITU0_CNT <= ITU0_GRB) );
    } else {
        /* GRB��20�ȉ��Ȃ�@����l����̒l���Q�Ƃ��� */
        while( (ITU0_CNT >= ITU0_GRA-20) || (ITU0_CNT <= ITU0_GRB) );
    }
    ITU0_GRB = speed_max * accele_l / 100;

    /* �E�O���[�^ */
    if( accele_r > 0 ) {
        PADR &= 0xbf;
    } else if( accele_r < 0 ) {
        PADR |= 0x40;
        accele_r = -accele_r;
    }
    /* GRB��CNT���20�ȏ㏬�����l���ǂ����̃`�F�b�N */
    if( ITU1_GRB > 20 ) {
        /* GRB��20�ȏ�Ȃ�@�P���ɔ�r */
        while( (ITU1_CNT >= ITU1_GRB-20) && (ITU1_CNT <= ITU1_GRB) );
    } else {
        /* GRB��20�ȉ��Ȃ�@����l����̒l���Q�Ƃ��� */
        while( (ITU1_CNT >= ITU1_GRA-20) || (ITU1_CNT <= ITU1_GRB) );
    }
    ITU1_GRB = speed_max * accele_r / 100;
}

/************************************************************************/
/* �ヂ�[�^��~����i�u���[�L�A�t���[�j                                 */
/* �����@ �����[�^:FREE or BRAKE , �E���[�^:FREE or BRAKE               */
/************************************************************************/
void motor_mode_r( int mode_l, int mode_r )
{
    if( mode_l ) {
        P1DR |= 0x01;
    } else {
        P1DR &= 0xfe;
    }
    if( mode_r ) {
        P1DR |= 0x02;
    } else {
        P1DR &= 0xfd;
    }
}

/************************************************************************/
/* �O���[�^��~����i�u���[�L�A�t���[�j                                 */
/* �����@ �����[�^:FREE or BRAKE , �E���[�^:FREE or BRAKE               */
/************************************************************************/
void motor_mode_f( int mode_l, int mode_r )
{
    if( mode_l ) {
        P1DR |= 0x04;
    } else {
        P1DR &= 0xfb;
    }
    if( mode_r ) {
        P1DR |= 0x08;
    } else {
        P1DR &= 0xf7;
    }
}
/************************************************************************/
/* ���[�^��~����i�u���[�L�A�t���[�j	                                */
/* �����@ FREE or BRAKE 									            */
/************************************************************************/
void motor_mode( int mode )
{
    motor_mode_f( mode, mode );
    motor_mode_r( mode, mode );
}

/************************************************************************/
/* ���[�^���x����                                                       */
/* �����@ �����[�^:-100�`100�A�ヂ�[�^:-100�`100�A�E���[�^:-100�`100    */
/*        0�Œ�~�A100�Ő��]100%�A-100�ŋt�]100%                        */
/* �߂�l �Ȃ�                                                          */
/************************************************************************/
void motor( int accele_l, int accele_m,int accele_r )
{
	speed_r(accele_r,accele_m);
	speed_f(0,accele_l);
}
/************************************************************************/
/* ���s				                                                    */
/* �����@ ���s:-100�`100�A0�Œ�~�A100�Ő��]100%�A-100�ŋt�]100% 		*/
/*        ��]:-100�`100�A�u+�v�Ŏ��v�����A�u�|�v�Ŕ����v����           */
/* �߂�l �Ȃ�                                                          */
/************************************************************************/
void run( int sp, int turn )
{
	int	sp_l,sp_m,sp_r;

    saveData[8] = sp;             /* ���O�ۑ�                     */
    saveData[9] = turn;             /* ���O�ۑ�                     */

	/* �����[�^�[ */
	sp_l = sp + turn;
	if(sp_l > 100) sp_l = 100;
	if(sp_l < -100) sp_l = -100;
	/* �ヂ�[�^�[ */
	sp_m = -turn;
	if(sp_m > 100) sp_m = 100;
	if(sp_m < -100) sp_m = -100;
	/* �E���[�^�[ */
	sp_r = sp - turn; 
	if(sp_r > 100) sp_r = 100;
	if(sp_r < -100) sp_r = -100;
  

	motor(sp_l,sp_m,sp_r);
}
/************************************************************************/
/* end of file                                                          */
/************************************************************************/