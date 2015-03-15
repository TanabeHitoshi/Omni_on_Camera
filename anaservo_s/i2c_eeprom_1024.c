/****************************************************************************/
/* ファイル名   i2c_eeprom_1024.c                                           */
/* ﾌｧｲﾙ内容     I2C通信仕様の24C1024の制御 Cソース版 Ver3.01                */
/* Date         2007.02.19                                                  */
/* Copyright    ジャパンマイコンカーラリー実行委員会                        */
/****************************************************************************/

/*
使い方

●initI2CEeprom関数

EEP-ROMの書き込みを許可します。最初に必ず実行します。
引　数：EEP-ROMの繋がっているDDRポートの指定(&を付ける)
        EEP-ROMの繋がっているDRポートの指定(&を付ける)
        DDRポートの入出力設定値
        EEP-ROMのSCL端子の繋がっているビット番号
        EEP-ROMのSDA端子の繋がっているビット番号
戻り値：なし

例）initI2CEeprom( &PADDR, &PADR, 0x5f, 7, 5 );
    EEP-ROMはポートAに繋がっている
    入出力設定は0x57
    SCL端子はbit7に接続、SDA端子はbit5に接続

●selectI2CEepromAddress関数

I2Cバスに接続されているどのEEP-ROMを使用するか選択します。
引　数：EEP-ROMのアドレス
戻り値：なし

例）selectI2CEepromAddress( 1 );
    1番を使用
    EEP-ROM A1="1"に接続されているEEP-ROMを選択するという
    ことになります。

●readI2CEeprom関数

EEP-ROMからデータを読み込みます。
引　数：unsigned long アドレス 0x00000-0x1ffff
戻り値：char データ

例) i = readI2CEeprom( 0x0005 );
    EEP-ROMの0x0005番地のデータを変数iに代入します

●writeI2CEeprom関数

EEP-ROMへデータを書き込みます。
10msは書き込み作業中のため、アクセスできません。
引　数：unsigned long   アドレス 0x00000-0x1ffff ,
        char            データ
戻り値：なし

例）writeI2CEeprom( 0x2000, -100 );
    EEP-ROMの0x2000番地に-100を書き込みます

●pageWriteI2CEeprom関数

EEP-ROMへ複数バイトのデータを書き込みます。
10msは書き込み作業中のため、アクセスできません。
データの個数は２５６バイトまで指定できます。
※重要：アドレスは、
引　数：unsigned long   EEP-ROMに書き込むアドレス0x00000-0x1ffff ,
        int             個数1-256 ,
        char*           データがあるアドレス
戻り値：なし

例）char d[ 4 ];
    d[0]=5; d[1]=4; d[2]=1; d[3]=10;
    pageWriteI2CEeprom( 0x1000, 4, d );
    EEP-ROMの0x1000番地から、配列ｄにあるデータを４バイト分書き込みます

●clearI2CEeprom

EEP-ROMのデータをオールクリアします。
引　数：なし
戻り値：なし

例）clearI2CEeprom();

●checkI2CEeprom関数

書き込み後、書き込み作業が終わったかどうかチェックします。
引　数：なし
戻り値：1:次読み書きOK  0:まだ

例）while( !checkI2CEeprom() );
    書き込みが終了したかチェックします。

●setPageWriteI2CEeprom関数

EEP-ROMへ複数バイトのデータを書き込みます。書き込み準備を行うだけで
すぐに終了します。実際の書き込みはI2CEepromProcess関数で行います。
データは256バイトまで指定できます。
引　数：unsigned long   EEP-ROMに書き込むアドレス0x00000-0x1ffff ,
        int             個数1-256 ,
        char*           データがあるアドレス
戻り値：なし

例）char d[ 4 ];
    d[0]=5; d[1]=4; d[2]=1; d[3]=10;
    setPageWriteI2CEeprom( 0x1000, 4, d );  書き込み準備のみ

    while( 1 ) {
        I2CEepromProcess();                 実際の書き込みはこの関数
    }

●I2CEepromProcess関数

setPageWriteI2CEeprom関数で書き込み準備を行います。
この関数は実際に、EEP-ROMに書き込み作業を行います。
ただし、少しずつ書き込み作業を行って、書き込みだけで処理を占有しないように
しています。この関数は、書き込みデータ数＋５回以上実行してください。
通常は処理のループ内に入れておきます。書き込み作業がないときは何もしません
ので、入れておくだけでＯＫです。
引　数：なし
戻り値：なし

例）main {

        init();

        while( 1 ) {
            I2CEepromProcess();             常に実行するようにする

            その他の処理
        }
    }

*/

/*======================================*/
/* インクルード                         */
/*======================================*/
#include    <machine.h>
#include    "i2c_eeprom.h"

