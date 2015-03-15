/****************************************************************************/
/* �t�@�C����   i2c_eeprom.c                                                */
/* ̧�ٓ��e     I2C�ʐM�d�l��24C256�̐��� C�\�[�X�� Ver3.01                 */
/* Date         2007.02.19                                                  */
/* Copyright    �W���p���}�C�R���J�[�����[���s�ψ���                        */
/****************************************************************************/

/*
�g����

��initI2CEeprom�֐�

EEP-ROM�̏������݂������܂��B�ŏ��ɕK�����s���܂��B
���@���FEEP-ROM�̌q�����Ă���DDR�|�[�g�̎w��(&��t����)
        EEP-ROM�̌q�����Ă���DR�|�[�g�̎w��(&��t����)
        DDR�|�[�g�̓��o�͐ݒ�l
        EEP-ROM��SCL�[�q�̌q�����Ă���r�b�g�ԍ�
        EEP-ROM��SDA�[�q�̌q�����Ă���r�b�g�ԍ�
�߂�l�F�Ȃ�

��jinitI2CEeprom( &PADDR, &PADR, 0x5f, 7, 5 );
    EEP-ROM�̓|�[�gA�Ɍq�����Ă���
    ���o�͐ݒ��0x57
    SCL�[�q��bit7�ɐڑ��ASDA�[�q��bit5�ɐڑ�

��selectI2CEepromAddress�֐�

I2C�o�X�ɐڑ�����Ă���ǂ�EEP-ROM���g�p���邩�I�����܂��B
���@���Funsigned char   EEP-ROM�̃A�h���X 0�`3
�߂�l�F�Ȃ�

��jselectI2CEepromAddress( 1 );
    1�Ԃ��g�p
    EEP-ROM A2="0" A1="0" A0="1"�ɐڑ�����Ă���EEP-ROM��I������Ƃ���
    ���ƂɂȂ�܂��B

��readI2CEeprom�֐�

EEP-ROM����f�[�^��ǂݍ��݂܂��B
���@���Funsigned int �A�h���X 0-32767
�߂�l�Fchar �f�[�^

��) i = readI2CEeprom( 0x0005 );
    EEP-ROM��0x0005�Ԓn�̃f�[�^��ϐ�i�ɑ�����܂�

��writeI2CEeprom�֐�

EEP-ROM�փf�[�^���������݂܂��B
10ms�͏������ݍ�ƒ��̂��߁A�A�N�Z�X�ł��܂���B
���@���Funsigned int �A�h���X 0-32767 ,char �f�[�^
�߂�l�F�Ȃ�

��jwriteI2CEeprom( 0x2000, -100 );
    EEP-ROM��0x2000�Ԓn��-100���������݂܂�

��pageWriteI2CEeprom�֐�

EEP-ROM�֕����o�C�g�̃f�[�^���������݂܂��B�������ݍ�Ƃ��I���܂�
���̊֐��͏I�����܂���B
�������݌�A10ms�͏������ݍ�ƒ��̂��߁A�A�N�Z�X�ł��܂���B
�f�[�^�͂U�S�o�C�g�܂Ŏw��ł��܂��B
���@���Funsigned int �A�h���X 0-32767 , int �� ,char* �f�[�^������A�h���X
�߂�l�F�Ȃ�

��jchar d[ 4 ];
    d[0]=5; d[1]=4; d[2]=1; d[3]=10;
    pageWriteI2CEeprom( 0x1000, 4, d );
    EEP-ROM��0x1000�Ԓn����A�z�񂄂ɂ���f�[�^���S�o�C�g���������݂܂�

��clearI2CEeprom

EEP-ROM�̃f�[�^���I�[���N���A���܂��B
���@���F�Ȃ�
�߂�l�F�Ȃ�

��jclearI2CEeprom();

��checkI2CEeprom�֐�

�������݌�A�������ݍ�Ƃ��I��������ǂ����`�F�b�N���܂��B
���@���F�Ȃ�
�߂�l�F1:���ǂݏ���OK  0:�܂�

��jwhile( !checkI2CEeprom() );
    �������݂��I���������`�F�b�N���܂��B

��setPageWriteI2CEeprom�֐� 2006.02.22�ǉ�

EEP-ROM�֕����o�C�g�̃f�[�^���������݂܂��B�������ݏ������s��������
�����ɏI�����܂��B���ۂ̏������݂�I2CEepromProcess�֐��ōs���܂��B
�f�[�^��64�o�C�g�܂Ŏw��ł��܂��B
���@���Funsigned int �A�h���X 0-32767 , int �� ,char* �f�[�^������A�h���X
�߂�l�F�Ȃ�

��jchar d[ 4 ];
    d[0]=5; d[1]=4; d[2]=1; d[3]=10;
    setPageWriteI2CEeprom( 0x1000, 4, d );  �������ݏ����̂�

    while( 1 ) {
        I2CEepromProcess();                 ���ۂ̏������݂͂��̊֐�
    }

��I2CEepromProcess�֐� 2006.02.22�ǉ�

setPageWriteI2CEeprom�֐��ŏ������ݏ������s���܂��B
���̊֐��͎��ۂɁAEEP-ROM�ɏ������ݍ�Ƃ��s���܂��B
�������A�������������ݍ�Ƃ��s���āA�������݂����ŏ������L���Ȃ��悤��
���Ă��܂��B���̊֐��́A�������݃f�[�^���{�T��ȏ���s���Ă��������B
�ʏ�͏����̃��[�v���ɓ���Ă����܂��B�������ݍ�Ƃ��Ȃ��Ƃ��͉������܂���
�̂ŁA����Ă��������łn�j�ł��B
���@���F�Ȃ�
�߂�l�F�Ȃ�

��jmain {

        init();

        while( 1 ) {
            I2CEepromProcess();             ��Ɏ��s����悤�ɂ���

            ���̑��̏���
        }
    }

*/

