
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_port_disp.h"
#include "LVGL_UI.h"
#include "MathFun.h"


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


#define ROTATEDIR_FORWARD     (0x01)//顺时针
#define ROTATEDIR_OPPOSITE    (0xFF)//逆时针



lv_obj_t *Eye_Group[4];
lv_obj_t *Eye_base[4];//瞳孔
lv_obj_t *Eye_in_high[4];
lv_obj_t *Eye_in_low[4];
lv_anim_t EyeBodyPath_Anim;
lv_anim_t EyeFocalize_Anim;
lv_obj_t *BackGround;
lv_obj_t *Face;
lv_anim_t FaceAnim;


static uint8_t RotateDir = ROTATEDIR_OPPOSITE;

////口字形
/// 可调整 R_LEN 
static int8_t Eye_Position[4][3]=
{
    { R_LEN,  R_LEN, 0},
    {-R_LEN,  R_LEN, 1},
    {-R_LEN, -R_LEN, 2},
    { R_LEN, -R_LEN, 3},
};

///// 十字型
///// 可调整 R_LEN 
// int8_t Eye_Position[4][3]=
// {
//     {0,      -R_LEN, 0},
//     {R_LEN,  0,      1},
//     {0,      R_LEN,  2},
//     {-R_LEN, 0,      3},
// };

static void Eye_BodyAnimPath();

static void FaceAnim_CB(void *var, int32_t v)
{
    uint8_t i=0;

    for ( i = 0; i < 4; i++)
    {
        lv_obj_set_style_opa(Eye_Group[i],v,LV_PART_MAIN);
    }
    if(LV_OPA_COVER == v)
    {
        lv_anim_del(FaceAnim.var,FaceAnim.exec_cb);
        lv_anim_start(&EyeBodyPath_Anim);
    }
}


static void Face_Create()
{
    BackGround = lv_obj_create(NULL);
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

    lv_obj_set_style_outline_opa(Face,LV_OPA_90,LV_PART_MAIN);
    lv_obj_set_style_outline_width(Face,8,LV_PART_MAIN);
    lv_obj_set_style_outline_color(Face,lv_palette_main(LV_PALETTE_GREY),LV_PART_MAIN);//眼球外轮廓

    lv_obj_set_style_border_opa(Face,LV_OPA_80,LV_PART_MAIN);
    lv_obj_set_style_border_side(Face,LV_BORDER_SIDE_FULL,LV_PART_MAIN);
    lv_obj_set_style_border_color(Face,lv_palette_darken(LV_PALETTE_GREY,3),LV_PART_MAIN);
    lv_obj_set_style_border_width(Face,15,LV_PART_MAIN);

    lv_obj_set_scrollbar_mode(Face,LV_SCROLLBAR_MODE_OFF);
    lv_obj_center(Face);



    lv_anim_init(&FaceAnim);
    lv_anim_set_var(&FaceAnim,Face);
    lv_anim_set_values(&FaceAnim,0,LV_OPA_COVER);
    lv_anim_set_time(&FaceAnim, 1000);
    lv_anim_set_delay(&FaceAnim, 10);
    lv_anim_set_exec_cb(&FaceAnim, FaceAnim_CB);
    lv_anim_set_path_cb(&FaceAnim,lv_anim_path_ease_in_out);
    lv_anim_set_repeat_count(&FaceAnim, 1);


}


//调整轨迹动画回调
static void Eye_BodyAnimPath_CB(void *var, int32_t v)
{
    uint8_t i;
    int16_t x1,y1;

    for ( i = 0; i < 4; i++)
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

            lv_anim_set_delay(&EyeBodyPath_Anim, 0);
            //旋转结束后调整眼部焦距
            lv_anim_set_repeat_count(&EyeFocalize_Anim, 2);
            lv_anim_start(&EyeFocalize_Anim);
        }
    }
}

