
/* 
 * From g96p0935@mse.waseda.ac.jp Mon Jun 14 09:34:15 1999 Received: from
 * ei5sun.yz.yamagata-u.ac.jp (ei5sun.yz.yamagata-u.ac.jp [133.24.114.42])
 * by ei5nazha.yz.yamagata-u.ac.jp (8.9.3/8.9.3) with ESMTP id JAA20673 for 
 * <aito@ei5nazha.yz.yamagata-u.ac.jp>; Mon, 14 Jun 1999 09:34:14 +0900
 * (JST) Received: from pandora.mse.waseda.ac.jp
 * (root@pandora.mse.waseda.ac.jp [133.9.5.9]) by
 * ei5sun.yz.yamagata-u.ac.jp (8.8.0/3.5Wbeta) with ESMTP id JAA23968 for
 * <aito@ei5sun.yz.yamagata-u.ac.jp>; Mon, 14 Jun 1999 09:35:30 +0900 (JST)
 * Received: from localhost (root@[133.9.85.55]) by pandora.mse.waseda.ac.jp 
 * (8.9.1+3.0W/3.7W) with ESMTP id JAA18473; Mon, 14 Jun 1999 09:30:31 +0900 
 * (JST) Message-Id: <199906140030.JAA18473@pandora.mse.waseda.ac.jp> To:
 * aito@ei5sun.yz.yamagata-u.ac.jp Subject: w3m:英和辞典検索機能塔載
 * Cc: g96p0935@mse.waseda.ac.jp From: Takashi Nishimoto
 * <g96p0935@mse.waseda.ac.jp> X-Mailer: Mew version 1.93 on Emacs 19.34 /
 * Mule 2.3 (SUETSUMUHANA) Mime-Version: 1.0 Content-Type: Text/Plain;
 * charset=iso-2022-jp Content-Transfer-Encoding: 7bit Date: Mon, 14 Jun
 * 1999 09:29:56 +0900 X-Dispatcher: imput version 980506 Lines: 150
 * 
 * 西本@早大です。
 * 
 * Quick Hack で w3m
 * に英和辞典検索機能と単語単位のカーソル移動を実装しま した。
 * 
 * Unix を使っていると、英文を読む機会が多いですね。 Emacs
 * 内ではワンタッチで英和辞典を検索する sdic
 * のようなツールがありま
 * すが、ちょっとした文書を読むだけにいちいち Emacs
 * に読み込むのはかった るいので、なんとか w3m
 * でできないかと思い、作業に踏み切りました。
 * 
 * すると意外に簡単に実装できました。僕は C
 * プログラムの改造は初めてです が、 Emacs Lisp
 * 並の手軽さで実装できたことには感動しました。
 * 
 * dictword が調べる単語を聞いて検索する関数で、 dictwordat
 * がカーソル位
 * 置の単語を検索する関数です。ソースを見れば明らかなように検索する外部コ
 * マンドは w3mdict です。 Unix
 * なので、普段使っているコマンドへの  symlink にしています。w
 * に dictword、 W に dictwordat を割り当てていま
 * す。また、右手で読めるように ; にも dictwordat
 * を割り当てています。 */
#include "fm.h"
#include <signal.h>

#ifdef DICT

#define DICTCMD "w3mdict "
#define DICTBUFFERNAME "*dictionary*"
/* char *DICTBUFFERNAME="*dictionary*"; */

char *
GetWord(char *word)
{
    Line *l = Currentbuf->currentLine;
    char *lb = l->lineBuf;
    int i, b, e, pos = Currentbuf->pos;

    i = pos;
    while (!IS_ALPHA(lb[i]) && i >= 0)
	i--;
    pos = i;
    while (IS_ALPHA(lb[i]) && i >= 0)
	i--;
    i++;
    if (!IS_ALPHA(lb[i]))
	return NULL;
    b = i;
    i = pos;
    while (IS_ALPHA(lb[i]) && i <= l->len - 1)
	i++;
    e = i - 1;
    strncpy(word, &lb[b], e - b + 1);
    word[e - b + 1] = '\0';
    return word;
}

void
execdict(char *word)
{
    Buffer *buf;
    static char cmd[100], bufname[100];
    MySignalHandler(*prevtrap) ();

    if (word == NULL)
	return;
    strcpy(cmd, DICTCMD);
    strcat(cmd, word);
    buf = namedBuffer(Firstbuf, SHELLBUFFERNAME);
    if (buf != NULL)
	Firstbuf = deleteBuffer(Firstbuf, buf);

    if (cmd == NULL || *cmd == '\0') {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    prevtrap = signal(SIGINT, intTrap);
    crmode();
    buf = getshell(cmd);
/* sprintf(bufname,"*dictionary(%s)*",word); */
/* buf->buffername = bufname; */
    buf->buffername = DICTBUFFERNAME;
    buf->filename = word;
    signal(SIGINT, prevtrap);
    term_raw();
    if (buf == NULL) {
	disp_message("Execution failed", FALSE);
    }
    else if (buf->firstLine == NULL) {
	/* if the dictionary doesn't describe the word. */
	char msg[100];
	sprintf(msg, "Word \"%s\" Not Found", word);
	disp_message(msg, FALSE);

    }
    else {
	buf->nextBuffer = Firstbuf;
	Currentbuf = Firstbuf = buf;
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

void
dictword(void)
{
    execdict(inputStr("(dictionary)!", ""));
}

void
dictwordat(void)
{
    static char word[100];
    execdict(GetWord(word));
}
#endif				/* DICT */