/*======================================*/
/* �C���N���[�h                         */
/*======================================*/
#include    <machine.h>
#include    "i2c_eeprom.h"

/*======================================*/
/* �萔�ݒ�                             */
/*======================================*/

/*
setPageWriteI2CEeprom�֐��P��ŕۑ��o����T�C�Y
*/
#define SAVE_SIZE       16              /* �ۑ���                   */

/* �E�G�C�g���K�v�ȏꍇ��1�A�Ȃ��̏ꍇ��0�Ƃ��� */
#if 0
#define I2C_WAIT()      i2c_wait();
#else
#define I2C_WAIT()      ;
#endif

/* �}�N���Ŋ֐���` */
#define i2c_SDA_0();    *dr_port &= ~i2c_sda;\
                        *ddr_port = (ddr_data |= i2c_sda);   /* SDA="0" */
#define i2c_SDA_1();    *ddr_port = (ddr_data &= ~i2c_sda);  /* SDA="1" */
#define i2c_SCL_0();    *dr_port &= ~i2c_scl;\
                        *ddr_port = (ddr_data |= i2c_scl);   /* SCL="0" */
#define i2c_SCL_1();    *ddr_port = (ddr_data &= ~i2c_scl);  /* SCL="1" */

/* ���̑���` */
#define ACK             0               /* ���[�h����ACK�L��(ACK=0) */
#define NO_ACK          1               /* ���[�h����ACK����(ACK=1) */

/* �f�[�^�ۑ��֘A */
int             saveIndex;              /* �ۑ��C���f�b�N�X             */
int             saveSendIndex;          /* ���M�C���f�b�N�X             */
int             saveFlag;               /* �ۑ��t���O                   */
char            saveData[16];           /* �ꎞ�ۑ��G���A               */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*  �v���O�����G���A                                                    */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static unsigned char*   ddr_port;           /* EEP-ROM�̂���DDR�|�[�g   */
static unsigned char*   dr_port;            /* EEP-ROM�̂���DR�|�[�g    */
static unsigned char    ddr_data;           /* ���o�͐ݒ�l             */
static unsigned char    i2c_scl;            /* SCL�r�b�g                */
static unsigned char    i2c_sda;            /* SDA�r�b�g                */

