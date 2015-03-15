/*======================================*/
/* �萔�ݒ�                             */
/*======================================*/
/* �f�[�^�ۑ��֘A */
extern int             saveIndex;              /* �ۑ��C���f�b�N�X             */
extern int             saveSendIndex;          /* ���M�C���f�b�N�X             */
extern int             saveFlag;               /* �ۑ��t���O                   */
extern char            saveData[16];           /* �ꎞ�ۑ��G���A               */

/*======================================*/
/* �v���g�^�C�v�錾                     */
/*======================================*/
void i2c_wait( void );
void i2c_start( void );
void i2c_stop( void );
int i2c_write( char data );
char i2c_read( int ack );
void initI2CEeprom( unsigned char* ddrport, unsigned char* drport, unsigned char ddrdata, unsigned char scl, unsigned char sda );
void selectI2CEepromAddress( unsigned char address );
char readI2CEeprom( unsigned long address );
void writeI2CEeprom( unsigned long address, char write );
void pageWriteI2CEeprom( unsigned long address, int count, char* data );
void setPageWriteI2CEeprom( unsigned long address, int count, char* data );
void I2CEepromProcess( void );
void clearI2CEeprom( void );
int checkI2CEeprom( void );
