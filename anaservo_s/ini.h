#ifndef	INI_H
#define	INI_H

extern unsigned long   cnt0;                   /* timer関数用                  */
extern unsigned long   cnt1;                   /* main内で使用                 */
extern unsigned long   cnt_lcd;                /* LCD処理で使用                */
extern unsigned long   stop_timer;				/* 走行タイマー					*/

extern int             pattern;                /* パターン番号                 */
extern int             iTimer10;               /* 10msカウント用               */
extern int			Cycle;					/* 周期				*/

void init( void );
void timer( unsigned long timer_set );

#endif