/*======================================*/
/* 定数設定                             */
/*======================================*/

/*
setPageWriteI2CEeprom関数１回で保存出来るサイズ
*/
#define SAVE_SIZE       16              /* 保存数                   */

/* ウエイトが必要な場合は1、なしの場合は0とする */
#if 0
#define I2C_WAIT()      i2c_wait();
#else
#define I2C_WAIT()      ;
#endif

/* マクロで関数定義 */
#define i2c_SDA_0();    *dr_port &= ~i2c_sda;\
                        *ddr_port = (ddr_data |= i2c_sda);   /* SDA="0" */
#define i2c_SDA_1();    *ddr_port = (ddr_data &= ~i2c_sda);  /* SDA="1" */
#define i2c_SCL_0();    *dr_port &= ~i2c_scl;\
                        *ddr_port = (ddr_data |= i2c_scl);   /* SCL="0" */
#define i2c_SCL_1();    *ddr_port = (ddr_data &= ~i2c_scl);  /* SCL="1" */

/* その他定義 */
#define ACK             0               /* リード時のACK有効(ACK=0) */
#define NO_ACK          1               /* リード時のACK無効(ACK=1) */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*  プログラムエリア                                                    */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static unsigned char*   ddr_port;           /* EEP-ROMのあるDDRポート   */
static unsigned char*   dr_port;            /* EEP-ROMのあるDRポート    */
static unsigned char    ddr_data;           /* 入出力設定値             */
static unsigned char    i2c_scl;            /* SCLビット                */
static unsigned char    i2c_sda;            /* SDAビット                */

static unsigned char    eep_address;        /* アドレスの選択           */
static int              write_mode;         /* 処理内容                 */
static unsigned char    write_eep_address;  /* 書き込みEEP-ROMのアドレス*/
static unsigned int     write_address;      /* 書き込みアドレス         */
static int              write_count;        /* 書き込み個数             */
static char             write_buff[SAVE_SIZE];/* 書き込みデータ保存バッファ*/
static char             *write_buff_p;      /* 書き込みデータ読み込み位置*/

/************************************************************************/
/* モジュール名 i2c_wait                                                */
/* 処理概要     EEP-ROMタイミング用ウエイト                             */
/************************************************************************/
void i2c_wait( void )
{
    int     i=1;    /* 速すぎる場合この数字を調整する */

    while( i-- );
}

/************************************************************************/
/* モジュール名 i2c_start                                               */
/* 処理概要     スタート信号送信                                        */
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
/* モジュール名 i2c_stop                                                */
/* 処理概要     ストップ信号送信                                        */
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
/* モジュール名 i2c_write                                               */
/* 処理概要     EEP-ROM　１バイト書き込み                               */
/* 引数         char            データ                                  */
/* 戻り値       int             acknowledge 0:有効  1:無効              */
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

        /* 次の準備 */
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
/* モジュール名 i2c_read                                                */
/* 処理概要     EEP-ROM　１バイト読み込み                               */
/* 引数         int ACK：データが続く NO_ACK：データ終了                */
/* 戻り値       char データ                                             */
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
/* モジュール名 initI2CEeprom                                           */
/* 処理概要     EEP-ROMのポート初期化                                   */
/* 引数         EEP-ROMの繋がっているDDRポートの指定(&を付ける)         */
/*              EEP-ROMの繋がっているDRポートの指定(&を付ける)          */
/*              DDRポートの入出力設定値                                 */
/*              EEP-ROMのSCL端子の繋がっているビット番号                */
/*              EEP-ROMのSDA端子の繋がっているビット番号なし            */
/* 戻り値       なし                                                    */
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
    selectI2CEepromAddress( 0 );        /* アドレスの選択           */
    write_mode  = 0;
}

/************************************************************************/
/* モジュール名 selectI2CEepromAddress                                  */
/* 処理概要     どの番号のEEP-ROMを使用するか選択                       */
/* 引数         unsigned char   番号 0-1                                */
/* 戻り値       なし                                                    */
/************************************************************************/
void selectI2CEepromAddress( unsigned char address )
{
    if( address ) {
        eep_address = 0x04;
    } else {
        eep_address = 0x00;
    }
}

/************************************************************************/
/* モジュール名 readI2CEeprom                                           */
/* 処理概要     EEP-ROM読み込み                                         */
/* 引数         unsigned long   アドレス 0x00000 - 0x1ffff              */
/* 戻り値       char            データ                                  */
/************************************************************************/
char readI2CEeprom( unsigned long address )
{
    char    ret;

    or_ccr( 0x80 );

    eep_address &= 0xfd;
    eep_address |= address >> 15 & 0x02;

    i2c_start();
    i2c_write( 0xa0 | eep_address);     /* device address(write)    */
    i2c_write( (address >> 8) & 0xff ); /* first address            */
    i2c_write( address & 0xff );        /* second address           */
    i2c_start();
    i2c_write( 0xa1 | eep_address );    /* device address(read)     */
    ret = i2c_read( NO_ACK );           /* data read                */
    i2c_stop();

    and_ccr( 0x7f );

    return ret;
}

