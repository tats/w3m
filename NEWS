Debian's w3m 0.5.3+git20230121

* new features
  - location of ~/.w3m can be changed with the W3M_DIR environment variable
  - new option tmp_dir to set directory for temporary files
    e.g. W3M_DIR=~/.local/state/w3m w3m -o tmp_dir=~/.cache/w3m
  - new option -H to use high-intensity colors
  - recognize link targets in dfn elements
* bug fixes
  - fix m17n backspace handling causes out-of-bounds write in checkType
    [CVE-2022-38223]
  - fix LESSOPEN format string problem
  - fix potential overflow in checkType
  - fix potential null deref for newBuffer
  - fix potential null deref in main.c, file.c, etc.c, fb_imlib2.c, news.c
  - fix browsing local file fails when argv_is_url
  - skip soft hyphen when reading token
  - fix configure tests broken with Clang 16
  - do not override history file if it was changed
  - only read a first title to avoid titles in svg
  - use GROFF_NO_SGR=1 for w3mman with non-Debian groff
  - handle failed system calls
  - fix charset declaration parser fails with turkish locale
  - fix images are not displayed for OSC 5379
  - prevent unneeded image resizing for sixel

Debian's w3m 0.5.3+git20220429

* new features
  - support kitty's APC G graphics protocol with ImageMagick's convert
  - support iTerm2's OSC 1337 graphics protocol
  - new option inline_img_protocol to select the graphics protocol
    (0: w3m-img, 1: OSC 5379, 2: sixel, 3: OSC 1337, 4: APC G)
  - new option ssl_cipher to specify TLSv1.2 ciphers, e.g. DEFAULT:@SECLEVEL=2
  - new option ssl_min_version for OpenSSL 1.1
  - new option -insecure to use insecure SSL config options
  - new option ssl_ca_default, explicitly use OpenSSL default paths by default
  - new option cross_origin_referer, use origin only Referer when cross origin
  - new option localhost_only to restrict connections only to localhost
  - new option disable_center to disable center alignment
  - support brotli content encoding
  - ignore the "-" option to accept `w3m -` as "read from stdin"
  - new configure option --with-cafile to detect CA bundle file
  - support auto-detection for configure --with-migemo
  - add fuzzer for OSS-Fuzz
  - add Italian translation
  - add Swedish translation
* bug fixes
  - prevent index overflow and huge allocation due to Str, libwc, and table
  - prevent integer overflow due to fontstat
  - prevent StrStream memory leak
  - prevent GC warnings of repeated allocation
  - prevent buffer overflow in shiftAnchorPosition
  - prevent buffer overflow READ when parsing Gopher URLs
  - prevent buffer overflow in gotoLine and gotoRealLine
  - prevent warnings when -Wnull-dereference, enabled by default
  - prevent warnings when -Wall, enabled by default
  - prevent warnings from Cppcheck
  - avoid zero length arrays even when GCC
  - fix fail to render over 32767 lines in a table cell
  - disable `<section>` behaves as `<hr>`
  - disable TLSv1.0 and TLSv1.1 by default
  - mention a workaround for SSL error
  - fix manipulation of ASN1_STRING
  - don't include username in Referer
  - don't set Referer when data URI scheme
  - fix broken anchor with link number at EOL
  - fix incorrect query string for `w3mman 7z`
  - drop imlib2-config, use pkg-config
  - improve named character references
  - improve `<dl>` rendering
  - prefer Imlib2 over GTK2 by default
  - replace encodeB with base64_encode to encode null bytes
  - wording fixes for configure --help

Debian's w3m 0.5.3+git20210102

* new features
  - support links containing divs for HTML5
  - rudimentary support for HTML5 tags: figure, figcaption, and section
  - enhance the behaviour of the q tag when m17n and Unicode are configured
  - support for file://hostname/... URLs
  - new commands CURSOR_TOP, CURSOR_MIDDLE, and CURSOR_BOTTOM
  - new option space_autocomplete, disabled by default
