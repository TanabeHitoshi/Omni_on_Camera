#ifndef	INI_H
#define	INI_H

extern unsigned long   cnt0;                   /* timer�֐��p                  */
extern unsigned long   cnt1;                   /* main���Ŏg�p                 */
extern unsigned long   cnt_lcd;                /* LCD�����Ŏg�p                */
extern unsigned long   stop_timer;				/* ���s�^�C�}�[					*/

extern int             pattern;                /* �p�^�[���ԍ�                 */
extern int             iTimer10;               /* 10ms�J�E���g�p               */
extern int			Cycle;					/* ����				*/

void init( void );
void timer( unsigned long timer_set );

#endif