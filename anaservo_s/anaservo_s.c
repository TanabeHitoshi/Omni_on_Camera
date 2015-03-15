/****************************************************************************/
/* TypeS��g�p�}�C�R���J�[�@�T���v���v���O���� Ver1.11                    */
/* �I���Ƀz�C�[���g�p�J�����}�V��											*/
/*                          2015.02.25 ���{������H�ȍ����w�Z�@�c�Ӑm�j   */
/****************************************************************************/

/*======================================*/
/* �C���N���[�h                         */
/*======================================*/
#include    <no_float.h>                /* stdio�̊ȗ��� �ŏ��ɒu��     */
#include    <stdio.h>                   /* printf�ȂǗp                 */
#include    <machine.h>                 /* �g�ݍ��݊֐��p               */
#include    "h8_3048.h"                 /* H8/3048F-ONE���W�X�^��`     */
#include    "i2c_eeprom.h"              /* EEP-ROM�ǉ�(�f�[�^�L�^)      */
#include    "types_beep.h"              /* TypeS��̃u�U�[            */
#include 	"isCamera.h"				/* �J�����p���C�u����			*/
#include 	"isSensor.h"					/* �}�[�J�[���o�p */
#include	"drive.h"					/* ���[�^�[�h���C�u���C�u����	*/
#include 	"sw.h"						/* �X�C�b�`���C�u����			*/
#include 	"led.h"						/* LED���C�u����				*/
#include	"ini.h"						/* �����ݒ�l					*/
/*======================================*/
/* �V���{����`                         */
/*======================================*/
/* �萔�ݒ� */

#define     FREE                1       /* ���[�^���[�h�@�t���[         */
#define     BRAKE               0       /* ���[�^���[�h�@�u���[�L       */

/*======================================*/
/* �v���g�^�C�v�錾                     */
/*======================================*/
void init( void );
void beep_out( int flag );

/*======================================*/
/* �O���[�o���ϐ��̐錾                 */
/*======================================*/