* bug fixes
  - fix and improve broken Gopher support, enabled by default
  - change the encoding of the Japanese document files to UTF-8
  - use the default ciphers without SSL_CTX_set_cipher_list for OpenSSL 1.1
  - fix compilation errors due to sys_errlist and longjmp
  - define X_DISPLAY_MISSING when configure --without-x for Imlib2
  - avoid the -l option of the man command for w3mman
  - fix some source formatting in the manual
  - show keyboard shortcuts in a consistent order in help
  - fix traditional Chinese translation
  - drop obsolete w3m-doc

Debian's w3m 0.5.3+git20200502

* bug fixes
  - support &apos; entity
  - prevent multiple User-Agent with -header
  - fix -Wchar-subscripts
* new features
  - support setting user_agent in siteconf
  - new command GOTO_HOME
  - extend ssl_forbid_method for TLSv1.2 and TLSv1.3

Debian's w3m 0.5.3+git20190105

* bug fixes
  - do not use deprecated features with OpenSSL 1.1
  - fix dependency for Imlib2
  - fix that the mark_all_pages option works
  - respect the simple_preserve_space option for table cells
  - fix error handling for ~/.w3m/request.log and localcgi_post()
* new feature
  - w3mman supports specifying a section number during a keyword search

Debian's w3m 0.5.3+git20180125

* bug fixes
  - fix stack overflow with malformed text [CVE-2018-6196]
  - fix null deref with malformed text [CVE-2018-6197]
  - fix /tmp file races only when ~/.w3m is unwritable [CVE-2018-6198]
  - do not remove w3mdict.cgi when "make distclean"
  - do not turn a form's GET into POST
  - correct <base ...> parsing
  - accept TERM=fbterm
* new feature
  - extend ssl_forbid_method to disable TLSv1.1

Debian's w3m 0.5.3+git20170102

* bug fixes
  - fix multiple flaws with malformed text
    (buffer overflow, use after free, infinite loop)
  - fix uninitialized variable when not USE_IMAGE

Debian's w3m 0.5.3+git20161120

* bug fixes
  - fix multiple flaws with malformed text
    (stack overflow, buffer overflow, null deref, out of memory)
    [CVE-2016-9622], [CVE-2016-9623], [CVE-2016-9624], [CVE-2016-9625],
    [CVE-2016-9626], [CVE-2016-9627], [CVE-2016-9628], [CVE-2016-9629],
    [CVE-2016-9630], [CVE-2016-9631], [CVE-2016-9632], [CVE-2016-9633]
  - fix stack overflow with nested table and textarea [CVE-2016-9439]
  - fix suspend (^Z) behavior

Debian's w3m 0.5.3+git20161031

* new features
  - support OSC 5379 remote imaging and sixel graphics
  - support SGR style mouse handler
  - support 32-bit color images
  - support FreeBSD framebuffer
  - support button element
  - support meta charset
  - include w3mdict.cgi to use a dictd dictionary query
  - add extbrowser4..9
  - add display_borders to display 0 pixel table borders
  - add siteconf feature
  - add German translation for options setting panel
  - add translations for de, zh_CN and zh_TW
