#include "Key.h"
#include "driver/gpio.h"

#define EVENT_CB(ev)   if(handle->cb[ev])handle->cb[ev]((Button*)handle)

uint8_t KeyPin[Key_MaxNum]=
{
	KEY_LEFT_PIN,
	KEY_DOWN_PIN,
	KEY_UP_PIN,
	KEY_RIGHT_PIN,
};

static  gpio_config_t GPIO = 
{
    .pin_bit_mask = 0,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = 1,
    .pull_down_en = 0,
    .intr_type = GPIO_INTR_DISABLE,
};


static struct Button* head_handle = NULL;
static uint8_t KeyState[Key_MaxNum];
static KeyInfo_t KeyInfo;


struct Button Button_Up;
struct Button Button_Down;
struct Button Button_Left;
struct Button Button_Right;

void button_init(struct Button* handle, uint8_t(*pin_level)(uint8_t), uint8_t active_level, uint8_t button_id)
{
	memset(handle, 0, sizeof(struct Button));
	handle->event = (uint8_t)NONE_PRESS;
	handle->hal_button_Level = pin_level;
	handle->button_level = handle->hal_button_Level(button_id);
	handle->active_level = active_level;
	handle->button_id = button_id;
}


void button_attach(struct Button* handle, PressEvent event, BtnCallback cb)
{
	handle->cb[event] = cb;
}


PressEvent get_button_event(struct Button* handle)
{
	return (PressEvent)(handle->event);
}


void button_handler(struct Button* handle)
{
	uint8_t read_gpio_level = handle->hal_button_Level(handle->button_id);

	//ticks counter working..
	if((handle->state) > 0) handle->ticks++;

	/*------------button debounce handle---------------*/
	if(read_gpio_level != handle->button_level) { //not equal to prev one
		//continue read 3 times same new level change
		if(++(handle->debounce_cnt) >= DEBOUNCE_TICKS) {
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
		}
	} else { //leved not change ,counter reset.
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	switch (handle->state) {
	case 0:
		if(handle->button_level == handle->active_level) {	//start press down
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->ticks = 0;
			handle->repeat = 1;
			handle->state = 1;
		} else {
			handle->event = (uint8_t)NONE_PRESS;
		}
		break;

	case 1:
		if(handle->button_level != handle->active_level) { //released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->ticks = 0;
			handle->state = 2;

		} else if(handle->ticks > LONG_TICKS) {
			handle->event = (uint8_t)LONG_PRESS_START;
			EVENT_CB(LONG_PRESS_START);
			handle->state = 5;
		}
		break;

	case 2:
		if(handle->button_level == handle->active_level) { //press down again
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->repeat++;
			EVENT_CB(PRESS_REPEAT); // repeat hit
			handle->ticks = 0;
			handle->state = 3;
		} else if(handle->ticks > SHORT_TICKS) { //released timeout
			if(handle->repeat == 1) {
				handle->event = (uint8_t)SINGLE_CLICK;
				EVENT_CB(SINGLE_CLICK);
			} else if(handle->repeat == 2) {
				handle->event = (uint8_t)DOUBLE_CLICK;
				EVENT_CB(DOUBLE_CLICK); // repeat hit
			}
			handle->state = 0;
		}
		break;

	case 3:
		if(handle->button_level != handle->active_level) { //released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			if(handle->ticks < SHORT_TICKS) {
				handle->ticks = 0;
				handle->state = 2; //repeat press
			} else {
				handle->state = 0;
			}
		}else if(handle->ticks > SHORT_TICKS){ // long press up
			handle->state = 0;
		}
		break;

	case 5:
		if(handle->button_level == handle->active_level) {
			//continue hold trigger
			handle->event = (uint8_t)LONG_PRESS_HOLD;
			EVENT_CB(LONG_PRESS_HOLD);

		} else { //releasd
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->state = 0; //reset
		}
		break;
    default:
        handle->state = 0; //reset
        break;
	}
}


int button_start(struct Button* handle)
{
	struct Button* target = head_handle;
	while(target) {
		if(target == handle) return -1;	//already exist.
		target = target->next;
	}
	handle->next = head_handle;
	head_handle = handle;
	return 0;
}


void button_stop(struct Button* handle)
{
	struct Button** curr;
	for(curr = &head_handle; *curr; ) {
		struct Button* entry = *curr;
		if (entry == handle) {
			*curr = entry->next;
			return;//glacier add 2021-8-18
		} else
			curr = &entry->next;
	}
}


void button_ticks()
{
	struct Button* target;
	for(target=head_handle; target; target=target->next) 
    {
		button_handler(target);
	}
}



