/* $Id: form.h,v 1.6 2003/09/22 21:02:18 ukai Exp $ */
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

#define MAX_TEXTAREA 10		/* max number of <textarea>..</textarea> 
				 * within one document */
#ifdef MENU_SELECT
#define MAX_SELECT 10		/* max number of <select>..</select>
				 * within one document */
#endif				/* MENU_SELECT */

typedef struct form_list {
    struct form_item_list *item;
    struct form_item_list *lastitem;
    int method;
    Str action;
    char *target;
    char *name;
#ifdef USE_M17N
    wc_ces charset;
#endif
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

void addSelectOption(FormSelectOption *fso, Str value, Str label, int chk);
void chooseSelectOption(struct form_item_list *fi, FormSelectOptionItem *item);
void updateSelectOption(struct form_item_list *fi, FormSelectOptionItem *item);
int formChooseOptionByMenu(struct form_item_list *fi, int x, int y);
#endif				/* MENU_SELECT */

typedef struct form_item_list {
    int type;
    Str name;
    Str value, init_value;
    int checked, init_checked;
    int accept;
    int size;
    int rows;
    int maxlength;
    int readonly;
#ifdef MENU_SELECT
    FormSelectOptionItem *select_option;
    Str label, init_label;
    int selected, init_selected;
#endif				/* MENU_SELECT */
    struct form_list *parent;
    struct form_item_list *next;
} FormItemList;

#endif				/* not FORM_H */
