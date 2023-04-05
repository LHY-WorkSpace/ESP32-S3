#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_demos.h"
#include "lv_port_disp.h"
#include "lvgl.h"
#include "Timer.h"
#include "driver/gpio.h"

//旋转半径
#define R_LEN (42)

//眼球直径
#define EYE_SIZE    		(60)

//瞳孔直径
#define EYE_HOLE_SIZE		(35)
//瞳孔最小直径(动画)
#define EYE_HOLE_MIN_SIZE		(25)

//高光直径
#define EYE_HIGHLIGHT_SIZE			(15)

//高光移动角度倍数(越大移动越远)
#define EYE_HIGHLIGHT_ANGLE_G		(8)

//上部高光初始位置
#define EYE_HIGHLIGHT_HIGH_X	( 15)
#define EYE_HIGHLIGHT_HIGH_Y	(-15)

//下部高光初始位置
#define EYE_HIGHLIGHT_LOW_X		(-18)
#define EYE_HIGHLIGHT_LOW_Y		( 18)


#define ROTATEDIR_FORWARD     ( 1)//顺时针
#define ROTATEDIR_OPPOSITE    (-1)//逆时针

#define RADTODEG(x) ((x) * 57.295779513082320876798154814105f)
#define DEGTORAD(x) ((x) * 0.01745329251994329576923690768489f)

lv_obj_t *Eye_Group[4];
lv_obj_t *Eye_base[4];//瞳孔
lv_obj_t *Eye_in_high[4];
lv_obj_t *Eye_in_low[4];
lv_anim_t EyeBodyPath_Anim[4];
lv_anim_t EyeFocalize_Anim[4];
lv_anim_t EyeBaseMove_Anim[4];


#define FAST_SIN_TABLE_SIZE 512