uint8_t read_button_GPIO(uint8_t button_id)
{
	// you can share the GPIO read function with multiple Buttons
	switch(button_id)
	{
		case Key_Up:
			return KEY_UP_STATE;
		case Key_Down:
			return KEY_DOWN_STATE;
		case Key_Left:
			return KEY_LEFT_STATE;
		case Key_Right:
			return KEY_RIGHT_STATE;
		default:
			return 0;
	}
}


uint8_t Key_Info;


// void PRESS_DOWN_Handler(void *Data)
// {
//     struct Button *Tmp;

//     Tmp = (struct Button *)Data;

//     switch (Tmp->button_id)
//     {
//         case Key_Up:
//             printf("Key_Up    Down\r\n");
//             break;
//         case Key_Down:
//             printf("Key_Down    Down\r\n");
//             break;
//         case Key_Left:
//             printf("Key_Left    Down\r\n");
//             break;
//         case Key_Right:
//             printf("Key_Right    Down\r\n");
//             break;
//         default:
// 			printf("==============\r\n");
//             break;
//     }
// }


// void PRESS_UP_Handler(void *Data)
// {
//     struct Button *Tmp;
//     Tmp = (struct Button *)Data;
//     switch (Tmp->button_id)
//     {
//         case Key_Up:
//             printf("Key_Up    Up\r\n");
//             break;
//         case Key_Down:
//             printf("Key_Down    Up\r\n");
//             break;
//         case Key_Left:
//             printf("Key_Left    Up\r\n");
//             break;
//         case Key_Right:
//             printf("Key_Right    Up\r\n");
//             break;
//         default:
// 			printf("==============\r\n");
//             break;
//     }
// }

// void PRESS_REPEAT_Handler(void *Data)
// {
//     struct Button *Tmp;
//     Tmp = (struct Button *)Data;
//     switch (Tmp->button_id)
//     {
//         case Key_Up:
//             printf("Key_Up    REPEAT\r\n");
//             break;
//         case Key_Down:
//             printf("Key_Down    REPEAT\r\n");
//             break;
//         case Key_Left:
//             printf("Key_Left    REPEAT\r\n");
//             break;
//         case Key_Right:
//             printf("Key_Right    REPEAT\r\n");
//             break;
//         default:
// 			printf("==============\r\n");
//             break;
//     }
// }



void SINGLE_Click_Handler(void *Data)
{
    struct Button *Tmp;
    Tmp = (struct Button *)Data;
    switch (Tmp->button_id)
    {
        case Key_Up:
            printf("Key_Up    Click\r\n");
			KeyState[Key_Up] = SINGLE_CLICK;
            break;
        case Key_Down:
            printf("Key_Down    Click\r\n");
			KeyState[Key_Down] = SINGLE_CLICK;
            break;
        case Key_Left:
            printf("Key_Left    Click\r\n");
			KeyState[Key_Left] = SINGLE_CLICK;
            break;
        case Key_Right:
            printf("Key_Right    Click\r\n");
			KeyState[Key_Right] = SINGLE_CLICK;
            break;
        default:
		printf("==============\r\n");
            break;
    }
}

void DOUBLE_Click_Handler(void *Data)
{
    struct Button *Tmp;
    Tmp = (struct Button *)Data;
    switch (Tmp->button_id)
    {
        case Key_Up:
            printf("Key_Up    Click * 2\r\n");
			KeyState[Key_Up] = DOUBLE_CLICK;
            break;
        case Key_Down:
            printf("Key_Down    Click * 2\r\n");
			KeyState[Key_Down] = DOUBLE_CLICK;
            break;
        case Key_Left:
            printf("Key_Left    Click * 2\r\n");
			KeyState[Key_Left] = DOUBLE_CLICK;
            break;
        case Key_Right:
            printf("Key_Right    Click * 2\r\n");
			KeyState[Key_Right] = DOUBLE_CLICK;
            break;
        default:
			printf("==============\r\n");
            break;
    }
}

// void LONG_PRESS_START_Handler(void *Data)
// {
//     struct Button *Tmp;
//     Tmp = (struct Button *)Data;
//     switch (Tmp->button_id)
//     {
//         case Key_Up:
//             printf("Key_Up    LONG_PRESS_START\r\n");
//             break;
//         case Key_Down:
//             printf("Key_Down   LONG_PRESS_START\r\n");
//             break;
//         case Key_Left:
//             printf("Key_Left    LONG_PRESS_START\r\n");
//             break;
//         case Key_Right:
//             printf("Key_Right    LONG_PRESS_START\r\n");
//             break;
//         default:
// 			printf("==============\r\n");
//             break;
//     }
// }