static unsigned char    eep_address;        /* �A�h���X�̑I��           */
static int              write_mode;         /* �������e                 */
static unsigned char    write_eep_address;  /* ��������EEP-ROM�̃A�h���X*/
static unsigned int     write_address;      /* �������݃A�h���X         */
static int              write_count;        /* �������݌�             */
static char             write_buff[SAVE_SIZE];/* �������݃f�[�^�ۑ��o�b�t�@*/
static char             *write_buff_p;      /* �������݃f�[�^�ǂݍ��݈ʒu*/

/************************************************************************/
/* ���W���[���� i2c_wait                                                */
/* �����T�v     EEP-ROM�^�C�~���O�p�E�G�C�g                             */
/************************************************************************/
void i2c_wait( void )
{
    int     i=1;    /* ��������ꍇ���̐����𒲐����� */

    while( i-- );
}

/************************************************************************/
/* ���W���[���� i2c_start                                               */
/* �����T�v     �X�^�[�g�M�����M                                        */
/************************************************************************/
void i2c_start( void )
{
    i2c_SDA_1();
    i2c_SCL_1();
    I2C_WAIT();

    i2c_SDA_0();
    i2c_SCL_1();
    I2C_WAIT();

    i2c_SDA_0();
    i2c_SCL_0();
    I2C_WAIT();
}

/************************************************************************/
/* ���W���[���� i2c_stop                                                */
/* �����T�v     �X�g�b�v�M�����M                                        */
/************************************************************************/
void i2c_stop( void )
{
    i2c_SDA_0();
    i2c_SCL_0();
    I2C_WAIT();

    i2c_SDA_0();
    i2c_SCL_1();
    I2C_WAIT();

    i2c_SDA_1();
    i2c_SCL_1();
    I2C_WAIT();
}

/************************************************************************/
/* ���W���[���� i2c_write                                               */
/* �����T�v     EEP-ROM�@�P�o�C�g��������                               */
/* ����         char            �f�[�^                                  */
/* �߂�l       int             acknowledge 0:�L��  1:����              */
/************************************************************************/
int i2c_write( char data )
{
    int     i, ret;

    for( i = 0; i<8; i++ ) {
        /* data set */
        if( data & 0x80 ) {
            i2c_SDA_1();
        } else {
            i2c_SDA_0();
        }

        /* SCL ON */
        i2c_SCL_1();
        I2C_WAIT();

        /* SCL OFF  */
        i2c_SCL_0();
        I2C_WAIT();

        /* ���̏��� */
        data <<= 1;
    }

    /* acknowledge read */
    i2c_SDA_1();
    I2C_WAIT();

    /* SCL ON */
    i2c_SCL_1();
    I2C_WAIT();

    ret = !!(*dr_port & i2c_sda);

    /* SCL OFF  */
    i2c_SCL_0();

    return ret;
}

/************************************************************************/
/* ���W���[���� i2c_read                                                */
/* �����T�v     EEP-ROM�@�P�o�C�g�ǂݍ���                               */
/* ����         int ACK�F�f�[�^������ NO_ACK�F�f�[�^�I��                */
/* �߂�l       char �f�[�^                                             */
/************************************************************************/
char i2c_read( int ack )
{
    char    ret;
    int     i;

    i2c_SDA_1();

    for( i = 0; i<8; i++ ) {
        ret <<= 1;

        /* SCL ON */
        i2c_SCL_1();
        I2C_WAIT();

        /* data read */
        if( *dr_port & i2c_sda ) {
            ret |= 0x01;
        }

        /* SCL OFF  */
        i2c_SCL_0();
    }

    /* acknowledge write */
    if( ack == ACK ) {
        i2c_SDA_0();
    } else {
        i2c_SDA_1();
    }

    /* SCL ON */
    i2c_SCL_1();
    I2C_WAIT();

    /* SCL OFF  */
    i2c_SCL_0();

    return ret;
}

