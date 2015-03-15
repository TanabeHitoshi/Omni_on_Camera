/*======================================*/
/* �C���N���[�h                         */
/*======================================*/
#include <stdio.h>
#include "h8_3048.h"                  /* R8C/38A SFR�̒�`�t�@�C��    */
#include <stdio.h>                 /* printf�g�p���C�u����         */
#include "isCamera.h"
#include "drive.h"

/* TAOS TSL1401CL */
#define		TAOS_SI_HIGH	P8DR |= 0x02	/* Port P8_1 bit */
#define		TAOS_SI_LOW		P8DR &= 0xfd	/* Port P8_1 bit */
#define		TAOS_CLK_HIGH	P8DR |= 0x01	/* Port P8_0 bit */
#define		TAOS_CLK_LOW	P8DR &= 0xfe	/* Port P8_0 bit */
#define		TAOS_EXPOSURE_DELAY	expose()

#define White_min	9					/* ���F�̍ŏ��l 			*/
#define	White_Max	150					/* ���C�����FMAX�l�̐ݒ� */

int 			ImageData[128];			/* �J�����̒l				*/
int				BinarizationData[128];	/* �Q�l��					*/
int   			EXPOSURE_timer = 0;		/* �I������					*/
int 			LineStart,LineStop;		/* �ǂݎ��ʒu�̎n�߂ƏI��� */
int				Max,Min,Ave;			/*�J�����ǂݎ��ő�l�A�ŏ��l�A���ϒl*/
unsigned int 	Rsensor01;				/* ���C���̉E�[ */
unsigned int 	Lsensor01;				/* ���C���̍��[ */
unsigned int 	Wide01;					/* ���C���̕� */
unsigned int 	Rsensor02;				/* ���C���̉E�[ */
unsigned int 	Lsensor02;				/* ���C���̍��[ */
unsigned int 	Wide02;					/* ���C���̕� */
unsigned int 	Rsensor;				/* ���C���̉E�[ */
unsigned int 	Lsensor;				/* ���C���̍��[ */
//unsigned int 	Wide;					/* ���C���̕� */
int				R_maker;				/* ���}�[�J�[ */
int				L_maker;				/* �E�}�[�J�[ */
int			 	Center;					/* ���C���̏d�S */
int			 	Center01;				/* ���C���̏d�S */
int			 	Center02;				/* ���C���̏d�S */
int 			White;					/* ���F�̌�	*/
int				pid_turn;