const float sinTable[FAST_SIN_TABLE_SIZE + 1] = 
{
	0.00000000f, 0.01227154f, 0.02454123f, 0.03680722f, 0.04906767f, 0.06132074f,
	0.07356456f, 0.08579731f, 0.09801714f, 0.11022221f, 0.12241068f, 0.13458071f,
	0.14673047f, 0.15885814f, 0.17096189f, 0.18303989f, 0.19509032f, 0.20711138f,
	0.21910124f, 0.23105811f, 0.24298018f, 0.25486566f, 0.26671276f, 0.27851969f,
	0.29028468f, 0.30200595f, 0.31368174f, 0.32531029f, 0.33688985f, 0.34841868f,
	0.35989504f, 0.37131719f, 0.38268343f, 0.39399204f, 0.40524131f, 0.41642956f,
	0.42755509f, 0.43861624f, 0.44961133f, 0.46053871f, 0.47139674f, 0.48218377f,
	0.49289819f, 0.50353838f, 0.51410274f, 0.52458968f, 0.53499762f, 0.54532499f,
	0.55557023f, 0.56573181f, 0.57580819f, 0.58579786f, 0.59569930f, 0.60551104f,
	0.61523159f, 0.62485949f, 0.63439328f, 0.64383154f, 0.65317284f, 0.66241578f,
	0.67155895f, 0.68060100f, 0.68954054f, 0.69837625f, 0.70710678f, 0.71573083f,
	0.72424708f, 0.73265427f, 0.74095113f, 0.74913639f, 0.75720885f, 0.76516727f,
	0.77301045f, 0.78073723f, 0.78834643f, 0.79583690f, 0.80320753f, 0.81045720f,
	0.81758481f, 0.82458930f, 0.83146961f, 0.83822471f, 0.84485357f, 0.85135519f,
	0.85772861f, 0.86397286f, 0.87008699f, 0.87607009f, 0.88192126f, 0.88763962f,
	0.89322430f, 0.89867447f, 0.90398929f, 0.90916798f, 0.91420976f, 0.91911385f,
	0.92387953f, 0.92850608f, 0.93299280f, 0.93733901f, 0.94154407f, 0.94560733f,
	0.94952818f, 0.95330604f, 0.95694034f, 0.96043052f, 0.96377607f, 0.96697647f,
	0.97003125f, 0.97293995f, 0.97570213f, 0.97831737f, 0.98078528f, 0.98310549f,
	0.98527764f, 0.98730142f, 0.98917651f, 0.99090264f, 0.99247953f, 0.99390697f,
	0.99518473f, 0.99631261f, 0.99729046f, 0.99811811f, 0.99879546f, 0.99932238f,
	0.99969882f, 0.99992470f, 1.00000000f, 0.99992470f, 0.99969882f, 0.99932238f,
	0.99879546f, 0.99811811f, 0.99729046f, 0.99631261f, 0.99518473f, 0.99390697f,
	0.99247953f, 0.99090264f, 0.98917651f, 0.98730142f, 0.98527764f, 0.98310549f,
	0.98078528f, 0.97831737f, 0.97570213f, 0.97293995f, 0.97003125f, 0.96697647f,
	0.96377607f, 0.96043052f, 0.95694034f, 0.95330604f, 0.94952818f, 0.94560733f,
	0.94154407f, 0.93733901f, 0.93299280f, 0.92850608f, 0.92387953f, 0.91911385f,
	0.91420976f, 0.90916798f, 0.90398929f, 0.89867447f, 0.89322430f, 0.88763962f,
	0.88192126f, 0.87607009f, 0.87008699f, 0.86397286f, 0.85772861f, 0.85135519f,
	0.84485357f, 0.83822471f, 0.83146961f, 0.82458930f, 0.81758481f, 0.81045720f,
	0.80320753f, 0.79583690f, 0.78834643f, 0.78073723f, 0.77301045f, 0.76516727f,
	0.75720885f, 0.74913639f, 0.74095113f, 0.73265427f, 0.72424708f, 0.71573083f,
	0.70710678f, 0.69837625f, 0.68954054f, 0.68060100f, 0.67155895f, 0.66241578f,
	0.65317284f, 0.64383154f, 0.63439328f, 0.62485949f, 0.61523159f, 0.60551104f,
	0.59569930f, 0.58579786f, 0.57580819f, 0.56573181f, 0.55557023f, 0.54532499f,
	0.53499762f, 0.52458968f, 0.51410274f, 0.50353838f, 0.49289819f, 0.48218377f,
	0.47139674f, 0.46053871f, 0.44961133f, 0.43861624f, 0.42755509f, 0.41642956f,
	0.40524131f, 0.39399204f, 0.38268343f, 0.37131719f, 0.35989504f, 0.34841868f,
	0.33688985f, 0.32531029f, 0.31368174f, 0.30200595f, 0.29028468f, 0.27851969f,
	0.26671276f, 0.25486566f, 0.24298018f, 0.23105811f, 0.21910124f, 0.20711138f,
	0.19509032f, 0.18303989f, 0.17096189f, 0.15885814f, 0.14673047f, 0.13458071f,
	0.12241068f, 0.11022221f, 0.09801714f, 0.08579731f, 0.07356456f, 0.06132074f,
	0.04906767f, 0.03680722f, 0.02454123f, 0.01227154f, 0.00000000f, -0.01227154f,
	-0.02454123f, -0.03680722f, -0.04906767f, -0.06132074f, -0.07356456f,
	-0.08579731f, -0.09801714f, -0.11022221f, -0.12241068f, -0.13458071f,
	-0.14673047f, -0.15885814f, -0.17096189f, -0.18303989f, -0.19509032f, 
	-0.20711138f, -0.21910124f, -0.23105811f, -0.24298018f, -0.25486566f, 
	-0.26671276f, -0.27851969f, -0.29028468f, -0.30200595f, -0.31368174f, 
	-0.32531029f, -0.33688985f, -0.34841868f, -0.35989504f, -0.37131719f, 
	-0.38268343f, -0.39399204f, -0.40524131f, -0.41642956f, -0.42755509f, 
	-0.43861624f, -0.44961133f, -0.46053871f, -0.47139674f, -0.48218377f, 
	-0.49289819f, -0.50353838f, -0.51410274f, -0.52458968f, -0.53499762f, 
	-0.54532499f, -0.55557023f, -0.56573181f, -0.57580819f, -0.58579786f, 
	-0.59569930f, -0.60551104f, -0.61523159f, -0.62485949f, -0.63439328f, 
	-0.64383154f, -0.65317284f, -0.66241578f, -0.67155895f, -0.68060100f, 
	-0.68954054f, -0.69837625f, -0.70710678f, -0.71573083f, -0.72424708f, 
	-0.73265427f, -0.74095113f, -0.74913639f, -0.75720885f, -0.76516727f, 
	-0.77301045f, -0.78073723f, -0.78834643f, -0.79583690f, -0.80320753f, 
	-0.81045720f, -0.81758481f, -0.82458930f, -0.83146961f, -0.83822471f, 
	-0.84485357f, -0.85135519f, -0.85772861f, -0.86397286f, -0.87008699f, 
	-0.87607009f, -0.88192126f, -0.88763962f, -0.89322430f, -0.89867447f, 
	-0.90398929f, -0.90916798f, -0.91420976f, -0.91911385f, -0.92387953f, 
	-0.92850608f, -0.93299280f, -0.93733901f, -0.94154407f, -0.94560733f, 
	-0.94952818f, -0.95330604f, -0.95694034f, -0.96043052f, -0.96377607f, 
	-0.96697647f, -0.97003125f, -0.97293995f, -0.97570213f, -0.97831737f, 
	-0.98078528f, -0.98310549f, -0.98527764f, -0.98730142f, -0.98917651f, 
	-0.99090264f, -0.99247953f, -0.99390697f, -0.99518473f, -0.99631261f, 
	-0.99729046f, -0.99811811f, -0.99879546f, -0.99932238f, -0.99969882f, 
	-0.99992470f, -1.00000000f, -0.99992470f, -0.99969882f, -0.99932238f, 
	-0.99879546f, -0.99811811f, -0.99729046f, -0.99631261f, -0.99518473f, 
	-0.99390697f, -0.99247953f, -0.99090264f, -0.98917651f, -0.98730142f, 
	-0.98527764f, -0.98310549f, -0.98078528f, -0.97831737f, -0.97570213f, 
	-0.97293995f, -0.97003125f, -0.96697647f, -0.96377607f, -0.96043052f, 
	-0.95694034f, -0.95330604f, -0.94952818f, -0.94560733f, -0.94154407f, 
	-0.93733901f, -0.93299280f, -0.92850608f, -0.92387953f, -0.91911385f, 
	-0.91420976f, -0.90916798f, -0.90398929f, -0.89867447f, -0.89322430f, 
	-0.88763962f, -0.88192126f, -0.87607009f, -0.87008699f, -0.86397286f, 
	-0.85772861f, -0.85135519f, -0.84485357f, -0.83822471f, -0.83146961f, 
	-0.82458930f, -0.81758481f, -0.81045720f, -0.80320753f, -0.79583690f, 
	-0.78834643f, -0.78073723f, -0.77301045f, -0.76516727f, -0.75720885f, 
	-0.74913639f, -0.74095113f, -0.73265427f, -0.72424708f, -0.71573083f, 
	-0.70710678f, -0.69837625f, -0.68954054f, -0.68060100f, -0.67155895f, 
	-0.66241578f, -0.65317284f, -0.64383154f, -0.63439328f, -0.62485949f, 
	-0.61523159f, -0.60551104f, -0.59569930f, -0.58579786f, -0.57580819f, 
	-0.56573181f, -0.55557023f, -0.54532499f, -0.53499762f, -0.52458968f, 
	-0.51410274f, -0.50353838f, -0.49289819f, -0.48218377f, -0.47139674f, 
	-0.46053871f, -0.44961133f, -0.43861624f, -0.42755509f, -0.41642956f, 
	-0.40524131f, -0.39399204f, -0.38268343f, -0.37131719f, -0.35989504f, 
	-0.34841868f, -0.33688985f, -0.32531029f, -0.31368174f, -0.30200595f, 
	-0.29028468f, -0.27851969f, -0.26671276f, -0.25486566f, -0.24298018f, 
	-0.23105811f, -0.21910124f, -0.20711138f, -0.19509032f, -0.18303989f, 
	-0.17096189f, -0.15885814f, -0.14673047f, -0.13458071f, -0.12241068f, 
	-0.11022221f, -0.09801714f, -0.08579731f, -0.07356456f, -0.06132074f, 
	-0.04906767f, -0.03680722f, -0.02454123f, -0.01227154f, -0.00000000f
};