/************************************************************************/
/* ���C���v���O����                                                     */
/************************************************************************/
void main( void )
{
    int             i, j;
    unsigned int    u;
    char            c;

    /* �}�C�R���@�\�̏����� */
    init();                             /* ������                       */
    initI2CEeprom( &P6DDR, &P6DR, 0x90, 6, 5);  /* EEP-ROM�����ݒ�      */
    initBeepS();                        /* �u�U�[�֘A����               */
    init_sci1( 0x00, 79 );              /* SCI1������                   */
    set_ccr( 0x00 );                    /* �S�̊��荞�݋���             */
	initCamera(10,118);

    /* �}�C�R���J�[�̏�ԏ����� */
    motor_mode( BRAKE );
    right_mode( FREE );
	run(0,0);
    LED_Right( 00 );
    setBeepPatternS( 0x8000 );
	pattern = 0;
	pid_flag = 0;

    /* �X�^�[�g���A�X�C�b�`��������Ă���΃f�[�^�]�����[�h */
    if( pushsw_get() ) {
        pattern = 101;
        cnt1 = 0;
    }

    while( 1 ) {
	sensor_process();
    I2CEepromProcess();                 /* I2C EEP-ROM�ۑ�����          */

    switch( pattern ) {
    case 0:
        /* �v�b�V���X�C�b�`�����҂� */
        LED_Right( 40 );
        if( pushsw_get() ) {
			Calibration();
            setBeepPatternS( 0x8000 );
            clearI2CEeprom();           /* ���b������                   */
            setBeepPatternS( 0xcc00 );
            cnt1 = 0;
			if(dipsw_get2() == 0x80) pattern = 2000;
            else					 pattern = 1;
            break;
        }
        i =  (cnt1/200) % 2 + 1;
        led_out( i );                   /* LED�_�ŏ���                  */
        break;

    case 1:
        /* �X�^�[�g�o�[�J�҂� */
        LED_Right( 20 );
        led_out( 0x0 );
        cnt1 = 0;
        pattern = 11;
		saveIndex = 0x0000;
        saveFlag = 1;               /* �f�[�^�ۑ��J�n               */
		break;
	case 9:
        LED_Right( 20 );
		speed = 20;pid_flag =1;
		if(R_maker == 1){
		    setBeepPatternS( 0x8000 );
			pattern = 11;
			cnt1 = 0;
		}
		break;
	case 10:
        LED_Right( 20 );
		speed = 20;pid_flag =1;
		if(cnt1 > 1000){
			pattern = 11;
		}
		break;
    case 11:
        /* �ʏ�g���[�X */
        LED_Right( 20 );
		speed = 30;pid_flag =1;
		if(R_maker == 1){
		    setBeepPatternS( 0x8000 );
			cnt1 = 0;
//			pattern = 12;
//			beep_out(1);
//		}else if(L_maker == 1){
//		    setBeepPatternS( 0xcc00 );
//			beep_out(1);
		}else{
			beep_out(0);
		}
		break;
	case 12:
        LED_Right( 20 );
		speed = 20;pid_flag =1;
		if(cnt1 > 1000) pattern = 13;
		break;
	case 13:
        LED_Right( 0 );
		pid_flag =0;
		motor(0,0,0);
		break;
    case 101:
        /* ��~ */
        LED_Right( 0 );
		motor(0,0,0);
        setBeepPatternS( 0xc000 );
        saveFlag = 0;
        saveSendIndex = 0;
        pattern = 102;
        cnt1 = 0;
        break;

    case 102:
        /* �v�b�V���X�C�b�`�������ꂽ���`�F�b�N */
        if( !pushsw_get() ) {
            pattern = 103;
            cnt1 = 0;
        }
        break;

   case 103:
        /* 0.5s�҂� */
        if( cnt1 >= 500 ) {
            pattern = 104;
            cnt1 = 0;
        }
        break;

    case 104:
        /* �v�b�V���X�C�b�`�������ꂽ���`�F�b�N */
        led_out( cnt1 / 200 % 2 ? 0x6 : 0x9  );
        if( pushsw_get() ) {
            pattern = 105;
            cnt1 = 0;
        }
        break;

    case 105:
        /* �^�C�g���]���A�]������ */
        printf( "\n" );
        printf( "CarName Data Out\n" ); /* �����̃J�[�l�[�������Ă������� */
        printf( "Pattern, PID�l, ���S�l, ���C����01, ���C����02, Max, Min, �}�[�J�[, " );
        printf( "�X�s�[�h, ��]\n" );
        pattern = 106;
        break;

    case 106:
        /* �f�[�^�]�� */
        led_out( 1 << (cnt1/100) % 4 );

        /* �I���̃`�F�b�N */
/*        if( (readI2CEeprom( saveSendIndex )==0) ||
                                (saveSendIndex >= 0x8000) ) {
            pattern = 107;
            setBeepPatternS( 0xff00 );
            cnt1 = 0;
            break;
        }
*/
        /* �f�[�^�̓]�� */
        printf( "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            /* �p�^�[�� */
            (int)readI2CEeprom( saveSendIndex+0 ),
            /* PID�l */
            (int)readI2CEeprom( saveSendIndex+1 ),
            /* ���S�l */
            (int)readI2CEeprom( saveSendIndex+2 ),
            /* ���C����01 */
            (int)readI2CEeprom( saveSendIndex+3 ),
            /* ���C����02 */
            (int)readI2CEeprom( saveSendIndex+4 ),
            /* Max */
            (int)readI2CEeprom( saveSendIndex+5 ),
            /* Min */
            (int)readI2CEeprom( saveSendIndex+6 ),
            /* �}�[�J�[ */
            (int)readI2CEeprom( saveSendIndex+7 )

        );

        saveSendIndex += 16;            /* ���̑��M����                 */
        break;

    case 107:
        /* �]���I�� */
        led_out( 0xf );
        break;

	case 2000:
		if(cnt0 > 500){
			if(R_maker == 1){
			    setBeepPatternS( 0x8000 );
				beep_out(1);
			}else if(L_maker == 1){
			    setBeepPatternS( 0xcc00 );
				beep_out(1);
			}else{
				beep_out(0);
			}
//			raw_view();
			bi_view();
			cnt0 = 0;
		}
		break;
	
        default:
            break;
        }
    }

}
/************************************************************************/
/* �u�U�[����                                                           */
/* �����@0:�u�U�[OFF 1:�u�U�[ON                                         */
/************************************************************************/
void beep_out( int flag )
{
    if( flag ) {
        P6DR |= 0x10;
    } else {
        P6DR &= 0xef;
    }
}
/************************************************************************/
/* end of file                                                          */
/************************************************************************/