/************************************************************************/
/* �Z���T�[�ǂݎ��                                             */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Ȃ�			                                                */
/************************************************************************/
void sensor_process(void) {
	
	int i;
	int s;
	
	ImageCapture();
	expose();
	ImageCapture();				/* �C���[�W�L���v�`�� 	*/
	binarization();				/* �Q�l��				*/
	WhiteLineWide();			/* �����̕��𑪒�		*/
	WhiteLineCenter();			/* �����̒��S�𑪒�		*/

//	printf("Exposure = %d Max = %d Min = %d Ave = %d Wide = %d Center = %d\n",EXPOSURE_timer,Max,Min,Ave,WhiteLineWide(),WhiteLineCenter());
	pid_turn = PID();
}
/************************************************************************/
/* �J�����̏�����                                                */
/* �����@ �J�n�ʒu�A�I���ʒu                                                          */
/* �߂�l �Ȃ�                                                 */
/************************************************************************/
void initCamera(int Start,int Stop)
{
	LineStart = Start;
	LineStop = Stop;
}
/************************************************************************/
/* �C���[�W�L���v�`��                                                 */
/* �����@ �J�n�ʒu�A�I���ʒu                                                          */
/* �߂�l �Ȃ�                                                 */
/************************************************************************/
void ImageCapture(void){	 
	
	unsigned char i;

	Max = 0,Min = 1024;

	TAOS_SI_HIGH;  
	TAOS_CLK_HIGH;  
	TAOS_SI_LOW;
	ImageData[0] = 0;//get_ad7();	// inputs data from camera (first pixel)
	TAOS_CLK_LOW;

	for(i = 1; i < LineStart; i++) {		
		TAOS_CLK_HIGH;		
		TAOS_CLK_LOW;
	}
	for(i = LineStart; i < LineStop; i++) {					 
		TAOS_CLK_HIGH;
		ImageData[i] = get_ad2();	// inputs data from camera (one pixel each time through loop) 
		TAOS_CLK_LOW;
		
		if(Max < ImageData[i]){
			Max = ImageData[i];
		}			
		if(Min > ImageData[i]){
			Min = ImageData[i];
		}	
	}
	for(i = LineStop; i < 128; i++) {		
		TAOS_CLK_HIGH;		
		TAOS_CLK_LOW;
	}

	TAOS_CLK_HIGH;
	TAOS_CLK_LOW;
}
/************************************************************************/
/* �Q�l��                                                               */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Ȃ�			                                                */
/************************************************************************/
void binarization(void)
{
	int i;

	/* �ō��l�ƍŒ�l����Ԃ̒l�����߂� */
	Ave = (Max + Min) * 2 / 3;
	/* ���͂O�@���͂P�ɂ��� */
	White = 0;					/* ���̐����O�ɂ��� */
	if( Max > 50 ){
		/* �����꒼���̂Ƃ� */
		if(Min > 80){
			White = 128;
			for(i=1; i < 128; i++){
				BinarizationData[i] = 1;
			}
		}else{		
			for(i = LineStart ; i < LineStop; i++) {
				if(  ImageData[i] > Ave ){	
					White++;			
					BinarizationData[i] = 1;
				}else{
					BinarizationData[i] = 0;
				}
			}
		}
	/* ������ʂ̂Ƃ� */
	}else{
		for(i=1; i < 128; i++){
			BinarizationData[i] = 0;
		}
	}

}
/************************************************************************/
/* �����̕��𑪒�                                                       */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Ȃ�			                                                 */
/************************************************************************/
void WhiteLineWide(void)
{
	int t=0,i;
		
	Lsensor01 = Lsensor02 = LineStart;
	Rsensor01 = Rsensor02 = LineStop;
		
	for(i = Lsensor01 ; i < LineStop; i++) {
		if(t==0){	/* �P�{�ڂ̔��� */
			if( BinarizationData[i] ){		/* ������ŏ��̔� */
				Lsensor01 = i;
				t = 1;
			}
		}else if(t==1){
			if( BinarizationData[i] ){		/* ������ŏ��̔� */
				Lsensor01 = i;
				t = 2;
			}
		}else if(t==2){	/* �P�{�ڂ̍��� */
			if( !BinarizationData[i] ){		/* ������ŏ��̍� */			
				Rsensor01 = i;
				t = 3;
			}
		}else if(t==3){
			if( !BinarizationData[i] ){		/* ������ŏ��̍� */			
				Rsensor01 = i;
				t = 4;
			}
		}else if(t==4){	/* �Q�{�ڂ̔��� */
			if( BinarizationData[i] ){		/* ������ŏ��̔� */
				Lsensor02 = i;
				t = 5;
			}
		}else if(t==5){
			if( BinarizationData[i] ){		/* ������ŏ��̔� */
				Lsensor02 = i;
				t = 6;
			}
		}else if(t==6){	/* �Q�{�ڂ̍��� */
			if( !BinarizationData[i] ){		/* ������ŏ��̍� */			
				Rsensor02 = i;
				t = 7;
			}
		}else if(t==7){	
			if( !BinarizationData[i] ){		/* ������ŏ��̍� */			
				Rsensor02 = i;
				break;
			}
		}
	}
	if(White > White_min){
		Wide01 = Rsensor01 - Lsensor01;		/* 1�{�ڂ̔����������߂� */	
		if( Lsensor02 != LineStart){
			Wide02 = Rsensor02 - Lsensor02; /* 2�{�ڂ̔����������߂� */
		}else{
			Wide02 = 0;						/* 2�{�ڔ����Ȃ�		*/
		}
	}else{
		Wide01 = Wide02 = 0;				/* ����� */
	}			
}
/************************************************************************/
/* �����̒��S�𑪒�                                                       */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Ȃ�			                                                 */
/************************************************************************/
void WhiteLineCenter(void)
{
	int abs_Center01,abs_Center02;		/* �Z���^�[�l�̐�Βl */

	if(White > White_min){
		if(Wide01 != 0){
			Center01 = (Lsensor01 + Rsensor01)/2 - 64;	/* �d�S�����߂� */	
		}
		if(Wide02 != 0){
			Center02 = (Lsensor02 + Rsensor02)/2 - 64;	/* �d�S�����߂� */
		}			
	}else{
		Center = 0;								/* ����� */
	}
	 
	/* �}�[�J�[�̔��� */
	if( Wide01 != 0 ){
		if( Wide02 == 0){			/* �������P�{�̂Ƃ� */
			Center = Center01;
			R_maker = 0; L_maker = 0;
		}else if( Wide01 > 20){		/* Wide01���}�[�J�[�@Wide02������ */
			Center = Center02;
			L_maker = 1; R_maker = 0; 
		}else if( Wide02 > 20){		/* Wide01�������@Wide02���}�[�J�[ */
			Center = Center01;
			L_maker = 0; R_maker = 1; 
		}else{					
			/* �Z���^�[�l�̐�Βl */
			if(Center01 > 0) abs_Center01 = Center01;
			else			 abs_Center01 = -Center01;
			if(Center02 > 0) abs_Center02 = Center02;
			else			 abs_Center02 = -Center02;
			if( abs_Center01 > abs_Center02){/* Wide01���}�[�J�[�@Wide02������ */
				Center = Center02;
				L_maker = 1; R_maker = 0; 
			}else if( abs_Center01 < abs_Center02){/* Wide01�������@Wide02���}�[�J�[ */
				Center = Center01;
				L_maker = 0; R_maker = 1; 
			}
		}
	}	
}
/************************************************************************/
/* �L�����u���[�V����                                                   */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Ȃ�			                                                */
/************************************************************************/
void Calibration(void)
{
	int i;
	printf("Calibration now......\n");
	while(Max < White_Max){
		ImageCapture();
		for(i=0; i < EXPOSURE_timer ;i++);
		ImageCapture();
		EXPOSURE_timer += 50;
	}
	printf("EXPOSURE_timer = %d\n",EXPOSURE_timer);
}
/************************************************************************/
/* �I�����Ԓ���                                                         */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Ȃ��@�@�@�@�@                                                */
/************************************************************************/
void expose( void )
{
	int i;
	
//	if( Wide != 0 && !(White >= 90)){//���łȂ����ł��Ȃ�
//printf("White_Max = %d Max = %d\n",White_Max,Max);
//		if(Max < White_Max){
//			EXPOSURE_timer += (White_Max - Max)*10;
//			EXPOSURE_timer += 10;
//		}else{
//			EXPOSURE_timer -= (Max - White_Max)*10;
//			EXPOSURE_timer -= 10;
//		}
//		EXPOSURE_timer += (White_Max - Max);
//	}
//	if( EXPOSURE_timer > 1000) EXPOSURE_timer = 1000;
//	if( EXPOSURE_timer < 0 ) EXPOSURE_timer = 0;
	
	for(i=0;i<EXPOSURE_timer;i++);

}
/************************************************************************/
/* RAW�f�[�^�̕\��	                                                   */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Ȃ�			                                                 */
/************************************************************************/
void raw_view(void)
{
	int i;
	for(i = LineStart;i < LineStop; i++){
		printf("%d ",ImageData[i]);
	}
	printf("\n\n");
}
/************************************************************************/
/* �Q�l���f�[�^�̕\��	                                                   */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Ȃ�			                                                 */
/************************************************************************/
void bi_view(void)
{
	int i;
	for(i = LineStart;i < LineStop; i++){
		if(BinarizationData[i] == 1){
			printf("*");
		}else{
			printf(" ");
		}
	}
	printf("Exposure = %d Max = %d Min = %d Ave = %d Wide = %d Center = %d\n",EXPOSURE_timer,Max,Min,Ave,Wide01,Center);
	printf("Lsensor01 = %d Rsensor01 = %d Wide01 = %d Lsensor02 = %d Rsensor02 = %d Wide02 = %d L_maker = %d R_maker = %d\n",
			Lsensor01,Rsensor01,Wide01,Lsensor02,Rsensor02,Wide02,L_maker,R_maker);
}

/************************************************************************/
/* PID				                                                    */
/* �����@ �Ȃ�                                                          */
/* �߂�l �Ȃ�			                                                */
/************************************************************************/
int PID(void)
{
	static float	iCenter = 0.0;
	static float	preCenter = 0.0;
	float			h;
//	Center /= 2;
	iCenter +=  (float)Center - preCenter;
	h = (float)Center * Kp + iCenter * Ki + ((float)Center - preCenter) * Kd;
	preCenter = (float)Center;
	
	return h;
}

/************************************************************************/
/* A/D�l�ǂݍ���(AN2)                                                   */
/* �����@ �Ȃ�                                                          */
/* �߂�l A/D�l 0�`1023                                                 */
/************************************************************************/
int get_ad2( void )
{
    int i;

    /* A/D�R���o�[�^�̐ݒ� */
	i = i;						/* ��AD��1�T�C�N���E�G�C�g�����*/
	AD_CSR |= 0x20;				/* AD �X�^�[�g */
	while( !(AD_CSR & 0x80) );	/* �G���h�t���O���`�F�b�N */
	AD_CSR &= 0x7f;				/* �G���h�t���O�N���A */
	i = AD_DRC >> 6;			/* ��� */
	return i;
}