/************************************************************************/
/* ���W���[���� initI2CEeprom                                           */
/* �����T�v     EEP-ROM�̃|�[�g������                                   */
/* ����         EEP-ROM�̌q�����Ă���DDR�|�[�g�̎w��(&��t����)         */
/*              EEP-ROM�̌q�����Ă���DR�|�[�g�̎w��(&��t����)          */
/*              DDR�|�[�g�̓��o�͐ݒ�l                                 */
/*              EEP-ROM��SCL�[�q�̌q�����Ă���r�b�g�ԍ�                */
/*              EEP-ROM��SDA�[�q�̌q�����Ă���r�b�g�ԍ��Ȃ�            */
/* �߂�l       �Ȃ�                                                    */
/************************************************************************/
void initI2CEeprom( unsigned char* ddrport, unsigned char* drport,
            unsigned char ddrdata, unsigned char scl, unsigned char sda )
{
    ddr_port    = ddrport;
    dr_port     = drport;
    ddr_data    = ddrdata;
    i2c_scl     = 0x01 << scl;
    i2c_sda     = 0x01 << sda;

    *dr_port    = *dr_port & (~i2c_scl) & (~i2c_sda);
    *ddr_port   = ddr_data;
    selectI2CEepromAddress( 0 );        /* �A�h���X�̑I��           */
    write_mode  = 0;
}

/************************************************************************/
/* ���W���[���� selectI2CEepromAddress                                  */
/* �����T�v     �ǂ̔ԍ���EEP-ROM���g�p���邩�I��                       */
/* ����         unsigned char   EEP-ROM�̃A�h���X 0�`3                  */
/* �߂�l       �Ȃ�                                                    */
/************************************************************************/
void selectI2CEepromAddress( unsigned char address )
{
    address &= 0x03;
    eep_address = address << 1;
}

/************************************************************************/
/* ���W���[���� readI2CEeprom                                           */
/* �����T�v     EEP-ROM�ǂݍ���                                         */
/* ����         unsigned long   �A�h���X 0-32767                        */
/* �߂�l       char            �f�[�^                                  */
/************************************************************************/
char readI2CEeprom( unsigned long address )
{
    char    ret;

    or_ccr( 0x80 );

    i2c_start();
    i2c_write( 0xa0 | eep_address);     /* device address(write)    */
    i2c_write( (address >> 8) & 0xff ); /* first address            */
    i2c_write( address & 0xff );        /* second address           */
    i2c_start();
    i2c_write( 0xa1 | eep_address);     /* device address(read)     */
    ret = i2c_read( NO_ACK );           /* data read                */
    i2c_stop();

    and_ccr( 0x7f );

    return ret;
}

/************************************************************************/
/* ���W���[���� writeI2CEeprom                                          */
/* �����T�v     EEP-ROM��������                                         */
/* ����         unsigned long �A�h���X 0-32767 ,char �f�[�^             */
/* �߂�l       �Ȃ�                                                    */
/************************************************************************/
void writeI2CEeprom( unsigned long address, char write )
{
    or_ccr( 0x80 );

    i2c_start();
    i2c_write( 0xa0 | eep_address );    /* device address(write)    */
    i2c_write( (address >> 8) & 0xff ); /* first address            */
    i2c_write( address & 0xff );        /* second address           */
    i2c_write( write );                 /* data write               */
    i2c_stop();

    and_ccr( 0x7f );
}

/************************************************************************/
/* ���W���[���� pageWriteI2CEeprom                                      */
/* �����T�v     EEP-ROM�y�[�W��������                                   */
/*              ���̊֐����őS�Ă̏������I��炷                        */
/*              �������A���Ԃ�������                                    */
/* ����         unsigned long �A�h���X 0-32767 , int ��(1-64) ,       */
/*              char* �f�[�^������A�h���X                              */
/* �߂�l       �Ȃ�                                                    */
/************************************************************************/
void pageWriteI2CEeprom( unsigned long address, int count, char* data )
{
    or_ccr( 0x80 );

    i2c_start();
    i2c_write( 0xa0 | eep_address );    /* device address(write)    */
    i2c_write( (address >> 8) & 0xff ); /* first address            */
    i2c_write( address & 0xff );        /* second address           */
    do {
        i2c_write( *data );             /* data write               */
        data++;
    } while( --count );
    i2c_stop();

    and_ccr( 0x7f );
}