* bug fixes
  - fix multiple flaws with malformed text
    [CVE-2016-9422], [CVE-2016-9423], [CVE-2016-9424], [CVE-2016-9425],
    [CVE-2016-9426], [CVE-2016-9428], [CVE-2016-9429], [CVE-2016-9430],
    [CVE-2016-9431], [CVE-2016-9432], [CVE-2016-9433], [CVE-2016-9434],
    [CVE-2016-9435], [CVE-2016-9436], [CVE-2016-9437], [CVE-2016-9438],
    [CVE-2016-9440], [CVE-2016-9441], [CVE-2016-9443], [CVE-2016-9621]
  - fix potential heap buffer corruption due to Strgrow [CVE-2016-9442]
  - disable SSLv2 and SSLv3 by default [CVE-2014-3566]
  - set ssl_verify_server to 1 by default
  - disable RC4, export ciphers, and keys < 128 bits
  - use SSL_OP_NO_COMPRESSION due to "CRIME attack" [CVE-2012-4929]
  - use SSL_MODE_RELEASE_BUFFERS
  - disable USE_EGD for LibreSSL
  - appease gcc -Werror=format-security
  - option -s is now "squeeze multiple blank lines" to work as pager, and
    -j and -e are obsolete, so use -O{s|j|e} to specify display charset
  - accept single quoted meta refresh URL
  - assume "text" if a form input type is unknown
  - accept cookies by default
  - set use_dictcommand to 1 by default
  - set default_url to 1 by default
  - set argv_is_url to 1 by default
  - set alt_entity to 0 by default
  - fix build problems with Boehm GC 7.2, imlib2 1.4.6 and glibc 2.14
  - fix parallel make failure
  - fix incorrect ucs_ambwidth_map
  - and many fixes

w3m 0.5.3 - 2011-01-15

* security fix
 - fix vulnerabilities indicated by bugs.debian.org.
 - suppress sending Referer, if https:// -> http://
* new features
 - adapt w3mimg to native windows on MS Windows.
 - support xterm-incompatible terminals without gpm.
 - add "xhtml" to default guess.
 - introduce option pseudo_inlines.
 - add option to avoid "wrong number of dots" error in cookies.
* other bug fixes
 - fix "important" bugs from bugs.debian.org
 - preserve spaces in multibyte context.
 - fix proxy authentication.

w3m 0.5.2 - 2007-05-31

* security fix
 - fix format string vulnerability.
* new features
 - support gtk2 with w3m-img.
 - new option for LiveHTTPHeaders-like logs.
 - new option to fontify <del>, <s>, <ins>, and so on.
* other bug fixes
 - avoid errors in "configure" and "make".
 - '\n' handling in attributes' values of HTML tags.

w3m 0.5.1 - 2004-04-29

* fix minor bugs
 - build problem on some platform/some configuration
 - authentication bug
 - ipv6 FQDN resolv
 - SSL verify
 - search problem on different charset page/display
 - cleanup LANG==JA
 - DisplayCharset default
 - w3mhelp.cgi charset

w3m 0.5	- 2004-03-22

* gettextize
* m17n patch merged

w3m 0.4.2 - 2003-09-23

* options: -4, -6
* configuration file in $(sysconfdir)/$(package)/
* func: NEXT_VISITED, PREV_VISITED
* autoconfiscate (partially)
* rc: use_history

w3m 0.4.1 - 2003-03-07

* fix bugs
  - completion segfault in lineinput
  - incremental search
  - URL pattern fix
  - UFhalfclose bug
  - allow pipe in shell command
  - enhance ftp directory support
  - linenumber in edit
  - fix Bug#181897
  - W3M_TTY problem fixed

w3m 0.4 - 2003-02-24