// void LONG_PRESS_HOLD_Handler(void *Data)
// {
//     struct Button *Tmp;
//     Tmp = (struct Button *)Data;
//     switch (Tmp->button_id)
//     {
//         case Key_Up:
//             printf("Key_Up    LONG_PRESS_HOLD\r\n");
//             break;
//         case Key_Down:
//             printf("Key_Down   LONG_PRESS_HOLD\r\n");
//             break;
//         case Key_Left:
//             printf("Key_Left    LONG_PRESS_HOLD\r\n");
//             break;
//         case Key_Right:
//             printf("Key_Right    LONG_PRESS_HOLD\r\n");
//             break;
//         default:
// 			printf("==============\r\n");
//             break;
//     }
// }



void GPIO_Init(void)
{
	uint64_t PinTem=0;
	uint8_t i;

	for ( i = 0; i < Key_MaxNum; i++)
	{
		PinTem |= ((1ULL << (uint8_t)(KeyPin[i])));
	}

	GPIO.pin_bit_mask = PinTem;

	gpio_config(&GPIO);
}


void Key_Init()
{

	GPIO_Init();

    button_init(&Button_Up,read_button_GPIO,KEY_ACT_LEVEL,Key_Up);
    button_init(&Button_Down,read_button_GPIO,KEY_ACT_LEVEL,Key_Down);
    button_init(&Button_Left,read_button_GPIO,KEY_ACT_LEVEL,Key_Left);
    button_init(&Button_Right,read_button_GPIO,KEY_ACT_LEVEL,Key_Right);


    // button_attach(&Button_Up, PRESS_DOWN,       PRESS_DOWN_Handler);
	// button_attach(&Button_Up, PRESS_UP,         PRESS_UP_Handler);
	// button_attach(&Button_Up, PRESS_REPEAT,     PRESS_REPEAT_Handler);
	button_attach(&Button_Up, SINGLE_CLICK,     SINGLE_Click_Handler);
	button_attach(&Button_Up, DOUBLE_CLICK,     DOUBLE_Click_Handler);
	// button_attach(&Button_Up, LONG_PRESS_START, LONG_PRESS_START_Handler);
	// button_attach(&Button_Up, LONG_PRESS_HOLD,  LONG_PRESS_HOLD_Handler);

    // button_attach(&Button_Down, PRESS_DOWN,       PRESS_DOWN_Handler);
	// button_attach(&Button_Down, PRESS_UP,         PRESS_UP_Handler);
	// button_attach(&Button_Down, PRESS_REPEAT,     PRESS_REPEAT_Handler);
	button_attach(&Button_Down, SINGLE_CLICK,     SINGLE_Click_Handler);
	button_attach(&Button_Down, DOUBLE_CLICK,     DOUBLE_Click_Handler);
	// button_attach(&Button_Down, LONG_PRESS_START, LONG_PRESS_START_Handler);
	// button_attach(&Button_Down, LONG_PRESS_HOLD,  LONG_PRESS_HOLD_Handler);

    // button_attach(&Button_Left, PRESS_DOWN,       PRESS_DOWN_Handler);
	// button_attach(&Button_Left, PRESS_UP,         PRESS_UP_Handler);
	// button_attach(&Button_Left, PRESS_REPEAT,     PRESS_REPEAT_Handler);
	button_attach(&Button_Left, SINGLE_CLICK,     SINGLE_Click_Handler);
	button_attach(&Button_Left, DOUBLE_CLICK,     DOUBLE_Click_Handler);
	// button_attach(&Button_Left, LONG_PRESS_START, LONG_PRESS_START_Handler);
	// button_attach(&Button_Left, LONG_PRESS_HOLD,  LONG_PRESS_HOLD_Handler);


    // button_attach(&Button_Right, PRESS_DOWN,       PRESS_DOWN_Handler);
	// button_attach(&Button_Right, PRESS_UP,         PRESS_UP_Handler);
	// button_attach(&Button_Right, PRESS_REPEAT,     PRESS_REPEAT_Handler);
	button_attach(&Button_Right, SINGLE_CLICK,     SINGLE_Click_Handler);
	button_attach(&Button_Right, DOUBLE_CLICK,     DOUBLE_Click_Handler);
	// button_attach(&Button_Right, LONG_PRESS_START, LONG_PRESS_START_Handler);
	// button_attach(&Button_Right, LONG_PRESS_HOLD,  LONG_PRESS_HOLD_Handler);

    button_start(&Button_Up);
    button_start(&Button_Down);
    button_start(&Button_Left);
    button_start(&Button_Right);

}   



KeyInfo_t GetKeyState()
{
	uint8_t i;
	for( i = 0; i < Key_MaxNum; i++)
	{
		if(KeyState[i] != 0xff)
		{
			break; 
		}
	}

	if( i >= Key_MaxNum) 
	{
		KeyInfo.KeyState = NONE_PRESS;
		KeyInfo.KeyNum = Key_MaxNum;
	}
	else
	{
		KeyInfo.KeyNum = i;
		KeyInfo.KeyState = KeyState[i];
	}

	memset(&KeyState,0xff,sizeof(KeyState));

	return KeyInfo;

}