//调整焦距动画回调
static void ChangeEyeFocalize_CB(void *var, int32_t v)
{
	uint8_t i;
    int16_t x1,y1;

    for ( i = 0; i < 4; i++)
    {
        lv_obj_set_size(Eye_base[i],v,v);
        lv_obj_set_style_radius(Eye_base[i],v/2,LV_PART_MAIN);
        lv_obj_align_to(Eye_base[i],Eye_Group[i],LV_ALIGN_CENTER,0,0);

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


//创建眼球主体
static void Eye_BodyCreate()
{
    uint8_t i=0;

    for ( i = 0; i < 4; i++)
    {
		//眼球 37,58,18   56,110,23   
		Eye_Group[i] = lv_obj_create(Face);
		lv_obj_set_size(Eye_Group[i],EYE_SIZE,EYE_SIZE);
		lv_obj_set_style_bg_color(Eye_Group[i],lv_color_hex(0x32760B),LV_PART_MAIN);//瞳孔颜色
		lv_obj_set_style_radius(Eye_Group[i],EYE_SIZE/2,LV_PART_MAIN);
		lv_obj_set_style_outline_width(Eye_Group[i],3,LV_PART_MAIN);
		lv_obj_set_style_outline_color(Eye_Group[i],lv_color_hex(0x32760B),LV_PART_MAIN);//瞳孔外轮廓颜色
		lv_obj_set_scrollbar_mode(Eye_Group[i],LV_SCROLLBAR_MODE_OFF);//关闭滚动条
        lv_obj_align_to(Eye_Group[i],Face,LV_ALIGN_CENTER,Eye_Position[i][0],Eye_Position[i][1]);
		lv_obj_clear_flag(Eye_Group[i],LV_OBJ_FLAG_CLICKABLE );//不可拖动
        lv_obj_set_style_opa(Eye_Group[i],LV_OPA_TRANSP,LV_PART_MAIN);

		//瞳孔
		Eye_base[i] = lv_obj_create(Eye_Group[i]);
		lv_obj_set_size(Eye_base[i],EYE_HOLE_SIZE,EYE_HOLE_SIZE);
		lv_obj_set_style_radius(Eye_base[i],EYE_HOLE_SIZE/2,LV_PART_MAIN);
		lv_obj_align_to(Eye_base[i],Eye_Group[i],LV_ALIGN_CENTER,0,0);
		lv_obj_set_style_bg_color(Eye_base[i],lv_color_black(),LV_PART_MAIN);
		lv_obj_set_style_outline_width(Eye_base[i],3,LV_PART_MAIN);
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
static void Eye_BodyAnimPath()
{	
    //创建眼球轨迹动画
    lv_anim_init(&EyeBodyPath_Anim);
    lv_anim_set_var(&EyeBodyPath_Anim,Eye_Group);
    lv_anim_set_values(&EyeBodyPath_Anim,0,90);
    lv_anim_set_time(&EyeBodyPath_Anim, 300);
    lv_anim_set_delay(&EyeBodyPath_Anim, 400);
    lv_anim_set_exec_cb(&EyeBodyPath_Anim, Eye_BodyAnimPath_CB);
    lv_anim_set_path_cb(&EyeBodyPath_Anim,lv_anim_path_ease_in_out);
    lv_anim_set_repeat_delay(&EyeBodyPath_Anim,200);
    // lv_anim_set_repeat_count(&EyeBodyPath_Anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_repeat_count(&EyeBodyPath_Anim, 3);
    
}


//创建瞳孔焦距动画
static void EyeFocalizeAnimCreat()
{

    lv_anim_init(&EyeFocalize_Anim);
    lv_anim_set_var(&EyeFocalize_Anim,Eye_base);
    lv_anim_set_values(&EyeFocalize_Anim,EYE_HOLE_SIZE,EYE_HOLE_MIN_SIZE);
    lv_anim_set_time(&EyeFocalize_Anim, 200);
    lv_anim_set_delay(&EyeFocalize_Anim, 500);
    lv_anim_set_exec_cb(&EyeFocalize_Anim, ChangeEyeFocalize_CB);
    lv_anim_set_path_cb(&EyeFocalize_Anim,lv_anim_path_ease_in_out);
    lv_anim_set_playback_time(&EyeFocalize_Anim,200);
    lv_anim_set_playback_delay(&EyeFocalize_Anim,200);
    lv_anim_set_repeat_delay(&EyeFocalize_Anim,300);
    lv_anim_set_repeat_count(&EyeFocalize_Anim, LV_ANIM_REPEAT_INFINITE);

}


//设置眼睛旋转方向和次数
void RotateEye(uint8_t Dir ,uint8_t times)
{
    lv_anim_set_repeat_count(&EyeBodyPath_Anim, times);
	RotateDir = Dir;
    lv_anim_start(&EyeBodyPath_Anim);
}

//设置调整瞳孔焦距次数
void ChangeEyeFocalize(uint8_t times)
{
    lv_anim_set_repeat_count(&EyeFocalize_Anim, times);
    lv_anim_start(&EyeFocalize_Anim);
}

//设置眼底颜色
void SetEyeBgColor(lv_palette_t color)
{
	uint8_t i=0;

    for ( i = 0; i < 4; i++)
    {
		lv_obj_set_style_bg_color(Eye_Group[i],lv_palette_main(color),LV_PART_MAIN);
    }
}

void SetEyeBgColorRGB(uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t i=0;

    for ( i = 0; i < 4; i++)
    {
		lv_obj_set_style_bg_color(Eye_Group[i],lv_color_make(r,g,b),LV_PART_MAIN);
    }
}



void Eye_Main()
{
	Face_Create();
	Eye_BodyCreate();
    Eye_BodyAnimPath();
	EyeFocalizeAnimCreat();
    // lv_anim_start(&FaceAnim);

}


void Eye_Anim_Del()
{
    lv_anim_del(FaceAnim.var,FaceAnim.exec_cb);
    lv_anim_del(EyeBodyPath_Anim.var, EyeBodyPath_Anim.exec_cb);
    lv_anim_del(EyeFocalize_Anim.var, EyeFocalize_Anim.exec_cb);
}



void Eye_Anim_Begin()
{
    lv_anim_start(&FaceAnim);
}