* rc: decode_url
* func: RESHAPE
* rc: fold_line
* local cookie: passed via file named $LOCAL_COOKIE or posted not in url query
* func: SEARCH can take arg
* URL data: support
* URL news:, nntp: newsgroup support
* rc: nntpserver, nntpmode, max_news
* rc: graphic_char
* rc: use_proxy
* rc: preserve_timestamp
* func: REDO, UNDO
* func: LIST, LIST_MENU, MOVE_LIST_MENU
* func: ACCESSKEY, LINK_MENU
* rc: display_ins_del
* 2 stroke keybinding
* func: MULTIMAP
* func: CLOSE_TAB_MOUSE, MENU_MOUSE, MOVE_MOUSE, TAB_MOUSE
* options: -N
* func: NEXT, PREV
* rc: image_map_list
* rc: open_tab_dl_list
* func: DOWNLOAD_LIST
* env: https_proxy
* rc: https_proxy
* options: -show-option
* rc: relative_wheel_scroll
* rc: relative_wheel_scroll_ratio
* rc: fixed_wheel_scroll_count
* separate auxbindir and libdir (local-CGI, file:///$LIB/)
* configure: -auxbindir
* rc: disable_secret_security_check (for windows?)
* tab browsing
* rc: open_tab_blank, close_tab_back
* func: CLOSE_TAB, NEW_TAB, NEXT_TAB, PREV_TAB, 
* func:	TAB_GOTO, TAB_GOTO_RELATIVE
* func: TAB_LEFT, TAB_LINK, TAB_MENU, TAB_RIGHT
* pre_form: ~/.w3m/pre_form
* rc: pre_form_file: pre_form configuration file

----------------------------------------------------------------

w3m 0.3.2.2 - 2002-12-06

* security fix: html_quote for img alt attributes

----------------------------------------------------------------

w3m 0.3.2.1 - 2002-11-27

* security fix: html_quote for frame contents
* backport from w3m 0.3.2+cvs
 - fix segmentation fault by large complex table.
	[w3m-dev 03371][w3m-dev 03438]

----------------------------------------------------------------
w3m 0.3.2 - 2002-11-05

* ~/.netrc: password for ftp
* rc: display_lineinfo: display current line number
* rc: passwd_file: passwd file for HTTP auth
* func: MARK_WORD
* rc: imgsize: obsoleted
* w3m-img for framebuffer merged

----------------------------------------------------------------
w3m 0.3.1 - 2002-07-16

* func: REINIT
	INIT_MAILCAP deleted, use REINIT MAILCAP instead
* func: DEFINE_KEY
* rc: keymap_file
* rc: use_dictcommand, dictcommand
* rc: mark_all_pages
* configure: -mandir
* func: COMMAND
* -title option: set buffer name to terminal title
* X-Face support: USE_XFACE, require uncompface

----------------------------------------------------------------
w3m 0.3 - 2002-03-06

* rc: mailer
	if mailer is set, it will be used for simple mailto: URLs
	otherwise, w3mmail.cgi will be used (when USE_W3MMAILER defined)
* rc: max_load_image
* rc: display_image, auto_image, image_scale, imgdisplay, imgsize
* func: DISPLAY_IMAGE, STOP_IMAGE
* w3m-img merged: w3m now can display images! see doc/README.img

----------------------------------------------------------------
w3m 0.2.5.1 - 2002-02-05

* backport from w3m/0.2.5+cvs-1.299
 - fix inputAnswer() and no "ssl_forbid_method" [w3m-dev 02985]
 - fix SunOS 4.1.4 build problem [w3m-dev 02972]
 - fix problem with Netscape-Enterprise WWW-authenticate [w3m-dev 02968]

----------------------------------------------------------------
w3m 0.2.5 - 2002-01-31

* RFC2617: HTTP Digest authentication
* rc: default_url=0(empty) 1(current URL) 2(link URL)
* GOTO_RELATIVE (M-u)
* highlight for incremental search
* support migemo (romaji search)
* use w3mmail.cgi for mailto: URL
* support external URI loader
* support -dump_extra ftp://
* new regex implementation

----------------------------------------------------------------
w3m 0.2.4 - 2002-01-07

* RFC2818 server identity check
* incremental search (C-s, C-r)

----------------------------------------------------------------
w3m 0.2.3.2 - 2001-12-22

* fix security hole in w3m/scripts

----------------------------------------------------------------
w3m 0.2.3.1 - 2001-12-20

* sync with cvs repository
* fix bug in configure

----------------------------------------------------------------
w3m 0.2.3 - 2001-12-20

* command line option: -help, -version
* new libgc included
* new runtime option use_mark, nextpage_topline, label_topline, vi_prec_num
   emacs_like_lineedit, ftppass_hostnamegen
* RFC2732 support (IPv6)
* new w3mhelp system
* several configure changes
* code cleanups, now gcc -Wall -Werror safe

----------------------------------------------------------------
w3m 0.2.2 - 2001-11-15

* sync with w3m 0.2.1-inu-1.5
* w3m maintained in sourceforge.net/projects/w3m

