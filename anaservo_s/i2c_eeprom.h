/*======================================*/
/* 定数設定                             */
/*======================================*/
/* データ保存関連 */
extern int             saveIndex;              /* 保存インデックス             */
extern int             saveSendIndex;          /* 送信インデックス             */
extern int             saveFlag;               /* 保存フラグ                   */
extern char            saveData[16];           /* 一時保存エリア               */

/*======================================*/
/* プロトタイプ宣言                     */
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
