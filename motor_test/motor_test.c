/****************************************************************************/
/* ���[�^�h���C�u���TypeS �e�X�g�v���O���� Ver.1.01                        */
/*                          2007.12 �W���p���}�C�R���J�[�����[���s�ψ���    */
/****************************************************************************/

/*======================================*/
/* �C���N���[�h                         */
/*======================================*/
#include    <no_float.h>                /* stdio�̊ȗ��� �ŏ��ɒu��     */
#include    <stdio.h>                   /* printf�ȂǗp                 */
#include    <machine.h>                 /* �g�ݍ��݊֐��p               */
#include    "h8_3048.h"                 /* H8/3048F-ONE���W�X�^��`     */
#include    "i2c_eeprom.h"              /* EEP-ROM�ǉ�(�f�[�^�L�^)      */

/*======================================*/
/* �V���{����`                         */
/*======================================*/

/* �萔�ݒ� */
#define     MOTOR_CYCLE         3072    /* ���[�^PWM�̃T�C�N�����A      */
                                        /*          �^�C�}�l 1ms        */
#define     MOTOR_OFFSET        37      /* �p���X�����炷����           */
                                        /* 1=325.52[ns]                 */

#define     FREE                1       /* ���[�^���[�h�@�t���[         */
#define     BRAKE               0       /* ���[�^���[�h�@�u���[�L       */

/*======================================*/
/* �v���g�^�C�v�錾                     */
/*======================================*/
void init( void );
unsigned char sensor_inp( void );
unsigned char center_inp( void );
unsigned char startbar_get( void );
unsigned char dipsw_get( void );
unsigned char dipsw_get2( void );
unsigned char pushsw_get( void );
unsigned char cn8_get( void );
void led_out( unsigned char led );
void speed_r( int accele_l, int accele_r );
void speed2_r( int accele_l, int accele_r );
void speed_f( int accele_l, int accele_r );
void speed2_f( int accele_l, int accele_r );
void motor_mode_r( int mode_l, int mode_r );
void motor_mode_f( int mode_l, int mode_r );
void servoPwmOut( int pwm );
void motor_mode_s( int mode );
void beep_out( int flag );

/*======================================*/
/* �O���[�o���ϐ��̐錾                 */
/*======================================*/
int             pattern;                /* �}�C�R���J�[����p�^�[��     */
unsigned long   cnt1;                   /* �^�C�}�p                     */
int             iPWM_l, iPWM_r, iPWM_s; /* ����PWM�ݒ�o�b�t�@�p        */

