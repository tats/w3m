/* 
 * HTML forms
 */
#ifndef FORM_H
#define FORM_H

#include "Str.h"

#define FORM_UNKNOWN        -1
#define FORM_INPUT_TEXT     0
#define FORM_INPUT_PASSWORD 1
#define FORM_INPUT_CHECKBOX 2
#define FORM_INPUT_RADIO    3
#define FORM_INPUT_SUBMIT   4
#define FORM_INPUT_RESET    5
#define FORM_INPUT_HIDDEN   6
#define FORM_INPUT_IMAGE    7
#define FORM_SELECT         8
#define FORM_TEXTAREA       9
#define FORM_INPUT_BUTTON   10
#define FORM_INPUT_FILE     11

#define FORM_I_TEXT_DEFAULT_SIZE 40
#define FORM_I_SELECT_DEFAULT_SIZE 40
#define FORM_I_TEXTAREA_DEFAULT_WIDTH 40

#define FORM_METHOD_GET 0
#define FORM_METHOD_POST 1
#define FORM_METHOD_INTERNAL 2
#define FORM_METHOD_HEAD 3

#define FORM_ENCTYPE_URLENCODED 0
#define FORM_ENCTYPE_MULTIPART  1

#define MAX_TEXTAREA 100	/* max number of * <textarea>..</textarea> 
				 * within one * document */
#ifdef MENU_SELECT
#define MAX_SELECT 100		/* max number of <select>..</select> *
				 * within one document */
#endif				/* MENU_SELECT */

typedef struct form_list {
    struct form_item_list *item;
    struct form_item_list *lastitem;
    int method;
    Str action;
    char *target;
    int charset;
    int enctype;
    struct form_list *next;
    int nitems;
    char *body;
    char *boundary;
    unsigned long length;
} FormList;

#ifdef MENU_SELECT
typedef struct form_select_option_item {
    Str value;
    Str label;
    int checked;
    struct form_select_option_item *next;
} FormSelectOptionItem;

typedef struct form_select_option {
    FormSelectOptionItem *first;
    FormSelectOptionItem *last;
} FormSelectOption;

void addSelectOption(FormSelectOption * fso, Str value, Str label, int chk);
Str chooseSelectOption(FormSelectOptionItem * item, int choose_type);
void formChooseOptionByMenu(struct form_item_list *fi, int x, int y);
/* macros for chooseSelectOption */
#define CHOOSE_OPTION 0
#define CHOOSE_VALUE 1
#endif				/* MENU_SELECT */

typedef struct form_item_list {
    int type;
    Str name;
    Str value;
    int checked;
    int accept;
    int size;
    int rows;
    int maxlength;
#ifdef MENU_SELECT
    FormSelectOptionItem *select_option;
    Str label;
#endif				/* MENU_SELECT */
    struct form_list *parent;
    struct form_item_list *next;
    int anchor_num;
} FormItemList;

#endif				/* not FORM_H */