/************************************************************************/
/* ���W���[���� setPageWriteI2CEeprom                                   */
/* �����T�v     EEP-ROM�y�[�W��������                                   */
/*              ���̊֐��Ńy�[�W�������݂��J�n����                      */
/* ����         unsigned long �A�h���X 0-32767 , int ��,              */
/*              char* �f�[�^������A�h���X                              */
/* �߂�l       �Ȃ�                                                    */
/************************************************************************/
void setPageWriteI2CEeprom( unsigned long address, int count, char* data )
{
    /* �������ݒ��Ȃ炱�̊֐��͒��~ */
    if( write_mode != 0 ) return;

    write_mode          = 1;
    write_eep_address   = eep_address;
    write_address       = address;
    write_count         = count;
    write_buff_p        = write_buff;

    if( count >= SAVE_SIZE ) count = SAVE_SIZE;
    do {
        *write_buff_p++ = *data++;
    } while( --count );

    write_buff_p = write_buff;
}

/************************************************************************/
/* ���W���[���� I2CEepromProcess                                        */
/* �����T�v     EEP-ROM�y�[�W��������                                   */
/* ����         �Ȃ�                                                    */
/* �߂�l       �Ȃ�                                                    */
/************************************************************************/
void I2CEepromProcess( void )
{
    or_ccr( 0x80 );

    switch( write_mode ) {
    case 1:
        i2c_start();
        write_mode = 2;
        break;
    case 2:
        i2c_write( 0xa0 | write_eep_address );  /* device address(write)*/
        write_mode = 3;
        break;
    case 3:
        i2c_write( (write_address >> 8) & 0xff );   /* first address    */
        write_mode = 4;
        break;
    case 4:
        i2c_write( write_address & 0xff );          /* second address   */
        write_mode = 5;
        break;
    case 5:
        i2c_write( *write_buff_p++ );               /* data write       */
        if( !(--write_count) ) write_mode = 6;
        break;
    case 6:
        i2c_stop();
        write_mode = 0;
        break;
    }

    and_ccr( 0x7f );
}

/************************************************************************/
/* ���W���[���� clearI2CEeprom                                          */
/* �����T�v     EEP-ROM�@�I�[���N���A                                   */
/* ����         �Ȃ�                                                    */
/* �߂�l       �Ȃ�                                                    */
/************************************************************************/
void clearI2CEeprom( void )
{
    unsigned int    address = 0;
    int             i;

    or_ccr( 0x80 );

    while( 1 ) {
        i2c_start();
        i2c_write( 0xa0 | eep_address );    /* device address(write)*/
        i2c_write( address >> 8 );          /* first address        */
        i2c_write( address & 0xff );        /* second address       */
        for( i=0; i<64; i++ ) {
            i2c_write( 0 );                 /* data write           */
        }
        i2c_stop();

        while( !checkI2CEeprom() );         /* wait                 */

        address += 64;
        if( address >= 32768 ) break;
    }

    and_ccr( 0x7f );
}

/************************************************************************/
/* ���W���[���� checkI2CEeprom                                          */
/* �����T�v     EEP-ROM�������݌�A���ɓǂݏ����ł��邩�`�F�b�N         */
/* ����         �Ȃ�                                                    */
/* �߂�l       1:���ǂݏ���OK 0:�܂�                                   */
/************************************************************************/
int checkI2CEeprom( void )
{
    int ret;

    or_ccr( 0x80 );

    i2c_start();
    ret = !i2c_write( 0xa0 | eep_address ); /* device address(write)*/
    i2c_stop();

    and_ccr( 0x7f );

    return ret;
}

/************************************************************************/
/* end of file                                                          */
/************************************************************************/