// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.1
// LVGL version: 8.3.6
// Project name: Watch

#include "../ui.h"

uint16_t AngTab[16] = {450,300,150,3600,3450,3300,3150,3000,2850,2700,2550,2400,2250,2100,1950,1800};
lv_anim_t Boost1_Anim;

static void Boost1_Anim_CB(void *var, int32_t v)
{
static uint8_t Flag = 0;
    if(v == 0 )
    {
        if(Flag == 1)
        {
            lv_disp_load_scr(ui_BoostModeB);
            Boost2_AnimBegin();
        }

    }
    else if(v <= 16  )
    {
        lv_obj_add_flag(ui_MainIndecA[v-1], LV_OBJ_FLAG_HIDDEN);

        if(v > 4 )
        {
            lv_obj_clear_flag(ui_ChargeIndecB[(v-4)/4], LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(ui_ChargeIndecA[(v-4)/4], LV_OBJ_FLAG_HIDDEN);
        }
    }
    else
    {
        Flag =1;
        lv_obj_add_flag(ui_MainIndecB[v-17], LV_OBJ_FLAG_HIDDEN);
    }

    lv_label_set_text_fmt(ui_SecNum,"%ld",21-v);

}



void ui_BoostModeA_screen_init(void)
{
    uint8_t i;

    ui_BoostModeA = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_BoostModeA, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_img_src(ui_BoostModeA, &ui_img_akf_gg_png, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_opa(ui_BoostModeA, 255, LV_PART_MAIN | LV_STATE_DEFAULT);



    for (i = 0; i < 4; i++)
    {
        ui_ChargeIndecB[i] = lv_img_create(ui_BoostModeA);
        lv_img_set_src(ui_ChargeIndecB[i], &ui_img_centerindec_png);
        lv_obj_set_width(ui_ChargeIndecB[i], LV_SIZE_CONTENT);   /// 12
        lv_obj_set_height(ui_ChargeIndecB[i], LV_SIZE_CONTENT);    /// 15
        lv_obj_set_x(ui_ChargeIndecB[i], -4);
        lv_obj_set_y(ui_ChargeIndecB[i], 6);
        lv_obj_set_align(ui_ChargeIndecB[i], LV_ALIGN_CENTER);
        lv_obj_add_flag(ui_ChargeIndecB[i], LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(ui_ChargeIndecB[i], LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_img_set_pivot(ui_ChargeIndecB[i], 8, 1);
        lv_img_set_angle(ui_ChargeIndecB[i], i*400);
        lv_obj_add_flag(ui_ChargeIndecB[i], LV_OBJ_FLAG_HIDDEN);     /// Flags
    }


    for (i = 0; i < 4; i++)
    {
        ui_ChargeIndecA[i] = lv_img_create(ui_BoostModeA);
        lv_img_set_src(ui_ChargeIndecA[i], &ui_img_secondindec_png);
        lv_obj_set_width(ui_ChargeIndecA[i], LV_SIZE_CONTENT);   /// 237
        lv_obj_set_height(ui_ChargeIndecA[i], LV_SIZE_CONTENT);    /// 237
        lv_obj_set_x(ui_ChargeIndecA[i], 65);
        lv_obj_set_y(ui_ChargeIndecA[i], 2);
        lv_obj_set_align(ui_ChargeIndecA[i], LV_ALIGN_CENTER);
        lv_obj_add_flag(ui_ChargeIndecA[i], LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(ui_ChargeIndecA[i], LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_img_set_pivot(ui_ChargeIndecA[i], -10, 14);
        lv_img_set_angle(ui_ChargeIndecA[i], -95*i);
        lv_obj_add_flag(ui_ChargeIndecA[i], LV_OBJ_FLAG_HIDDEN);     /// Flags
    }

    for (i = 0; i < 16; i++)
    {
        ui_MainIndecA[i] = lv_img_create(ui_BoostModeA);
        lv_img_set_src(ui_MainIndecA[i], &ui_img_akf_png);
        lv_obj_set_width(ui_MainIndecA[i], LV_SIZE_CONTENT);   /// 27
        lv_obj_set_height(ui_MainIndecA[i], LV_SIZE_CONTENT);    /// 92
        lv_obj_set_x(ui_MainIndecA[i], -15);
        lv_obj_set_y(ui_MainIndecA[i], -63);
        lv_obj_set_align(ui_MainIndecA[i], LV_ALIGN_CENTER);
        lv_obj_add_flag(ui_MainIndecA[i], LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(ui_MainIndecA[i], LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_img_set_pivot(ui_MainIndecA[i], 27, 109);
        lv_img_set_angle(ui_MainIndecA[i],AngTab[i]);
    }


    for (i = 0; i < 5; i++)
    {
        ui_MainIndecB[i] = lv_img_create(ui_BoostModeA);
        lv_img_set_src(ui_MainIndecB[i], &ui_img_smallindecotr_1_png);
        lv_obj_set_width(ui_MainIndecB[i], LV_SIZE_CONTENT);   /// 22
        lv_obj_set_height(ui_MainIndecB[i], LV_SIZE_CONTENT);    /// 25
        lv_obj_set_x(ui_MainIndecB[i], 37);
        lv_obj_set_y(ui_MainIndecB[i], 89);
        lv_obj_set_align(ui_MainIndecB[i], LV_ALIGN_CENTER);
        lv_obj_add_flag(ui_MainIndecB[i], LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
        lv_obj_clear_flag(ui_MainIndecB[i], LV_OBJ_FLAG_SCROLLABLE);      /// Flags
        lv_img_set_pivot(ui_MainIndecB[i], -27, -80);
        lv_img_set_angle(ui_MainIndecB[i], -100*i);
    }

    ui_BoxSide = lv_img_create(ui_BoostModeA);
    lv_img_set_src(ui_BoxSide, &ui_img_akf_line_png);
    lv_obj_set_width(ui_BoxSide, LV_SIZE_CONTENT);   /// 62
    lv_obj_set_height(ui_BoxSide, LV_SIZE_CONTENT);    /// 8
    lv_obj_set_x(ui_BoxSide, 38);
    lv_obj_set_y(ui_BoxSide, -22);
    lv_obj_set_align(ui_BoxSide, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_BoxSide, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_BoxSide, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_SecNumBox = lv_obj_create(ui_BoostModeA);
    lv_obj_set_width(ui_SecNumBox, 45);
    lv_obj_set_height(ui_SecNumBox, 30);
    lv_obj_set_x(ui_SecNumBox, 36);
    lv_obj_set_y(ui_SecNumBox, 103);
    lv_obj_set_align(ui_SecNumBox, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(ui_SecNumBox, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_SecNumBox, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_SecNumBox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_SecNumBox, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_SecNumBox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_SecNum = lv_label_create(ui_SecNumBox);
    lv_obj_set_width(ui_SecNum, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_SecNum, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_SecNum, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(ui_SecNum,"%d",00);
    lv_obj_set_style_text_color(ui_SecNum, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_SecNum, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui_SecNum, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui_SecNum, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_SecNum, &ui_font_Seg40, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_SecLable = lv_label_create(ui_BoostModeA);
    lv_obj_set_width(ui_SecLable, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_SecLable, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_SecLable, 49);
    lv_obj_set_y(ui_SecLable, 19);
    lv_obj_set_align(ui_SecLable, LV_ALIGN_CENTER);
    lv_label_set_text(ui_SecLable, "SEC.");
    lv_obj_set_style_text_color(ui_SecLable, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_SecLable, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_SecLable, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_anim_init(&Boost1_Anim);
    lv_anim_set_var(&Boost1_Anim,ui_MainIndecA);
    lv_anim_set_values(&Boost1_Anim,0,21);
    lv_anim_set_time(&Boost1_Anim, 121000);
    lv_anim_set_exec_cb(&Boost1_Anim, Boost1_Anim_CB);
    lv_anim_set_path_cb(&Boost1_Anim,lv_anim_path_linear);
    lv_anim_set_repeat_count(&Boost1_Anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&Boost1_Anim);

}


void Boost1_Anim_Begin()
{
    lv_anim_start(&Boost1_Anim);
}


void Boost1_Anim_Del()
{
    lv_anim_del(ui_MainIndecA,Boost1_Anim_CB);
}