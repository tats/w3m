/* params1 */

global int Tabstop init(8);
#ifdef JP_CHARSET
extern char DisplayCode;
#endif				/* JP_CHARSET */
global char RenderFrame init(FALSE);
global char TargetSelf init(FALSE);
global int displayLink init(FALSE);
global int UseExternalDirBuffer init(TRUE);
#ifdef __EMX__
global char *DirBufferCommand init("file:///$LIB/dirlist.cmd");
#else
global char *DirBufferCommand init("file:///$LIB/dirlist.cgi");
#endif
global int multicolList init(FALSE);

/* params2 */
#ifdef COLOR
global int useColor init(TRUE);
global int basic_color init(8);	/* don't change */
global int anchor_color init(4);	/* blue  */
global int image_color init(2);	/* green */
global int form_color init(1);	/* red   */
#ifdef BG_COLOR
global int bg_color init(8);	/* don't change */
#endif				/* BG_COLOR */
global int useActiveColor init(FALSE);
global int active_color init(6);	/* cyan */
global int useVisitedColor init(FALSE);
global int visited_color init(5);	/* magenta  */
#endif				/* COLOR */

/* params3 */

global int PagerMax init(PAGER_MAX_LINE);
#ifdef USE_HISTORY
global int URLHistSize init(100);
global int SaveURLHist init(TRUE);
#endif				/* USE_HISTORY */
global int confirm_on_quit init(TRUE);
global int showLineNum init(FALSE);
global char *ftppasswd init(NULL);
global char *UserAgent init(NULL);
global char *AcceptLang init(NULL);
global int WrapDefault init(FALSE);
#ifdef USE_COOKIE
global int use_cookie init(TRUE);
global int accept_bad_cookie init(FALSE);
#endif				/* USE_COOKIE */

/* params4 */

global char *HTTP_proxy init(NULL);
global char *GOPHER_proxy init(NULL);
global char *FTP_proxy init(NULL);
global int NOproxy_netaddr init(TRUE);
global char *NO_proxy init(NULL);
#ifdef INET6
global int DNS_order init(0);
#endif				/* INET6 */

/* params5 */

global char *document_root init(NULL);
global char *personal_document_root init(NULL);
global char *cgi_bin init(NULL);

/* params6 */

global char *Editor init(DEF_EDITOR);
global char *Mailer init(DEF_MAILER);
global char *ExtBrowser init(DEF_EXT_BROWSER);
global char *ExtBrowser2 init(NULL);
global char *ExtBrowser3 init(NULL);
global int BackgroundExtViewer init(TRUE);
