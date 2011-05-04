
#include "wc.h"

static wc_ces char_conv_f_ces = 0, char_conv_t_ces = WC_CES_WTF;
static wc_status char_conv_st;

void
wc_char_conv_init(wc_ces f_ces, wc_ces t_ces)
{
    wc_input_init(f_ces, &char_conv_st);
    char_conv_st.state = -1;
    char_conv_f_ces = f_ces;
    char_conv_t_ces = t_ces;
}

Str
wc_char_conv(char c)
{
    return wc_Str_conv((*char_conv_st.ces_info->char_conv)((wc_uchar)c, &char_conv_st),
	WC_CES_WTF, char_conv_t_ces);
}