float FastSin(float x)
{
	float sinVal, fract, in; // Temporary variables for input, output
	unsigned short index; // Index variable
	float a, b; // Two nearest output values
	int n;
	float findex;

	// input x is in radians
	// Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi
	in = x * 0.159154943092f;

	// Calculation of floor value of input
	n = (int) in;

	// Make negative values towards -infinity
	if(x < 0.0f){
		n--;
	}

	// Map input value to [0 1]
	in = in - (float) n;

	// Calculation of index of the table
	findex = (float) FAST_SIN_TABLE_SIZE * in;
	index = ((unsigned short)findex) & 0x1ff;

	// fractional value calculation
	fract = findex - (float) index;

	// Read two nearest values of input value from the sin table
	a = sinTable[index];
	b = sinTable[index+1];

	// Linear interpolation process
	sinVal = (1.0f-fract)*a + fract*b;

	// Return the output value
	return (sinVal);
}

float FastCos(float x)
{
	float cosVal, fract, in; // Temporary variables for input, output
	unsigned short index; // Index variable
	float a, b; // Two nearest output values
	int n;
	float findex;

	// input x is in radians
	// Scale the input to [0 1] range from [0 2*PI] , divide input by 2*pi, add 0.25 (pi/2) to read sine table
	in = x * 0.159154943092f + 0.25f;

	// Calculation of floor value of input
	n = (int) in;

	// Make negative values towards -infinity
	if(in < 0.0f){
		n--;
	}

	// Map input value to [0 1]
	in = in - (float) n;

	// Calculation of index of the table
	findex = (float) FAST_SIN_TABLE_SIZE * in;
	index = ((unsigned short)findex) & 0x1ff;

	// fractional value calculation
	fract = findex - (float) index;

	// Read two nearest values of input value from the cos table
	a = sinTable[index];
	b = sinTable[index+1];

	// Linear interpolation process
	cosVal = (1.0f-fract)*a + fract*b;

	// Return the output value
	return (cosVal);
}


