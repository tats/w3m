#ifndef PARSETAG_H
#define PARSETAG_H
struct parsed_tagarg {
    char *arg;
    char *value;
    struct parsed_tagarg *next;
};

extern char *tag_get_value(struct parsed_tagarg *t, char *arg);
extern int tag_exists(struct parsed_tagarg *t, char *arg);
extern struct parsed_tagarg *cgistr2tagarg(char *cgistr);
#endif				/* not PARSETAG_H */