/* �G���R�[�_�֘A */
int             iTimer10;               /* 10ms�J�E���g�p               */
long            lEncoderTotal;          /* �ώZ�l�ۑ��p                 */
int             iEncoderMax;            /* 10ms���̒l�̍ő�l�ۑ��p     */
int             iEncoder;               /* 10ms���̍ŐV�l               */
unsigned int    uEncoderBuff;           /* �v�Z�p�@���荞�ݓ��Ŏg�p     */

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
    init_sci1( 0x00, 79 );              /* SCI1������                   */
    set_ccr( 0x00 );                    /* �S�̊��荞�݋���             */

    /* �}�C�R���J�[�̏�ԏ����� */
    speed_r( 0, 0 );
    speed_f( 0, 0 );
    servoPwmOut( 0 );

    while( 1 ) {
        switch( pattern ) {
        case 0:
            /* ���j���[ */
            printf( "\n\n" );
            printf( "Motor Drive PCB TypeS Test Program Ver1.00\n" );
            printf( "\n" );
            printf( "1 : LED�̃e�X�g\n" );
            printf( "2 : �X�C�b�`�̃e�X�g\n" );
            printf( "3 : CN8�̓��̓e�X�g\n" );
            printf( "4 : �u�U�[�̃e�X�g\n" );
            printf( "5 : �G���R�[�_�̃e�X�g\n" );
            printf( "6 : EEP-ROM�̃e�X�g\n" );
            printf( "7 : �{�����[���̃e�X�g\n" );
            printf( "8 : �A�i���O�Z���T��̃e�X�g\n" );
            printf( "9 : ���[�^�̃e�X�g\n" );
            printf( "\n" );
            printf( "1-9�̐�������͂��Ă������� " );
            pattern = 1;
            break;

        case 1:
            if( get_sci(&c) ) {
                if( c >= '1' && c <= '9' ) {
                    printf( "%c\n\n" , c );
                    cnt1 = 0;
                    pattern = (c - 0x30) * 10 + 1;
                }
            }
            break;

        case 11:
            /* LED�̃e�X�g */
            printf( "����LED2�`5�����Ԃɓ_�����ł��B" );
            printf( "�I�������ǂꂩ�L�[�������Ă��������B\n" );
            pattern = 12;
            break;

        case 12:
            led_out( 1 << (cnt1 / 500) );
            if( cnt1 >= 2000 ) cnt1 = 0;
            if( get_sci(&c) == 1 ) {
                led_out( 0 );
                pattern = 0;
            }
            break;

        case 21:
            /* �X�C�b�`�̃e�X�g */
            printf( "���݃X�C�b�`�̒l��\�����ł��B" );
            printf( "�I�������ǂꂩ�L�[�������Ă��������B\n" );
            pattern = 22;
            break;

        case 22:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            if( cnt1 >= 200 ) {
                cnt1 = 0;
                printf( "�f�B�b�vSW(SW3)�̒l : %02x ", dipsw_get2() );
                printf( "�v�b�V��SW(SW2)�̒l : %1x\r", pushsw_get() );
            }
            break;

        case 31:
            /* CN8�̓��̓e�X�g */
            printf( "CN8�ɓ��͂���Ă���l��\�����ł��B" );
            printf( "�I�������ǂꂩ�L�[�������Ă��������B\n" );
            pattern = 32;
            break;

        case 32:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            if( cnt1 >= 200 ) {
                cnt1 = 0;
                printf( "CN8�̓��͒l : %1x\r", cn8_get() );
            }
            break;

        case 41:
            /* �u�U�[�̃e�X�g */
            printf( "�u�U�[���e�X�g���܂��B" );
            printf( "�I�������ǂꂩ�L�[�������Ă��������B\n" );
            pattern = 42;
            break;

        case 42:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                beep_out( 0 );
                break;
            }
            if( cnt1 <= 1000 ) {
                beep_out( 1 );
            } else if( cnt1 <= 2000 ) {
                beep_out( 0 );
            } else {
                cnt1 = 0;
            }
            break;

        case 51:
            /* �G���R�[�_�̃e�X�g */
            printf( "�G���R�[�_�̓��̓p���X����\�����܂��B" );
            printf( "�I�������ǂꂩ�L�[�������Ă��������B\n" );
            lEncoderTotal = 0;
            pattern = 52;
            break;

        case 52:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            if( cnt1 >= 200 ) {
                cnt1 = 0;
                printf( "�G���R�[�_�̃p���X�l : %8ld\r", lEncoderTotal );
            }
            break;

        case 61:
            /* EEP-ROM�̃e�X�g */
            printf( "EEP-ROM������ɏ������߂邩�`�F�b�N���܂��B" );
            printf( "�`�F�b�N���̓L�[���͂��󂯕t���܂���B" );
            printf( "�`�F�b�N���܂����H(Y or N) " );
            pattern = 62;
            break;

        case 62:
            if( get_sci(&c) == 1 ) {
                switch( c ) {
                case 'Y':
                case 'y':
                    printf( "y\n" );
                    pattern = 63;
                    break;
                case 'N':
                case 'n':
                    printf( "n\n" );
                    pattern = 0;
                    break;
                }
            }
            break;

        case 63:
            for( u=0; u<0x8000 ; u++ ) {
                if( u % 0x80 == 0 ) {
                    printf( "%3ld%%�`�F�b�N����...\r",
                                        (long)u * 100 / 0x8000 );
                }
                writeI2CEeprom( u, 0xaa );
                while( !checkI2CEeprom() );
                if( (unsigned char)readI2CEeprom( u ) != 0xaa ) {
                    break;
                }

                writeI2CEeprom( u, 0x55 );
                while( !checkI2CEeprom() );
                if( (unsigned char)readI2CEeprom( u ) != 0x55 ) {
                    break;
                }

                writeI2CEeprom( u, 0x00 );
                while( !checkI2CEeprom() );
                if( (unsigned char)readI2CEeprom( u ) != 0x00 ) {
                    break;
                }
            }
            printf( "\n" );
            if( u == 0x8000 ) {
                printf( "EEP-ROM�͐���ł����B\n" );
           } else {
                printf( "EEP-ROM�Ɉُ킪����܂����B" );
                printf( "���c�s�ǂȂǊm�F���Ă��������B\n" );
            }
            pattern = 64;
            break;

        case 64:
            printf( "�ǂꂩ�L�[�������Ă��������B" );
            while( get_sci(&c) );
            pattern = 65;
            break;

        case 65:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            break;

        case 71:
            /* �{�����[���̃e�X�g */
            printf( "�{�����[���d����\�����ł��B" );
            printf( "�I�������ǂꂩ�L�[�������Ă��������B\n" );
            pattern = 72;
            break;

        case 72:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            if( cnt1 >= 200 ) {
                cnt1 = 0;
                i =  AD_DRB >> 6;
                j = (long)500 * i / 1023;
                printf( "�{�����[���̓d�� : %4d (%01d.%02dV)\r",
                                                i, j/100, j%100 );
            }
            break;

        case 81:
            /* �A�i���O�Z���T��̃e�X�g */
            printf( "�A�i���O�Z���T��̒l��\�����ł��B" );
            printf( "�I�������ǂꂩ�L�[�������Ă��������B\n" );
            pattern = 82;
            break;

        case 82:
            if( get_sci(&c) == 1 ) {
                pattern = 0;
                break;
            }
            if( cnt1 >= 200 ) {
                cnt1 = 0;
                printf( "Left=%4d , Right=%4d , "
                        "Digital=%1x , Center=%1d , Bar=%1d\r",
                            AD_DRD>>6, AD_DRC>>6, sensor_inp(),
                            center_inp(), startbar_get() );
            }
            break;

        case 91:
            /* ���[�^�̃e�X�g */
            printf( "���[�^�̃e�X�g�����܂��B\n" );
            printf( "1 : ����냂�[�^\n" );
            printf( "2 : �E��냂�[�^\n" );
            printf( "3 : �T�[�{���[�^\n" );
            printf( "4 : ���O���[�^\n" );
            printf( "5 : �E�O���[�^\n" );
            printf( "0 : �I��\n" );
            printf( "\n" );
            printf( "0-5�̐�������͂��Ă������� " );
            pattern = 92;
            break;

        case 92:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    pattern = 0;
                    break;
                } else if( c >= '1' && c <= '5' ) {
                    printf( "%c\n\n" , c );
                    cnt1 = 0;
                    pattern = (c - 0x31) * 10 + 101;
                }
            }
            break;

        case 101:
            printf( "����냂�[�^���e�X�g���܂��B\n" );
            printf( "�X�y�[�X�L�[�������ē����ς��܂��B" );
            printf( "0�L�[�ŏI���ł��B\n" );
            cnt1 = 0;
            i = 0;
            j = 1;
            pattern = 102;
            break;

        case 102:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                    pattern = 91;
                    break;
                } else if( c == ' ' ) {
                    cnt1 = 0;
                    i++;
                    if( i >= 5 ) i = 0;
                }
            }
            switch( i ) {
            case 0:
                if( i != j ) {
                    printf( "��~��                     \r" );
                    j = i;
                }
                speed2_r( 0, 0 );
                motor_mode_r( BRAKE , BRAKE );
                break;
            case 1:
                if( i != j ) {
                    printf( "���]50%% <-> �u���[�L���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 50, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2:
                if( i != j ) {
                    printf( "�t�]50%% <-> �u���[�L���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( -50, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 3:
                if( i != j ) {
                    printf( "���]50%% <-> �t���[�@���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 50, 0 );
                    motor_mode_r( FREE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( FREE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 4:
                if( i != j ) {
                    printf( "�t�]50%% <-> �t���[�@���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( -50, 0 );
                    motor_mode_r( FREE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( FREE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            }
            break;

        case 111:
            printf( "�E��냂�[�^���e�X�g���܂��B\n" );
            printf( "�X�y�[�X�L�[�������ē����ς��܂��B" );
            printf( "0�L�[�ŏI���ł��B\n" );
            cnt1 = 0;
            i = 0;
            j = 1;
            pattern = 112;
            break;

        case 112:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                    pattern = 91;
                    break;
                } else if( c == ' ' ) {
                    cnt1 = 0;
                    i++;
                    if( i >= 5 ) i = 0;
                }
            }
            switch( i ) {
            case 0:
                if( i != j ) {
                    printf( "��~��                     \r" );
                    j = i;
                }
                speed2_r( 0, 0 );
                motor_mode_r( BRAKE , BRAKE );
                break;
            case 1:
                if( i != j ) {
                    printf( "���]50%% <-> �u���[�L���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 0, 50 );
                    motor_mode_r( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2:
                if( i != j ) {
                    printf( "�t�]50%% <-> �u���[�L���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 0, -50 );
                    motor_mode_r( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 3:
                if( i != j ) {
                    printf( "���]50%% <-> �t���[�@���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 0, 50 );
                    motor_mode_r( BRAKE , FREE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 4:
                if( i != j ) {
                    printf( "�t�]50%% <-> �t���[�@���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_r( 0, -50 );
                    motor_mode_r( BRAKE , FREE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_r( 0, 0 );
                    motor_mode_r( BRAKE , FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            }
            break;

        case 121:
            printf( "�T�[�{���[�^���e�X�g���܂��B\n" );
            printf( "�X�y�[�X�L�[�������ē����ς��܂��B" );
            printf( "0�L�[�ŏI���ł��B\n" );
            cnt1 = 0;
            i = 0;
            j = 1;
            pattern = 122;
            break;

        case 122:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    servoPwmOut( 0 );
                    motor_mode_s( BRAKE );
                    pattern = 91;
                    break;
                } else if( c == ' ' ) {
                    cnt1 = 0;
                    i++;
                    if( i >= 5 ) i = 0;
                }
            }
            switch( i ) {
            case 0:
                if( i != j ) {
                    printf( "��~��                     \r" );
                    j = i;
                }
                servoPwmOut( 0 );
                motor_mode_s( BRAKE );
                break;
            case 1:
                if( i != j ) {
                    printf( "���]50%% <-> �u���[�L���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    servoPwmOut( 50 );
                    motor_mode_s( BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    servoPwmOut( 0 );
                    motor_mode_s( BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2:
                if( i != j ) {
                    printf( "�t�]50%% <-> �u���[�L���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    servoPwmOut( -50 );
                    motor_mode_s( BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    servoPwmOut( 0 );
                    motor_mode_s( BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 3:
                if( i != j ) {
                    printf( "���]50%% <-> �t���[�@���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    servoPwmOut( 50 );
                    motor_mode_s( FREE );
                } else if(  cnt1 <= 2000 ) {
                    servoPwmOut( 0 );
                    motor_mode_s( FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 4:
                if( i != j ) {
                    printf( "�t�]50%% <-> �t���[�@���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    servoPwmOut( -50 );
                    motor_mode_s( FREE );
                } else if(  cnt1 <= 2000 ) {
                    servoPwmOut( 0 );
                    motor_mode_s( FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            }
            break;

        case 131:
            printf( "���O���[�^���e�X�g���܂��B\n" );
            printf( "�X�y�[�X�L�[�������ē����ς��܂��B" );
            printf( "0�L�[�ŏI���ł��B\n" );
            cnt1 = 0;
            i = 0;
            j = 1;
            pattern = 132;
            break;

        case 132:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                    pattern = 91;
                    break;
                } else if( c == ' ' ) {
                    cnt1 = 0;
                    i++;
                    if( i >= 5 ) i = 0;
                }
            }
            switch( i ) {
            case 0:
                if( i != j ) {
                    printf( "��~��                     \r" );
                    j = i;
                }
                speed2_f( 0, 0 );
                motor_mode_f( BRAKE , BRAKE );
                break;
            case 1:
                if( i != j ) {
                    printf( "���]50%% <-> �u���[�L���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 50, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2:
                if( i != j ) {
                    printf( "�t�]50%% <-> �u���[�L���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( -50, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 3:
                if( i != j ) {
                    printf( "���]50%% <-> �t���[�@���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 50, 0 );
                    motor_mode_f( FREE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( FREE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 4:
                if( i != j ) {
                    printf( "�t�]50%% <-> �t���[�@���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( -50, 0 );
                    motor_mode_f( FREE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( FREE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            }
            break;

        case 141:
            printf( "�E��냂�[�^���e�X�g���܂��B\n" );
            printf( "�X�y�[�X�L�[�������ē����ς��܂��B" );
            printf( "0�L�[�ŏI���ł��B\n" );
            cnt1 = 0;
            i = 0;
            j = 1;
            pattern = 142;
            break;

        case 142:
            if( get_sci(&c) ) {
                if( c == '0' ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                    pattern = 91;
                    break;
                } else if( c == ' ' ) {
                    cnt1 = 0;
                    i++;
                    if( i >= 5 ) i = 0;
                }
            }
            switch( i ) {
            case 0:
                if( i != j ) {
                    printf( "��~��                     \r" );
                    j = i;
                }
                speed2_f( 0, 0 );
                motor_mode_f( BRAKE , BRAKE );
                break;
            case 1:
                if( i != j ) {
                    printf( "���]50%% <-> �u���[�L���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 0, 50 );
                    motor_mode_f( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 2:
                if( i != j ) {
                    printf( "�t�]50%% <-> �u���[�L���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 0, -50 );
                    motor_mode_f( BRAKE , BRAKE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , BRAKE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 3:
                if( i != j ) {
                    printf( "���]50%% <-> �t���[�@���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 0, 50 );
                    motor_mode_f( BRAKE , FREE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            case 4:
                if( i != j ) {
                    printf( "�t�]50%% <-> �t���[�@���쒆\r" );
                    j = i;
                }
                if( cnt1 <= 1000 ) {
                    speed2_f( 0, -50 );
                    motor_mode_f( BRAKE , FREE );
                } else if(  cnt1 <= 2000 ) {
                    speed2_f( 0, 0 );
                    motor_mode_f( BRAKE , FREE );
                } else {
                    cnt1 = 0;
                }
                break;
            }
            break;

        default:
            break;
        }
    }
}

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
    P8DDR = 0xe0;
    P9DDR = 0xf7;
    PADDR = 0xd6;
    PBDDR = 0xff;

    /* A/D�̏����ݒ� */
    AD_CSR = 0x1b;                      /* �X�L�������[�h�g�pAN0-AN3    */
    AD_CSR |= 0x20;                     /* AD�X�^�[�g                   */

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

    /* ITU3,4 �E�A�����[�^�A�X�e�A�����O */
    ITU3_TCR = 0x03;
    ITU4_TCR = 0x03;
    ITU_FCR  = 0x2e;                    /* ITU3,4�ő���PWM���[�h        */
    ITU_TOCR = 0x12;                    /* TIOCB3,TOCXA4,TOCXB4�͔��]   */
    ITU3_IER = 0x01;                    /* TCNT = GRA�ɂ�銄���ݐ���   */
    ITU3_CNT = MOTOR_OFFSET;
    ITU4_CNT = 0;
    ITU3_GRA = MOTOR_CYCLE / 2 + MOTOR_OFFSET - 1;  /* PWM�����ݒ�      */
    ITU3_BRB = ITU3_GRB = iPWM_r = MOTOR_CYCLE / 2;
    ITU4_BRA = ITU4_GRA = iPWM_l = MOTOR_CYCLE / 2;
    ITU4_BRB = ITU4_GRB = iPWM_s = MOTOR_CYCLE / 2;

    ITU_MDR = 0x03;                     /* PWM���[�h�ݒ�                */
    ITU_STR = 0x1f;                     /* ITU�̃J�E���g�X�^�[�g        */
}

/************************************************************************/
/* ITU3 ���荞�ݏ���                                                    */
/************************************************************************/
#pragma interrupt( interrupt_timer3 )
void interrupt_timer3( void )
{
    unsigned int    i;

    ITU3_TSR &= 0xfe;                   /* �t���O�N���A                 */
    ITU3_BRB = iPWM_r;                  /* PWM�l�̍X�V                  */
    ITU4_BRA = iPWM_l;                  /* PWM�l�̍X�V                  */
    ITU4_BRB = iPWM_s;                  /* PWM�l�̍X�V                  */
    cnt1++;

    /* �G���R�[�_���� */
    iTimer10++;
    if( iTimer10 >= 10 ) {
        iTimer10 = 0;
        i = ITU2_CNT;
        iEncoder       = i - uEncoderBuff;
        lEncoderTotal += iEncoder;
        if( iEncoder > iEncoderMax )
                    iEncoderMax = iEncoder;
        uEncoderBuff = i;
    }
}

/************************************************************************/
/* �Z���T��Ԍ��o                                                       */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Z���T�l                                                      */
/************************************************************************/
unsigned char sensor_inp( void )
{
    unsigned char sensor;

    sensor  = (~PADR & 0x08) | (~P8DR & 0x07);

    return sensor;
}

/************************************************************************/
/* ���S�Z���T���                                                       */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Z���T�l                                                      */
/************************************************************************/
unsigned char center_inp( void )
{
    unsigned char sensor;

    sensor  = ~PADR & 0x20;
    sensor  = !!sensor;

    return sensor;
}

/************************************************************************/
/* �X�^�[�g�o�[�Z���T���                                               */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Z���T�l                                                      */
/************************************************************************/
unsigned char startbar_get( void )
{
    unsigned char sensor;

    sensor  = ~P8DR & 0x08;
    sensor  = !!sensor;

    return sensor;
}

/************************************************************************/
/* �f�B�b�v�X�C�b�`�l�ǂݍ���                                           */
/* �߂�l �X�C�b�`�l 0�`15                                              */
/************************************************************************/
unsigned char dipsw_get( void )
{
    unsigned char sw;

    sw  = ~P6DR;                        /* �f�B�b�v�X�C�b�`�ǂݍ���     */
    sw &= 0x0f;

    return  sw;
}

/************************************************************************/
/* �f�B�b�v�X�C�b�`�l�ǂݍ���                                           */
/* �߂�l �X�C�b�`�l 0�`15                                              */
/************************************************************************/
unsigned char dipsw_get2( void )
{
    unsigned char sw;

    sw  = ~P2DR;                        /* ���[�^�h���C�u���sw�ǂݍ���*/

    return  sw;
}

/************************************************************************/
/* �v�b�V���X�C�b�`�l�ǂݍ���                                           */
/* �߂�l �X�C�b�`�l ON:1 OFF:0                                         */
/************************************************************************/
unsigned char pushsw_get( void )
{
    unsigned char sw;

    sw = ~P8DR & 0x10;                  /* �v�b�V���X�C�b�`�ǂݍ���     */
    sw = !!sw;

    return  sw;
}

/************************************************************************/
/* �v�b�V���X�C�b�`�l�ǂݍ���                                           */
/* �߂�l �X�C�b�`�l ON:1 OFF:0                                         */
/************************************************************************/
unsigned char cn8_get( void )
{
    unsigned char data;

    data = P7DR >> 4;

    return  data;
}

/************************************************************************/
/* LED����                                                              */
/* �����@4��LED���� 0�`15                                             */
/************************************************************************/
void led_out( unsigned char led )
{
    unsigned char data;

    led <<= 4;
    data = P1DR & 0x0f;
    P1DR = data | led;
}

/************************************************************************/
/* ��ւ̑��x����                                                       */
/* �����@ �����[�^:-100�`100 , �E���[�^:-100�`100                       */
/*        0�Œ�~�A100�Ő��]100%�A-100�ŋt�]100%                        */
/************************************************************************/
void speed_r( int accele_l, int accele_r )
{
    unsigned int    sw_data;
    unsigned int    work;

    sw_data  = dipsw_get() + 5;             /* �f�B�b�v�X�C�b�`�ǂݍ��� */
    sw_data *= 5;                           /* 5�`20 �� 25�`100�ɕϊ�   */

    /* �����[�^ */
    if( accele_l > 0 ) {
        PBDR &= 0xbf;
    } else if( accele_l < 0 ) {
        PBDR |= 0x40;
        accele_l = -accele_l;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET;
    work   = (long)work * accele_l * sw_data / 10000;
    iPWM_l = MOTOR_CYCLE / 2 - work;

    /* �E���[�^ */
    if( accele_r > 0 ) {
        PBDR &= 0x7f;
    } else if( accele_r < 0 ) {
        PBDR |= 0x80;
        accele_r = -accele_r;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET;
    work   = (long)work * accele_r * sw_data / 10000;
    iPWM_r = MOTOR_CYCLE / 2 - work;
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
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET;
    work   = (long)work * accele_l / 100;
    iPWM_l = MOTOR_CYCLE / 2 - work;

    /* �E���[�^ */
    if( accele_r > 0 ) {
        PBDR &= 0x7f;
    } else if( accele_r < 0 ) {
        PBDR |= 0x80;
        accele_r = -accele_r;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET;
    work   = (long)work * accele_r / 100;
    iPWM_r = MOTOR_CYCLE / 2 - work;
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

    sw_data  = dipsw_get() + 5;             /* �f�B�b�v�X�C�b�`�ǂݍ��� */
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
/* �ヂ�[�^����i�u���[�L�A�t���[�j                                     */
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
/* �O���[�^����i�u���[�L�A�t���[�j                                     */
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
/* �T�[�{���[�^����                                                     */
/* �����@ �T�[�{���[�^PWM�F-100�`100                                    */
/************************************************************************/
void servoPwmOut( int pwm )
{
    unsigned int    work;

    if( pwm > 0 ) {
        PADR &= 0x7f;
    } else if( pwm < 0 ) {
        PADR |= 0x80;
        pwm = -pwm;
    }
    work   = MOTOR_CYCLE / 2 - MOTOR_OFFSET;
    work   = (long)work * pwm / 100;
    iPWM_s = MOTOR_CYCLE / 2 - work;
}

/************************************************************************/
/* �T�[�{���[�^����i�u���[�L�A�t���[�j                                 */
/* �����@ FREE or BRAKE                                                 */
/************************************************************************/
void motor_mode_s( int mode )
{
    if( mode ) {
        P3DR |= 0x80;
    } else {
        P3DR &= 0x7f;
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