uint8_t RotateDir = ROTATEDIR_OPPOSITE;

 //口字形
int8_t Eye_Position[4][3]=
{
    { R_LEN,  R_LEN, 0},
    {-R_LEN,  R_LEN, 1},
    {-R_LEN, -R_LEN, 2},
    { R_LEN, -R_LEN, 3},
};


lv_obj_t *BackGround;
lv_obj_t *Face;
lv_obj_t *Btn[3];


void Face_Create()
{
    BackGround = lv_obj_create(lv_scr_act());
    lv_obj_set_size(BackGround,240,240);
    lv_obj_set_style_bg_color(BackGround,lv_color_black(),LV_PART_MAIN);
    lv_obj_set_style_radius(BackGround,0,LV_PART_MAIN);
    lv_obj_set_style_border_side(BackGround,LV_BORDER_SIDE_FULL,LV_PART_MAIN);
    lv_obj_set_style_border_color(BackGround,lv_color_white(),LV_PART_MAIN);
    lv_obj_set_style_border_width(BackGround,0,LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(BackGround,LV_SCROLLBAR_MODE_OFF);




    Face = lv_obj_create(BackGround);
    lv_obj_set_size(Face,240-8*2,240-8*2);
    lv_obj_set_style_bg_color(Face,lv_color_black(),LV_PART_MAIN);
    lv_obj_set_style_radius(Face,(240-8*2)/2,LV_PART_MAIN);

    //lv_obj_set_style_outline_pad();
    lv_obj_set_style_outline_opa(Face,LV_OPA_90,LV_PART_MAIN);
    lv_obj_set_style_outline_width(Face,8,LV_PART_MAIN);
    lv_obj_set_style_outline_color(Face,lv_palette_main(LV_PALETTE_GREY),LV_PART_MAIN);//眼球外轮廓

    //lv_obj_set_style_border_post();
    lv_obj_set_style_border_opa(Face,LV_OPA_80,LV_PART_MAIN);
    lv_obj_set_style_border_side(Face,LV_BORDER_SIDE_FULL,LV_PART_MAIN);
    lv_obj_set_style_border_color(Face,lv_palette_darken(LV_PALETTE_GREY,3),LV_PART_MAIN);
    lv_obj_set_style_border_width(Face,15,LV_PART_MAIN);

    lv_obj_set_scrollbar_mode(Face,LV_SCROLLBAR_MODE_OFF);
    lv_obj_center(Face);


    // lv_scr_load(BackGround);
    // lv_scr_load(Face);

}






//调整轨迹动画回调
static void Eye_BodyAnimPath_CB(void *var, int32_t v)
{
    uint8_t i;
    int16_t x1,y1;

    lv_obj_t *Eye_tmp = (lv_obj_t *)var;

    for ( i = 0; i < 4; i++)
    {
        if(Eye_tmp == Eye_Group[i])
        {

			if( RotateDir == 1)
			{
				//顺时针
				x1=(Eye_Position[Eye_Position[i][2]][0])*FastCos(DEGTORAD(v))-(Eye_Position[Eye_Position[i][2]][1])*FastSin(DEGTORAD(v));
				y1=(Eye_Position[Eye_Position[i][2]][1])*FastCos(DEGTORAD(v))+(Eye_Position[Eye_Position[i][2]][0])*FastSin(DEGTORAD(v));  
			}
			else
			{
				//逆时针
				x1=(Eye_Position[Eye_Position[i][2]][0])*FastCos(DEGTORAD(v))+(Eye_Position[Eye_Position[i][2]][1])*FastSin(DEGTORAD(v));
				y1=(Eye_Position[Eye_Position[i][2]][1])*FastCos(DEGTORAD(v))-(Eye_Position[Eye_Position[i][2]][0])*FastSin(DEGTORAD(v));  
			}


			lv_obj_align_to(Eye_Group[i],Face,LV_ALIGN_CENTER,x1,y1);

			if( v == 90)
			{
				Eye_Position[i][2] = (Eye_Position[i][2] +RotateDir)%4;

				lv_anim_set_delay(&EyeBodyPath_Anim[i], 0);
				//旋转结束后调整眼部焦距
				lv_anim_set_repeat_count(&EyeFocalize_Anim[i], 2);
				lv_anim_start(&EyeFocalize_Anim[i]);

			}
        }
    }
}

//调整焦距动画回调
static void ChangeEyeFocalize_CB(void *var, int32_t v)
{
	uint8_t i;
    int16_t x1,y1;

    lv_obj_t *Eye_tmp = (lv_obj_t *)var;

    for ( i = 0; i < 4; i++)
    {
        if(Eye_tmp == Eye_base[i])
        {
			lv_obj_set_size(Eye_tmp,v,v);
			lv_obj_set_style_radius(Eye_tmp,v/2,LV_PART_MAIN);
			lv_obj_align_to(Eye_tmp,Eye_Group[i],LV_ALIGN_CENTER,0,0);

			//逆时针
			x1=(EYE_HIGHLIGHT_HIGH_X)*FastCos(DEGTORAD((EYE_HOLE_SIZE-v)*EYE_HIGHLIGHT_ANGLE_G))+(EYE_HIGHLIGHT_HIGH_Y)*FastSin(DEGTORAD((EYE_HOLE_SIZE-v)*EYE_HIGHLIGHT_ANGLE_G));
			y1=(EYE_HIGHLIGHT_HIGH_Y)*FastCos(DEGTORAD((EYE_HOLE_SIZE-v)*EYE_HIGHLIGHT_ANGLE_G))-(EYE_HIGHLIGHT_HIGH_X)*FastSin(DEGTORAD((EYE_HOLE_SIZE-v)*EYE_HIGHLIGHT_ANGLE_G)); 
			lv_obj_align_to(Eye_in_high[i],Eye_Group[i],LV_ALIGN_CENTER,x1,y1);

			//逆时针
			x1=(EYE_HIGHLIGHT_LOW_X)*FastCos(DEGTORAD((EYE_HOLE_SIZE-v)*EYE_HIGHLIGHT_ANGLE_G))+(EYE_HIGHLIGHT_LOW_Y)*FastSin(DEGTORAD((EYE_HOLE_SIZE-v)*EYE_HIGHLIGHT_ANGLE_G));
			y1=(EYE_HIGHLIGHT_LOW_Y)*FastCos(DEGTORAD((EYE_HOLE_SIZE-v)*EYE_HIGHLIGHT_ANGLE_G))-(EYE_HIGHLIGHT_LOW_X)*FastSin(DEGTORAD((EYE_HOLE_SIZE-v)*EYE_HIGHLIGHT_ANGLE_G));  
			lv_obj_align_to(Eye_in_low[i],Eye_Group[i],LV_ALIGN_CENTER,x1,y1);

        }
    }
}


//创建眼球主体
void Eye_BodyCreate()
{
    uint8_t i=0;

    for ( i = 0; i < 4; i++)
    {
		//眼球 37,58,18   56,110,23   
		Eye_Group[i] = lv_obj_create(Face);
		lv_obj_set_size(Eye_Group[i],EYE_SIZE,EYE_SIZE);
		lv_obj_set_style_bg_color(Eye_Group[i],lv_color_make(56,110,23),LV_PART_MAIN);//瞳孔颜色
		lv_obj_set_style_radius(Eye_Group[i],EYE_SIZE/2,LV_PART_MAIN);
		lv_obj_set_style_outline_width(Eye_Group[i],3,LV_PART_MAIN);
		lv_obj_set_style_outline_color(Eye_Group[i],lv_color_make(84,104,58),LV_PART_MAIN);//瞳孔外轮廓颜色
		lv_obj_set_scrollbar_mode(Eye_Group[i],LV_SCROLLBAR_MODE_OFF);//关闭滚动条
        lv_obj_align_to(Eye_Group[i],Face,LV_ALIGN_CENTER,Eye_Position[i][0],Eye_Position[i][1]);
		lv_obj_clear_flag(Eye_Group[i],LV_OBJ_FLAG_CLICKABLE );//不可拖动


		//瞳孔
		Eye_base[i] = lv_obj_create(Eye_Group[i]);
		lv_obj_set_size(Eye_base[i],EYE_HOLE_SIZE,EYE_HOLE_SIZE);
		lv_obj_set_style_radius(Eye_base[i],EYE_HOLE_SIZE/2,LV_PART_MAIN);
		lv_obj_align_to(Eye_base[i],Eye_Group[i],LV_ALIGN_CENTER,0,0);
		lv_obj_set_style_bg_color(Eye_base[i],lv_color_black(),LV_PART_MAIN);
		lv_obj_set_style_outline_width(Eye_base[i],5,LV_PART_MAIN);
		lv_obj_set_style_outline_color(Eye_base[i],lv_color_black(),LV_PART_MAIN);
		lv_obj_set_scrollbar_mode(Eye_base[i],LV_SCROLLBAR_MODE_OFF);
		lv_obj_clear_flag(Eye_base[i],LV_OBJ_FLAG_CLICKABLE );

		//眼部高光(上部)
		Eye_in_high[i] = lv_obj_create(Eye_Group[i]);
		lv_obj_set_size(Eye_in_high[i],EYE_HIGHLIGHT_SIZE,EYE_HIGHLIGHT_SIZE);
		lv_obj_set_style_radius(Eye_in_high[i],EYE_HIGHLIGHT_SIZE/2,LV_PART_MAIN);
		lv_obj_align_to(Eye_in_high[i],Eye_Group[i],LV_ALIGN_CENTER,EYE_HIGHLIGHT_HIGH_X,EYE_HIGHLIGHT_HIGH_Y);
		lv_obj_set_scrollbar_mode(Eye_in_high[i],LV_SCROLLBAR_MODE_OFF);
		lv_obj_clear_flag(Eye_in_high[i],LV_OBJ_FLAG_CLICKABLE );

		//眼部高光(下部)
		Eye_in_low[i] = lv_obj_create(Eye_Group[i]);
		lv_obj_set_size(Eye_in_low[i],EYE_HIGHLIGHT_SIZE/2,EYE_HIGHLIGHT_SIZE/2);
		lv_obj_set_style_radius(Eye_in_low[i],EYE_HIGHLIGHT_SIZE/4,LV_PART_MAIN);
		lv_obj_align_to(Eye_in_low[i],Eye_Group[i],LV_ALIGN_CENTER,EYE_HIGHLIGHT_LOW_X,EYE_HIGHLIGHT_LOW_Y);
		lv_obj_set_scrollbar_mode(Eye_in_low[i],LV_SCROLLBAR_MODE_OFF);
		lv_obj_clear_flag(Eye_in_low[i],LV_OBJ_FLAG_CLICKABLE );

    }
}


// lv_anim_set_path_cb(&Anima_Eye1,lv_anim_path_ease_in_out);
// lv_anim_set_path_cb(&Anima_Eye1,lv_anim_path_linear);
// lv_anim_set_path_cb(&Anima,lv_anim_path_ease_in);
// lv_anim_set_path_cb(&Anima,lv_anim_path_ease_out);

//创建眼球动画
void Eye_BodyAnimPath()
{	
	uint8_t i=0;

	for ( i = 0; i < 4; i++)
    {
		//创建眼球轨迹动画
        lv_anim_init(&EyeBodyPath_Anim[i]);
        lv_anim_set_var(&EyeBodyPath_Anim[i],Eye_Group[i]);
        lv_anim_set_values(&EyeBodyPath_Anim[i],0,90);
        lv_anim_set_time(&EyeBodyPath_Anim[i], 300);
		lv_anim_set_delay(&EyeBodyPath_Anim[i], 400);
        lv_anim_set_exec_cb(&EyeBodyPath_Anim[i], Eye_BodyAnimPath_CB);
        lv_anim_set_path_cb(&EyeBodyPath_Anim[i],lv_anim_path_ease_in_out);
        lv_anim_set_repeat_delay(&EyeBodyPath_Anim[i],200);
        // lv_anim_set_repeat_count(&EyeBodyPath_Anim[i], LV_ANIM_REPEAT_INFINITE);
		lv_anim_set_repeat_count(&EyeBodyPath_Anim[i], 3);
    }
	
    for ( i = 0; i < 4; i++)
    {
        lv_anim_start(&EyeBodyPath_Anim[i]);
    }
}


//创建瞳孔焦距动画
void EyeFocalizeAnimCreat()
{
	uint8_t i=0;

	for (i = 0; i < 4; i++)
	{
		lv_anim_init(&EyeFocalize_Anim[i]);
        lv_anim_set_var(&EyeFocalize_Anim[i],Eye_base[i]);
        lv_anim_set_values(&EyeFocalize_Anim[i],EYE_HOLE_SIZE,EYE_HOLE_MIN_SIZE);
        lv_anim_set_time(&EyeFocalize_Anim[i], 200);
		lv_anim_set_delay(&EyeFocalize_Anim[i], 500);
        lv_anim_set_exec_cb(&EyeFocalize_Anim[i], ChangeEyeFocalize_CB);
        lv_anim_set_path_cb(&EyeFocalize_Anim[i],lv_anim_path_ease_in_out);
		lv_anim_set_playback_time(&EyeFocalize_Anim[i],200);
		lv_anim_set_playback_delay(&EyeFocalize_Anim[i],200);
        lv_anim_set_repeat_delay(&EyeFocalize_Anim[i],300);
        lv_anim_set_repeat_count(&EyeFocalize_Anim[i], LV_ANIM_REPEAT_INFINITE);

	}

}


//设置眼睛旋转方向和次数
void RotateEye(uint8_t Dir ,uint8_t times)
{
	uint8_t i=0;

    for ( i = 0; i < 4; i++)
    {
    	lv_anim_set_repeat_count(&EyeBodyPath_Anim[i], times);
	}

	RotateDir = Dir;

    for ( i = 0; i < 4; i++)
    {
        lv_anim_start(&EyeBodyPath_Anim[i]);
    }
}

//设置调整瞳孔焦距次数
void ChangeEyeFocalize(uint8_t times)
{
	uint8_t i=0;

    for ( i = 0; i < 4; i++)
    {
        lv_anim_set_repeat_count(&EyeFocalize_Anim[i], times);
    }

    for ( i = 0; i < 4; i++)
    {
        lv_anim_start(&EyeFocalize_Anim[i]);
    }

}

//设置眼底颜色
void SetEyeBgColor(lv_palette_t color)
{
	uint8_t i=0;

    for ( i = 0; i < 4; i++)
    {
		// lv_obj_set_style_bg_color(Eye_Group[i],lv_palette_darken(color,2),LV_PART_MAIN);
		lv_obj_set_style_bg_color(Eye_Group[i],lv_palette_main(color),LV_PART_MAIN);
    }
}


void Eye_Main()
{
	Eye_BodyCreate();
	Eye_BodyAnimPath();
	EyeFocalizeAnimCreat();

}






void lvgl_Task()
{
    TickType_t Time;

    Time=xTaskGetTickCount();
    while (1)
    {    lv_tick_inc(1);
		lv_task_handler();
        // vTaskDelayUntil(&Time,5/portTICK_PERIOD_MS);
    }
}

void LED_Task()
{
    TickType_t Time;	
    Time=xTaskGetTickCount();

    while (1)
    {
		gpio_set_level(1,0);
        vTaskDelayUntil(&Time,30/portTICK_PERIOD_MS);
		gpio_set_level(1,1);
        vTaskDelayUntil(&Time,30/portTICK_PERIOD_MS);
    }
} 







void app_main(void)
{
    printf("Hello world!\n");

	const   gpio_config_t test = 
	{
        .pin_bit_mask = ((1ULL << (uint8_t)(1))),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
   gpio_config(&test);
	Timer_Init();

    lv_init();
    lv_port_disp_init();
    // lv_demo_benchmark();

    Face_Create();
    Eye_Main();

	xTaskCreate( (TaskFunction_t)lvgl_Task,"LVGL",4500,NULL,11,NULL);
	// xTaskCreate( (TaskFunction_t)LED_Task,"led",2000,NULL,10,NULL);

    // while (1)
    // {
	// 	lv_task_handler();
	// 	// gpio_set_level(1,0);
    //     // vTaskDelayUntil(&Time,20/portTICK_PERIOD_MS);
	// 	// gpio_set_level(1,1);
    //     // vTaskDelayUntil(&Time,20/portTICK_PERIOD_MS);
    // }


    // while (1)
    // {
	// 	lv_task_handler();
	// 	vTaskDelayUntil(&Time,10/portTICK_PERIOD_MS);
    //     // vTaskDelay(10 / portTICK_PERIOD_MS);

	// 	// gpio_set_level(1,0);
	// 	// vTaskDelay(20 / portTICK_PERIOD_MS);
	// 	// gpio_set_level(1,1);
	// 	// vTaskDelay(20 / portTICK_PERIOD_MS);
    // }



}