/************************************************************************/
/* モジュール名 writeI2CEeprom                                          */
/* 処理概要     EEP-ROM書き込み                                         */
/* 引数         unsigned long アドレス 0x00000-0x1ffff ,char データ     */
/* 戻り値       なし                                                    */
/************************************************************************/
void writeI2CEeprom( unsigned long address, char write )
{
    or_ccr( 0x80 );

    eep_address &= 0xfd;
    eep_address |= address >> 15 & 0x02;

    i2c_start();
    i2c_write( 0xa0 | eep_address );    /* device address(write)    */
    i2c_write( (address >> 8) & 0xff ); /* first address            */
    i2c_write( address & 0xff );        /* second address           */
    i2c_write( write );                 /* data write               */
    i2c_stop();

    and_ccr( 0x7f );
}

/************************************************************************/
/* モジュール名 pageWriteI2CEeprom                                      */
/* 処理概要     EEP-ROMページ書き込み                                   */
/* 引数         unsigned long アドレス 0x00000-0x1fffff ,               */
/*              int 個数(1-64) , char* データがあるアドレス             */
/* 戻り値       なし                                                    */
/************************************************************************/
void pageWriteI2CEeprom( unsigned long address, int count, char* data )
{
    or_ccr( 0x80 );

    eep_address &= 0xfd;
    eep_address |= address >> 15 & 0x02;

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
/* モジュール名 setPageWriteI2CEeprom                                   */
/* 処理概要     EEP-ROMページ書き込み                                   */
/*              この関数でページ書き込みを開始する                      */
/* 引数         unsigned long アドレス 0x00000-0x1fffff ,               */
/*              int 個数(1-64) , char* データがあるアドレス             */
/* 戻り値       なし                                                    */
/************************************************************************/
void setPageWriteI2CEeprom( unsigned long address, int count, char* data )
{
    /* 書き込み中ならこの関数は中止 */
    if( write_mode != 0 ) return;

    eep_address &= 0xfd;
    eep_address |= address >> 15 & 0x02;

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
/* モジュール名 I2CEepromProcess                                        */
/* 処理概要     EEP-ROMページ書き込み                                   */
/* 引数         なし                                                    */
/* 戻り値       なし                                                    */
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
/* モジュール名 clearI2CEeprom                                          */
/* 処理概要     EEP-ROM　オールクリア                                   */
/* 引数         なし                                                    */
/* 戻り値       なし                                                    */
/************************************************************************/
void clearI2CEeprom( void )
{
    unsigned long   address;
    int             i;

    or_ccr( 0x80 );

    /* 0x00000-0x0ffff削除 */
    eep_address &= 0xfd;
    address = 0;
    while( 1 ) {
        i2c_start();
        i2c_write( 0xa0 | eep_address );    /* device address(write)    */
        i2c_write( address >> 8 );          /* first address            */
        i2c_write( address & 0xff );        /* second address           */
        for( i=0; i<256; i++ ) {
            i2c_write( 0 );                 /* data write               */
        }
        i2c_stop();

        while( !checkI2CEeprom() );         /* wait                     */

        address += 256;
        if( address >= 0x10000 ) break;
    }

    /* 0x10000-0x1ffff削除 */
    eep_address |= 0x02;
    address = 0;
    while( 1 ) {
        i2c_start();
        i2c_write( 0xa0 | eep_address );    /* device address(write)    */
        i2c_write( address >> 8 );          /* first address            */
        i2c_write( address & 0xff );        /* second address           */
        for( i=0; i<256; i++ ) {
            i2c_write( 0 );                 /* data write               */
        }
        i2c_stop();

        while( !checkI2CEeprom() );         /* wait                     */

        address += 256;
        if( address >= 0x10000 ) break;
    }

    and_ccr( 0x7f );
}

/************************************************************************/
/* モジュール名 checkI2CEeprom                                          */
/* 処理概要     EEP-ROM書き込み後、次に読み書きできるかチェック         */
/* 引数         なし                                                    */
/* 戻り値       1:次読み書きOK 0:まだ                                   */
/************************************************************************/
int checkI2CEeprom( void )
{
    int ret;

    or_ccr( 0x80 );

    i2c_start();
    ret = !i2c_write( 0xa0 | eep_address );
    i2c_stop();

    and_ccr( 0x7f );

    return ret;
}

/************************************************************************/
/* end of file                                                          */
/************************************************************************/