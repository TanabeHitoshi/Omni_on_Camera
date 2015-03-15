/*======================================*/
/* �C���N���[�h                         */
/*======================================*/
#include <stdio.h>
#include "h8_3048.h"                  /* R8C/38A SFR�̒�`�t�@�C��    */
#include <stdio.h>                 /* printf�g�p���C�u����         */
#include    "i2c_eeprom.h"              /* EEP-ROM�ǉ�(�f�[�^�L�^)      */
//#include "microsd_lib.h"                /* microSD���䃉�C�u����        */
//#include "lcd_lib.h"                    /* LCD�\���p�ǉ�                */
//#include "switch_lib.h"                 /* �X�C�b�`�ǉ�                 */
//#include "data_flash_lib.h"             /* �f�[�^�t���b�V�����C�u����   */
#include "isCamera.h"
#include "drive.h"
#include "sw.h"


/* ���݂̏�ԕۑ��p */

/************************************************************************/
/* ���[�^�h���C�u���TypeS��̃f�B�b�v�X�C�b�`�l�ǂݍ���                */
/* �߂�l �X�C�b�`�l 0�`15                                              */
/************************************************************************/
unsigned char dipsw_get2( void )
{
    unsigned char sw;

    sw  = ~P2DR;                        /* �h���C�u���TypeS��SW�ǂݍ���*/

    return  sw;
}

/************************************************************************/
/* ���[�^�h���C�u���TypeS��̃v�b�V���X�C�b�`�l�ǂݍ���                */
/* �߂�l �X�C�b�`�l 0:OFF 1:ON                                         */
/************************************************************************/
unsigned char pushsw_get( void )
{
    unsigned char sw;

    sw = ~P8DR & 0x10;                  /* �v�b�V���X�C�b�`�ǂݍ���     */
    sw = !!sw;

    return  sw;
}

/************************************************************************/
/* ���[�^�h���C�u���TypeS��CN8�̏�ԓǂݍ���                           */
/* �߂�l 0�`15                                                         */
/************************************************************************/
unsigned char cn8_get( void )
{
    unsigned char data;

    data = P7DR >> 4;

    return  data;
}

