2023-07-18  Rene Kita  <mail@rkta.de>

	* etc.c: Fix OOB access due to multiple backspaces.
	Origin: https://github.com/tats/w3m/pull/273
	Bug-Debian: https://github.com/tats/w3m/issues/268
	Bug-Debian: https://github.com/tats/w3m/issues/270 [CVE-2023-38252]
	Bug-Debian: https://github.com/tats/w3m/issues/271 [CVE-2023-38253]

2023-01-29  Markus Hiereth  <translation@hiereth.de>

	* po/de.po: Update German message catalogue.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1029933#5

2023-01-21  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS to 0.5.3+git20230121.

2023-01-15  Tatsuya Kinoshita  <tats@debian.org>

	* scripts/w3mman/w3mman2html.cgi.in:
	Add GROFF_NO_SGR=1 to w3mman2html.cgi for non-Debian groff.
	Bug-Debian: https://github.com/tats/w3m/pull/238
	Bug-Debian: https://github.com/tats/w3m/issues/201

	* scripts/w3mman/w3mman2html.cgi.in:
	Revert "Turn ansi escape sequences into html tags".
	This reverts commit 44af9271e0e984544762e2212549f134c86b4418.
	cf. https://github.com/tats/w3m/pull/238

2023-01-12  Tatsuya Kinoshita  <tats@debian.org>

	* fm.h, rc.c: Do not expand config value of tmp_dir.

	* config.h.dist, config.h.in, configure, configure.ac, rc.c:
	Use faccessat for rc_dir and tmp_dir.

	* local.c: Allow writeLocalCookie even when no_rc_dir.

	* main.c, rc.c: Call wtf_init in sync_with_option.

	* rc.c: Avoid modifying read-only rc_dir.

	* fm.h, main.c, proto.h, rc.c: Make tmp_dir if not found.

2023-01-09  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Prepare NEWS for w3m 0.5.3+git202301XX.

	* doc-de/FAQ.html, doc-jp/FAQ.html, doc/FAQ.html:
	Remove obsolete documents.

	* doc-de/FAQ.html, doc-de/MANUAL.html:
	Wrap long lines to avoid Lintian warnings.

2023-01-07  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Only read a first title.
	* file.c, fm.h: Revert "Only read title when in head".
	This reverts commit 0189e8aa5c4c4919a9bbc4dcbe0e521aada51e3c.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1020215

2023-01-06  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Indentation fix for HTMLtagproc1.

2023-01-06  Robert Alm Nilsson  <robert@robalni.org>

	* file.c, fm.h: Only read title when in head.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1020215

2023-01-06  Tatsuya Kinoshita  <tats@debian.org>

	* libwc/charset.c: Avoid locale sensitive tolower in wc_charset_to_ces.

2023-01-06  Sertaç Ö. Yıldız  <sertacyildiz@gmail.com>

	* libwc/charset.c:
	Fix charset declaration parser fails with turkish locale.
	Origin: https://bugzilla-attachments.redhat.com/attachment.cgi?id=160014
	Bug-Fedora: https://bugzilla.redhat.com/show_bug.cgi?id=249675

	* history.c: Use st_mtime instead of st_mtim.tv_sec to compile on macos.
	cf. https://github.com/tats/w3m/pull/247

2023-01-06  Rene Kita  <mail@rkta.de>

	* html.c, html.h, tagtable.tab: Recognize link targets in dfn elements.
	Refactor html.c.  Align in html.c.
	Origin: https://github.com/tats/w3m/pull/259
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1018696

	* Makefile.in, form.c, main.c, util.c, util.h:
	Handle failed system calls.
	* display.c, display.h, file.c, form.c, main.c, proto.h, terms.h:
	Move declarations to appropiate header files.
	Origin: https://github.com/tats/w3m/pull/257
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=398989

	* entity.js, etc.c, table.c, tests/allentity.expected:
	* tests/allentity.html: Skip soft hyphen when reading token.
	Fix generated HTML for entity test.
	Origin: https://github.com/tats/w3m/pull/256
	Bug-Debian: https://github.com/tats/w3m/issues/224
	Bug-Debian: https://github.com/tats/w3m/issues/258
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=830173

	* file.c: Check LESSOPEN to avoid undefined behaviour.
	Refactor lessopen_stream.
	Origin: https://github.com/tats/w3m/pull/254
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=991608

2023-01-05  Markus Hiereth  <translation@hiereth.de>

	* po/de.po: Update German message catalogue.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1011945#10

2023-01-05  Rene Kita  <mail@rkta.de>

	* buffer.c: Exit with error if a new buffer can't be allocated.
	Origin: https://git.sr.ht/~rkta/w3m/commit/1f88544c1a009ed2088ff20973bcfffe6cbcb5de
	Bug-Debian: https://github.com/tats/w3m/pull/232
	Bug-Debian: https://github.com/tats/w3m/pull/233

	* history.c, history.h:
	Merge history file if it was modified after start.
	* history.h, proto.h: Move declarations to the appropriate header file.
	* history.c: Add comment to explain placement of the ifdef.
	* history.c, proto.h: Let loadHistory return an error code.
	* history.c: Use 'goto fail' to remove code duplication.
	Origin: https://github.com/tats/w3m/pull/247
	Bug-Debian: https://github.com/tats/w3m/issues/176

2023-01-05  Alberto Fanjul  <albertofanjul@gmail.com>

	* scripts/w3mman/w3mman2html.cgi.in:
	Turn ansi escape sequences into html tags.
	Origin: https://github.com/tats/w3m/pull/238
	Bug-Debian: https://github.com/tats/w3m/issues/201

2023-01-04  Tatsuya Kinoshita  <tats@debian.org>

	* po/de.po, po/it.po, po/ja.po, po/sv_SE.po, po/w3m.pot, po/zh_CN.po:
	* po/zh_TW.po: Update PO strings.

	* doc/MANUAL.html, doc/README.img, libwc/wc_types.h, main.c, rc.c:
	English fixes.
	cf. https://github.com/tats/w3m/pull/241

2023-01-04  Rene Kita  <mail@rkta.de>

	* rc.c: Remove unused variable.
	* table.c: Remove a warning for bzero with GCC 12.
	* file.c: Fix potential null pointer dereference.
	* .github/workflows/build.yml:
	Don't error out on deprecated declaration warnings.
	Origin: https://github.com/tats/w3m/pull/255
	cf. https://github.com/tats/w3m/issues/252

2023-01-04  nico  <smnicolas@gmail.com>

	* doc/MANUAL.html, doc/w3m.1, fm.h, main.c, rc.c, terms.c:
	Add high-intensity colors option and cli flag.
	Origin: https://github.com/tats/w3m/pull/251
	cf. https://github.com/tats/w3m/issues/250
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=626291

2023-01-04  Trafficone  <trafficone@gmail.com>

	* doc/README.SSL, doc/README.keymap, doc/README.menu: Translate from
	doc-jp.
	* doc/README.cookie, doc/README.func, doc/README.img, doc/README.m17n:
	* doc/README.passwd: Clarified wording.  Minor grammar changes.
	Origin: https://github.com/tats/w3m/pull/241

2022-12-25  Tatsuya Kinoshita  <tats@debian.org>

	* configure: Update configure with acinclude.m4.

2022-12-25  Sam James  <sam@gentoo.org>

	* acinclude.m4: Fix configure tests broken with Clang 16.
	Origin: https://github.com/tats/w3m/pull/248

2022-12-25  Rin Okuyama  <rokuyama.rk@gmail.com>

	* image.c, terms.c:
	For sixel, no need to round image size to multiple of character size.
	Origin: https://github.com/tats/w3m/pull/246

	* image.c: Display resized image for OSC 5379 (mlterm).
	Origin: https://github.com/tats/w3m/pull/245

2022-12-25  Rene Kita  <mail@rkta.de>

	* doc/README.siteconf: Say what the comment character is.
	Use the comment character in Examples.
	Origin: https://github.com/tats/w3m/pull/237

	* main.c: Retry if loading of a file fails when argv_is_url.
	Origin: https://github.com/tats/w3m/pull/235
	Bug-Debian: https://github.com/tats/w3m/issues/210
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=537761
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=946440

2022-12-25  NRK  <nrk@disroot.org>

	* image.c: remove duplicate declaration.
	* cookie.c, entity.c, file.c, frame.c, func.c, image.c, linein.c:
	* mailcap.c, main.c, rc.c, rc.h, table.c, terms.c, terms.h:
	* w3mbookmark.c, w3mhelperpanel.c:
	fix all -Wmissing-prototypes warnings.
	* file.c, history.c, history.h, indep.c, indep.h, mailcap.c, proto.h:
	* rc.c, terms.c, url.c: fix some -Wstrict-prototypes warnings.
	Origin: https://github.com/tats/w3m/pull/234

2022-12-25  Rene Kita  <mail@rkta.de>

	* .github/workflows/build.yml:
	Add GitHub Action to build source when pushing.
	Origin: https://github.com/tats/w3m/pull/228

2022-12-21  Tatsuya Kinoshita  <tats@debian.org>

	* po/de.po, po/it.po, po/ja.po, po/sv_SE.po, po/w3m.pot, po/zh_CN.po:
	* po/zh_TW.po: Update PO strings.

2022-12-21  Rene Kita  <mail@rkta.de>

	* etc.c, fm.h, history.c, rc.c:
	Add option to set directory for temporary files.
	Origin: https://github.com/tats/w3m/pull/219
	cf. https://github.com/tats/w3m/issues/130

2022-12-21  Yash Lala  <yashlala@gmail.com>

	* rc.c: Use `Strnew_charp()` to create `char *` instead of `strdup()`.

	* rc.c:
	refactor: Substitute some clunky code with a `strdup()`.

	* doc/FAQ.html, doc/MANUAL.html, doc/w3m.1, rc.c:
	Set `rc_dir` based on `W3M_DIR` environment variable.
	Origin: https://github.com/tats/w3m/pull/207
	cf. https://github.com/tats/w3m/issues/130

2022-12-20  Tatsuya Kinoshita  <tats@debian.org>

	* etc.c: Fix potential overflow in checkType.

	* etc.c:
	Fix m17n backspace handling causes out-of-bounds write in checkType.
	[CVE-2022-38223]
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1019599
	Bug-Debian: https://github.com/tats/w3m/issues/242

2022-04-29  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS for w3m 0.5.3+git20220429.

	* config.guess, config.sub:
	Update config.* with autotools-dev 20220109.1.

2022-04-25  NRK  <nrk@disroot.org>

	Cppcheck fixes
	Origin: https://github.com/tats/w3m/pull/231
	* regex.c: check bound _before_ making access.
	* main.c: ensure map isn't NULL.
	* Str.c: properly close va_list.

2022-04-19  Rene Kita  <mail@rkta.de>

	Fix some more warnings
	Origin: https://github.com/tats/w3m/pull/230
	* w3mimg/x11/x11_w3mimg.c: Fix a warning about an unused variable.
	* libwc/wtf.c: Cast away a warning.

	Fix all warnings when building with -Wnull-dereference
	Origin: https://github.com/tats/w3m/pull/229
	* Makefile.in: Enable -Wnull-dereference by default.
	* main.c: Exit if we cannot allocate a new tab during start.
	* main.c: Fix potential null dereference.
	* file.c: Fix potential null dereference.
	* etc.c: Fix potential null dereference.
	* file.c, frame.c, ftp.c, news.c:
	Check return value of Str... functions.
	* w3mimg/fb/fb_imlib2.c: Fix potential null pointer dereference.
	* w3mimg/fb/fb.c: Check for NULL before dereferencing the pointer.
	* buffer.c: Do not call fclose() on a NULL pointer.
	* file.c: Check for NULL before dereferencing a pointer.
	* file.c: Check for NULL before dereferencing a pointer.
	* news.c: Remove null pointer dereference.

	* file.c: Fix broken anchor with link number at EOL.
	Origin: https://github.com/tats/w3m/pull/227

2022-04-15  Tatsuya Kinoshita  <tats@debian.org>

	* acinclude.m4, configure: Allow building without terminal library.
	This reverts commit 0d3416e0c250a4f08206967634fb641e9c8e008c.
	cf. https://github.com/tats/w3m/pull/221
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=46629

	* configure, configure.ac: Allow building without Perl.
	This reverts commit a0c8de3f3fe0178d655af54737e50168586f87a3.
	cf. https://github.com/tats/w3m/pull/221

	* acinclude.m4, configure: Do not reject FreeBSD framebuffer.
	This reverts commit cb3118b389b99eff001212c0d27a39f78c615a03.
	cf. https://github.com/tats/w3m/pull/221

2022-04-10  Tatsuya Kinoshita  <tats@debian.org>

	* proto.h: Remove unused encodeB declaration.

	* table.h: Remove unused MAX_WIDTH.

	* ChangeLog, gitlog2changelog: Prefer https for GNU URLs.

2022-04-09  Rene Kita  <mail@rkta.de>

	Fix all reported warnings when -Wall
	Origin: https://github.com/tats/w3m/pull/222
	Bug-Debian: https://github.com/tats/w3m/pull/216

	* symbol.c: Cast away a warning under OpenBSD 7.0.

	* Makefile.in: Enable -Wall by default.

	* image.c: Use unsigned int for image size.
	This removes a warning with -Wall.

	* terms.c: Use cast to suppress warning.

	* table.c: Initialize struct before use.

	* buffer.c: Fix a potential buffer overflow.
	When compiling with -Wformat-overflow=2 GCC reports:
	  note: 'sprintf' output between 16 and 35 bytes into a destination of size 32

	* etc.c, proto.h: Let base64_encode() take a char *.
	Throughout the whole code base only char * is passed, but a unsigned
	char * is expected. This leads to several warnings.
	Fix the interface and cast to unsigned char * internally to avoid any
	changes to the behaviour.

	* file.c: Explicitly cast to unsigned when passing to MD5().

	* terms.c: Let strncpy write the null terminator.

	* indep.c, indep.h: Take the correct char type in growbuf_append().
	This change removes all warnings (-Wall) from this function.

	* file.c: Fix warning for unused variable w/o MENU_SELECT.

	* display.c, etc.c, file.c, linein.c, table.c:
	Fix warning for unused variable without USE_M17N.

	* Str.c, file.c, image.c, istream.c, main.c, menu.c, news.c:
	Remove unused variable.

	* scrsize.c, w3mbookmark.c, w3mhelperpanel.c:
	Use main(void) when not taking arguments.

	* main.c, mktable.c, proto.h, w3mbookmark.c, w3mhelperpanel.c:
	Use standard conforming main() definition.

	* local.c, main.c: Suppress two warnings when compiling with tcc.
	While there, add some comments to better understand the code flow in
	localcgi_post().

	* Makefile.in: Move OPTS to end of CFLAGS.
	This allows the user to override default options.

2022-04-08  Rene Kita  <mail@rkta.de>

	Add some missing checks to configure
	Origin: https://github.com/tats/w3m/pull/221
	* acinclude.m4, configure: Check for linux/fb.h when configuring.
	* configure, configure.ac: Let configure fail if Perl is not in $PATH.
	* acinclude.m4, configure: Let configure fail if no terminal library
	is found.
	* .gitignore: Ignore autom4te cache directory.

	* posubst.in: Do not swallow errors from GCC.
	Use strict and warnings in posubst.
	Origin: https://github.com/tats/w3m/pull/220

2022-04-06  Tatsuya Kinoshita  <tats@debian.org>

	* po/it.po: Adjust spacing in menu strings.
	cf. https://github.com/tats/w3m/issues/225

	* po/zh_CN.po, po/zh_TW.po: Fix typo in menu strings.

2022-04-05  Markus Hiereth  <translation@hiereth.de>

	* po/de.po: Updated German message catalogue.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1003936#5

2022-04-05  Sebastian Rasmussen  <sebras@gmail.com>

	* po/LINGUAS, po/sv_SE.po: Add Swedish translation.
	Origin: https://github.com/tats/w3m/pull/209

2022-02-19  Rene Kita  <mail@rkta.de>

	* url.c: Check for end of string when parsing Gopher URLs.
	Origin: https://github.com/tats/w3m/pull/218
	Bug-Debian: https://github.com/tats/w3m/issues/199

2022-02-19  NRK  <nrk@disroot.org>

	* libwc/search.c: Fix wc_map_range_search() parameter type mismatch.
	Origin: https://github.com/tats/w3m/pull/214

2022-02-19  Kuang-che Wu  <kcwu@google.com>

	* fuzz/fuzz-conv.c: Improve fuzz-conv fuzzer.
	Origin: https://github.com/tats/w3m/pull/204

2022-02-16  Rene Kita  <mail@rkta.de>

	* acinclude.m4, configure: Do not use imlib2-config.
	imlib2-config was dropped by Imlib2:
	https://git.enlightenment.org/legacy/imlib2.git/commit/?id=e9d84bd2163e6fab494b5ce5cc8830a54ff97765
	Origin: https://github.com/tats/w3m/pull/215
	Bug-Debian: https://github.com/tats/w3m/issues/213

	* file.c: Ignore tokens that look like HTML, but are not.
	Origin: https://github.com/tats/w3m/pull/217
	Bug-Debian: https://github.com/tats/w3m/issues/200

2021-10-26  Kuang-che Wu  <kcwu@google.com>

	* istream.c: Fix StrStream memory leak.
	Origin: https://github.com/tats/w3m/pull/203

2021-10-26  bptato  <nincsnevem662@gmail.com>

	Fix stack overflow due to closing dd tags
	Origin: https://github.com/tats/w3m/pull/202
	Bug-Debian: https://github.com/tats/w3m/issues/198

	* file.c: Fix description title rendering (again).

	* file.c:
	Revert changes introducing #198 stack-overflow in HTMLlineproc0.

2021-10-26  Rene Kita  <mail@rkta.de>

	* table.c: Ensure VLA size is at least one (again).
	Origin: https://github.com/tats/w3m/pull/197

2021-09-18  Rene Kita  <mail@rkta.de>

	* istream.c: Fix manipulation of ASN1_STRING.
	Origin: https://github.com/tats/w3m/pull/193

2021-09-05  Rene Kita  <mail@rkta.de>

	* table.c: Ensure VLA size is at least one.
	Origin: https://github.com/tats/w3m/pull/192
	Bug-Debian: https://github.com/tats/w3m/issues/51

2021-08-30  Tatsuya Kinoshita  <tats@debian.org>

	* scripts/w3mman/w3mman.in: Fix incorrect query string for `w3mman 7z`.
	Bug-Debian: https://github.com/tats/w3m/issues/191

2021-08-19  Augusto Gunsch  <augustogunsch@tutanota.com>

	Update README.tab
	Origin: https://github.com/tats/w3m/pull/189

	* doc/README.tab: Fix wrong quote character.

	* doc/README.tab: Make spacing more consistent.

	* doc/README.tab: Update default tab navigation keybindings.

2021-07-17  Tatsuya Kinoshita  <tats@debian.org>

	* po/LINGUAS: Enable Italian translation.

	* po/it.po, po/ja.po: Update PO information.

2021-07-17  Marco Scardovi  <marco@scardovi.com>

	* po/it.po: Add italian translation.
	Origin: https://github.com/tats/w3m/pull/187

2021-07-03  Tatsuya Kinoshita  <tats@debian.org>

	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po:
	Update PO strings.

	* file.c, fm.h, rc.c, table.c:
	New option disable_center to disable center alignment.
	Bug-Debian: https://github.com/tats/w3m/issues/175
	Bug-Debian: https://github.com/tats/w3m/issues/185

2021-06-21  Tatsuya Kinoshita  <tats@debian.org>

	* doc/README.sixel: Add information of Debian's libsixel-bin package.
	Bug-Debian: https://github.com/tats/w3m/pull/184

2021-06-05  Laurenz  <git@laure.nz>

	* doc-jp/README, doc/README: Fix link to hboehm's gc library.
	Origin: https://github.com/tats/w3m/pull/183

2021-05-16  Tatsuya Kinoshita  <tats@debian.org>

	* fuzz/fuzz-conv.c:
	Prevent GC warnings of repeated allocation in fuzzer.

2021-05-16  bptato  <nincsnevem662@gmail.com>

	* terms.c: Fix null pointer dereference in put_image_kitty.
	Origin: https://github.com/tats/w3m/pull/182
	Bug-Debian: https://github.com/tats/w3m/issues/181

2021-04-24  Tatsuya Kinoshita  <tats@debian.org>

	* file.c, fm.h: Prevent integer overflow due to fontstat.

	* main.c: Check length of hostname with STR_SIZE_MAX.

2021-04-23  Tatsuya Kinoshita  <tats@debian.org>

	* etc.c, fm.h, local.c, main.c, proto.h, url.c:
	Treat 127.0.0.1, [::1], and hostname as localhost.

2021-04-17  Tatsuya Kinoshita  <tats@debian.org>

	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po:
	Update PO strings.

	* fm.h, rc.c, url.c:
	New option localhost_only to restrict connections only to localhost.
	Bug-Debian: https://github.com/tats/w3m/issues/117

2021-04-10  Tatsuya Kinoshita  <tats@debian.org>

	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po:
	Update PO strings.

	* fm.h, rc.c, url.c:
	New option cross_origin_referer to use origin only Referer.

	* main.c, proto.h, url.c: Don't include username in Referer.

	* main.c, url.c: Don't set Referer when data URI scheme.

2021-04-05  Tatsuya Kinoshita  <tats@debian.org>

	* etc.c: Use Strcatc and Strnulterm in base64_encode.

	* Str.h: New macros Strcatc and Strnulterm.

2021-04-05  bptato  <nincsnevem662@gmail.com>

	* etc.c, file.c, proto.h, terms.c: Return Str from base64_encode.
	Fix base64 padding could be applied incorrectly.
	Fix extraction of first gif frame for animations in put_image_kitty.
	Origin: https://github.com/tats/w3m/pull/177

2021-04-03  Tatsuya Kinoshita  <tats@debian.org>

	* file.c, table.c, table.h:
	Limit size and number of tables to prevent integer overflow.

2021-03-30  Tatsuya Kinoshita  <tats@debian.org>

	* table.c, table.h: Treat table height as int instead of short.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=838952

	* history.c, history.h, textlist.c, textlist.h:
	Treat textlist item number as int instead of short.
	cf. https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=838952

2021-03-29  Tatsuya Kinoshita  <tats@debian.org>

	* main.c: Prevent GC warnings of repeated allocation.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=746701
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=832407
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=862382

2021-03-27  Tatsuya Kinoshita  <tats@debian.org>

	* indep.c: Fix potential integer overflow in allocStr.

2021-03-26  Tatsuya Kinoshita  <tats@debian.org>

	* main.c: Ignore the "-" option to accept `w3m -` as "read from stdin".
	Bug-Debian: https://github.com/tats/w3m/issues/87

2021-03-25  Tatsuya Kinoshita  <tats@debian.org>

	* fuzz/fuzz-conv.c: Explicitly call GC_gcollect() in fuzzer.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=32453

	* fuzz/fuzz-conv.c: Prevent memory leak in fuzzer.

2021-03-23  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c: Reduce memory reallocation due to Strgrow.

	* libwc/ucs.c:
	Prevent unneeded memory allocation for language tags in libwc.
	cf. https://oss-fuzz.com/testcase-detail/6275874304425984
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31397

2021-03-22  Tatsuya Kinoshita  <tats@debian.org>

	* libwc/ucs.c: Prevent very long language tag in libwc.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=32229

2021-03-21  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c, Str.h: Check STR_SIZE_MAX in Strcat_char.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31804

2021-03-20  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c: Prevent very small allocation in Str.c.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31397

2021-03-19  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c: Decrease STR_SIZE_MAX to prevent large memory usage.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31468

2021-03-14  Tatsuya Kinoshita  <tats@debian.org>

	* fuzz/fuzz-conv.c:
	Include unistd.h for getpid() and unlink() in fuzzer.

	* fuzz/fuzz-conv.c: Call wtf_init() in fuzzing.

2021-03-11  Tatsuya Kinoshita  <tats@debian.org>

	* libwc/ucs.c, libwc/ucs.h:
	Prevent index overflow due to tag_map in libwc.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31936

2021-03-06  Tatsuya Kinoshita  <tats@debian.org>

	* fuzz/fuzz-conv.c: Set GC_oom_fn in fuzzing.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31500

	* fuzz/fuzz-conv.c: Call GC_INIT() in fuzzing.

2021-03-06  bptato  <nincsnevem662@gmail.com>

	* terms.c: Fix file handle leaks in kitty and iTerm2 image display.
	Origin: https://github.com/tats/w3m/pull/174

2021-03-04  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c: Prevent redundant memory reallocation in Str.c.

	* Str.c: Prevent unneeded Strgrow in Strinsert_char.

	* Str.c: Consider Strgrow overflow in Strinsert_char.

2021-03-03  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c: Prevent unneeded memory allocation in Strgrow.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31664

2021-03-02  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c: Prevent large memory usage and null-deref in Str.c.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31579
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31589

2021-03-02  bptato  <nincsnevem662@gmail.com>

	Support named character references specified by the living standard
	Origin: https://github.com/tats/w3m/pull/170

	* entity.js, tests/allentity.html:
	Use &gt; instead of &gt in entity test generator.

	* entity.js, tests/allentity.html:
	Fix small mistakes in entity test generator.

	* entity.js, entity.tab, tests/allentity.expected:
	* tests/allentity.html:
	Support single-codepoint HTML entities specified by whatwg.
	https://html.spec.whatwg.org/multipage/named-characters.html#named-character-references

	* entity.tab: Support period entity name.

2021-02-28  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c: Prevent zero size allocation in Str.c.

	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po:
	Update PO strings.

	* doc-jp/README.SSL, fm.h, rc.c, url.c:
	New option ssl_ca_default to explicitly use OpenSSL default paths.

	* libwc/utf7.c, libwc/utf8.c:
	Prevent unintentional integer overflow in libwc.

	* Str.c: Prevent unintentional integer overflow in Strcat_charp_n.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31500 (not yet fixed)

	* Str.c: Prevent unintentional integer overflow in Strgrow.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31467

2021-02-27  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c: One more fix overflow due to Strgrow.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31397 (not yet fixed)

2021-02-26  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c: Fix potential overflow due to Str.c.

	* Str.c: Fix integer overflow due to Strgrow.
	Bug-Chromium: https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=31397 (not yet fixed)

2021-02-24  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/README.SSL: Simplify doc for SSL.

2021-02-24  davkor  <david@adalogics.com>

	* fuzz/fuzz-conv.c:
	Added initial fuzzer for integration with OSS-Fuzz.
	Origin: https://github.com/tats/w3m/pull/169
	Bug-Debian: https://github.com/tats/w3m/issues/165

2021-02-22  Tatsuya Kinoshita  <tats@debian.org>

	* url.c: Don't fallback when SSL_CTX_load_verify_locations fails.

	* url.c:
	Don't use SSL_CTX_set_default_verify_paths when not USE_SSL_VERIFY.

	* acinclude.m4, config.h.dist, configure, doc-jp/README.SSL:
	Disable --with-cafile by default to use OpenSSL default paths.

2021-02-21  Tatsuya Kinoshita  <tats@debian.org>

	* main.c: Don't use SECLEVEL when not OPENSSL_TLS_SECURITY_LEVEL.

	* doc/w3m.1, main.c: Add eNULL to ssl_cipher when -insecure.

2021-02-21  bptato  <nincsnevem662@gmail.com>

	Improved iTerm2 image display + initial kitty image support
	Origin: https://github.com/tats/w3m/pull/168

	* image.c, rc.c, terms.c:
	Convert images to PNG for kitty with ImageMagick.

	* terms.c: Fix potential segfault.

	* terms.c: Fix small images on kitty.

	* etc.c, fm.h, image.c, rc.c, terms.c: Support kitty image protocol.

	* etc.c, terms.c: Handle iTerm2 images more efficiently.

2021-02-17  Tatsuya Kinoshita  <tats@debian.org>

	* libwc/iso2022.c, w3mimgdisplay.c: Typo fix.

2021-02-16  Tatsuya Kinoshita  <tats@debian.org>

	* acinclude.m4, configure:
	Add auto-detection for configure --with-migemo.

	* acinclude.m4, configure: Prefer Imlib2 over GTK2 by default.
	Bug-Debian: https://github.com/tats/w3m/issues/95
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=672121

	* acinclude.m4, configure:
	Indentation and wording fixes for configure --help.

2021-02-15  Tatsuya Kinoshita  <tats@debian.org>

	* acinclude.m4, configure: Wording fixes for configure --help.

	* acinclude.m4, configure:
	Accept configure --with-cafile without filename.

2021-02-14  bptato  <nincsnevem662@gmail.com>

	Improve description list rendering
	Origin: https://github.com/tats/w3m/pull/167
	Bug-Debian: https://github.com/tats/w3m/issues/162

	* html.c: Fix a mistake I made with </dd> and </dt> tags.

	* file.c, tests/dl.expected, tests/dl.html: Nested <dl>s.

	* file.c, tests/dl.expected, tests/dl.html: Fix <dl compact>.

	* tests/dl.expected, tests/dl.html: <dl> test.

	* file.c, html.c, html.h, tagtable.tab:
	Improve description list rendering.

2021-02-13  Tatsuya Kinoshita  <tats@debian.org>

	* acinclude.m4, configure: Minor fixes for ./configure --help.

	* acinclude.m4, config.h.dist, config.h.in, configure:
	* doc-jp/README.SSL, fm.h:
	Add auto-detection of ssl_ca_file by configure.
	New configure option --with-cafile to specify ssl_ca_file.
	cf. gnutls-trustfiles in Emacs 27.1
	<https://git.savannah.gnu.org/cgit/emacs.git/tree/lisp/net/gnutls.el?h=emacs-27.1#n106>

	* doc-jp/README.SSL: Doc fix.

	* url.c: Use ssl_ca_file and ssl_ca_path only when ssl_verify_server.

2021-02-12  Tatsuya Kinoshita  <tats@debian.org>

	* url.c: Check empty string conditions for ssl_ca_file and ssl_ca_path.

	* url.c:
	Fix OpenSSL default always overrides ssl_ca_file and ssl_ca_path.

2021-02-11  Tatsuya Kinoshita  <tats@debian.org>

	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po:
	Update PO strings.

	* doc/w3m.1, main.c: Update wording for -debug option.

	* doc/w3m.1: Update manpage for -insecure option.

	* url.c: Mention -insecure option as a workaround for SSL error.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=900984
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=934493
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=953045

	* main.c: New option -insecure to use insecure SSL config options.
	Alias for `-o ssl_cipher=ALL:@SECLEVEL=0 -o ssl_min_version=all
	-o ssl_forbid_method= -o ssl_verify_server=0`.

	* doc-jp/README.SSL: Typo fix for ssl_cipher.

	* doc-jp/README.SSL, rc.c, url.c:
	Update ssl_min_version to accept "all" and reject "SSLv2".

2021-02-11  bptato  <nincsnevem662@gmail.com>

	Support brotli content encoding
	Origin: https://github.com/tats/w3m/pull/164

	* config.h.dist: Update config.h.dist.

	* config.h.in, file.c, html.h: Support brotli content encoding.

2021-02-10  Tatsuya Kinoshita  <tats@debian.org>

	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po:
	Update PO strings.

	* doc-jp/README.SSL, fm.h: Disable TLSv1.0 and TLSv1.1 by default.

	* url.c: Mention ssl_* options as a workaround for SSL error.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=900984
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=934493
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=953045

	* doc-jp/README.SSL, fm.h, rc.c, url.c:
	New option ssl_min_version for OpenSSL 1.1.

	* doc-jp/README.SSL, fm.h, rc.c, url.c:
	New option ssl_cipher to specify ciphers for TLSv1.2 and below.
	e.g. DEFAULT:@SECLEVEL=2, ALL:@SECLEVEL=0

2021-02-06  Tatsuya Kinoshita  <tats@debian.org>

	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po:
	Update PO strings.

	* rc.c: Update option setting strings for inline image display method.

2021-02-06  bptato  <nincsnevem662@gmail.com>

	Improved inline image protocol support
	Origin: https://github.com/tats/w3m/pull/161

	* rc.c: Clarify inline image setting's wording.

	* etc.c: Use GC_MALLOC_ATOMIC_IGNORE_OFF_PAGE for allocating memory
	for base64 encoding.

	* etc.c, proto.h: base64_encode: fix input and output length types.

	* etc.c, file.c, proto.h: Some cleanup for base64_encode.

	* etc.c, file.c, fm.h, image.c, mimehead.c, proto.h, rc.c, terms.c:
	Support iTerm2 graphics protocol, replace encodeB with base64_encode.

	* image.c, rc.c: Avoid having external programs download images.

	* fm.h, image.c, main.c, rc.c:
	sixel and osc5379 image display protocols can be chosen in options.

2021-01-18  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Disable <section> behaves as <hr>.
	Because displaying horizontal rules are redundantly and unneeded,
	especially when sections are nested.

	* Makefile.in: Use xz with -9e for the dist target.

2021-01-03  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Indentation fix.

2021-01-02  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS for 0.5.3+git20210102.

2020-12-19  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/README.siteconf, doc/README.siteconf:
	Add examples of siteconf, set user_agent to Googlebot for Twitter.

2020-12-18  Tatsuya Kinoshita  <tats@debian.org>

	* po/zh_TW.po: Update PO-Revision-Date for zh_TW.

2020-12-18  Ambrose Li  <ambrose.li@gmail.com>

	* file.c: q_level was never initialized, causing random test failures.
	This should fix that.
	Origin: https://github.com/tats/w3m/pull/159

	* po/zh_TW.po: Translate new string.
	Origin: https://github.com/tats/w3m/pull/158

2020-12-17  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/README.siteconf, doc/README.siteconf:
	Update examples of siteconf, forward twitter.com to nitter.net.
	Because mobile.twitter.com without JavaScript is unusable anymore.

2020-12-16  Tatsuya Kinoshita  <tats@debian.org>

	* url.c: Use the default ciphers for OpenSSL 1.1 and later.
	cf. https://bugs.launchpad.net/ubuntu/+source/w3m/+bug/1325674
	    https://src.fedoraproject.org/rpms/w3m/blob/36f14df378762a3a03a6a724583ca5b0ff618ed5/f/Fix-the-cipher-list-string-to-ensure-that-it-contain.patch
	    https://fedoraproject.org/wiki/Packaging:CryptoPolicies

2020-12-09  bptato  <nincsnevem662@gmail.com>

	Small Gopher fixes/improvements
	Origin: https://github.com/tats/w3m/pull/157

	* file.c: Fix Gopher binaries causing w3m to be stuck in download mode;
	try to guess Gopher image type and fallback to png.

	* file.c, url.c: Add support for Gopher items 5 and I.

2020-11-24  Tatsuya Kinoshita  <tats@debian.org>

	* po/POTFILES.in, po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po:
	* po/zh_TW.po: Update PO.

	* configure: Update configure by autoconf 2.69.

	* config.h.dist, config.h.in, configure, configure.ac:
	Define X_DISPLAY_MISSING when configure --without-x for Imlib2.
	cf. https://github.com/NixOS/nixpkgs/commit/3cad8fba2958981307f94b865c2b970b95e10789

2020-11-22  Tatsuya Kinoshita  <tats@debian.org>

	* gitlog2changelog: Update example to use UTC for gitlog2changelog.

	* acinclude.m4, config.h.dist, config.h.in, configure, configure.ac:
	Drop HAVE_SYS_ERRLIST.

2020-11-22  Parag A Nemade  <pnemade@fedoraproject.org>

	* config.h.in:
	Fix compilation error "too few arguments to function 'longjmp'".
	Origin: https://src.fedoraproject.org/rpms/w3m/c/e7a12fa28cfbfbb0115ec74994092c1d3b8351d8?branch=master
	Bug-MacPorts: https://trac.macports.org/ticket/61356

2020-11-22  Parag Nemade  <pnemade@redhat.com>

	* etc.c, main.c: Fix FTBFS due to redefinition of sys_errlist.
	Origin: https://src.fedoraproject.org/rpms/w3m/c/99f30870caac12a3949b6736aa70b7233f4414d5?branch=master
	Bug-Fedora: https://bugzilla.redhat.com/show_bug.cgi?id=1038009
	Bug-MacPorts: https://trac.macports.org/ticket/61356

2020-11-15  Tatsuya Kinoshita  <tats@debian.org>

	* gitlog2changelog:
	Include gitlog2changelog to easily generate ChangeLog.

	* Makefile.in: Use xz instead of gzip for the dist target.

2020-11-12  Tatsuya Kinoshita  <tats@debian.org>

	* Makefile.in: Update the dist target to use git archive.

	* Bonus/README, Bonus/README.eng: Update examples for 2ch.cgi.

	* Bonus/2ch.cgi: Add charset=Shift_JIS to 2ch.cgi.

2020-11-11  Tatsuya Kinoshita  <tats@debian.org>

	* file.c, proto.h:
	Fix compilation errors when USE_GOPHER and not USE_M17N.

2020-11-11  bptato  <nincsnevem662@gmail.com>

	Support Gopher search and binary files.
	Origin: https://github.com/tats/w3m/pull/154

	* url.c: Remove useless loop.

	* file.c: Remove unnecessary file_unquote call.

	* url.c: Remove unnecessary variable assignment.

	* file.c, proto.h, url.c:
	Support Gopher items search (7) and binary file (9).

2020-10-24  Tatsuya Kinoshita  <tats@debian.org>

	* acinclude.m4, configure: Enable Gopher support by default.

2020-10-24  bptato  <nincsnevem662@gmail.com>

	Fix broken Gopher support.
	Origin: https://github.com/tats/w3m/pull/152
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=742455

	* file.c, url.c:
	Moved back filetype indicator to the beginning of file names.

	* file.c: Improved gopher directory display.

	* file.c, url.c: Improved gopher support.

2020-09-28  Tatsuya Kinoshita  <tats@debian.org>

	* scripts/w3mman/w3mman.in: Accept section "l" for w3mman.

2020-09-25  Tatsuya Kinoshita  <tats@debian.org>

	* scripts/w3mman/w3mman.in:
	Assume a local file if the argument contains slash for w3mman.

2020-09-25  Dustin Boyd  <memreflect@pm.me>

	* scripts/w3mman/w3mman2html.cgi.in: Remove -l flag in CGI script.
	Origin: https://bugs.freebsd.org/bugzilla/attachment.cgi?id=217947&action=diff
	Bug-FreeBSD: https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=249305

2020-09-15  Tatsuya Kinoshita  <tats@debian.org>

	* .cvsignore, libwc/.cvsignore, po/.cvsignore, scripts/.cvsignore:
	* scripts/multipart/.cvsignore, scripts/w3mman/.cvsignore:
	* w3mimg/.cvsignore, w3mimg/fb/.cvsignore, w3mimg/win/.cvsignore:
	* w3mimg/x11/.cvsignore: Remove .cvsignore.

	* .gitignore: Add .gitignore.

2020-09-15  Bruno Haible  <bruno@clisp.org>

	* url.c: Add support for file://hostname/... URLs.
	Origin: https://github.com/tats/w3m/files/3488813/file-hostname-support.diff.gz
	Bug-Debian: https://github.com/tats/w3m/issues/120

2020-09-06  Tatsuya Kinoshita  <tats@debian.org>

	* README, doc-jp/README, doc/README: Mention forked version.

	* configure, configure.ac, doc-jp/README, doc/README, po/Makevars:
	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po:
	Drop bug report address.

	* ChangeLog: Move old ChangeLog entries to ChangeLog.1.
	* ChangeLog.1: New file.

2020-09-02  bptato  <nincsnevem662@gmail.com>

	New option space_autocomplete.

	* linein.c:
	Add closing bracket I somehow forgot about in the previous commit.
	Origin: https://github.com/tats/w3m/pull/150

	* fm.h, linein.c, rc.c: Space is now entered in URL fields instead of
	triggering file completion, old behavior can be toggled via options.
	Origin: https://github.com/tats/w3m/pull/149

2020-09-01  Tatsuya Kinoshita  <tats@debian.org>

	* scripts/w3mhelp-funcdesc.ja.pl.in:
	Fix broken Japanese help page, convert to UTF-8.

	* doc-jp/README.func: Add CURSOR_* commands to Japanese README.func.

2020-08-31  bptato  <nincsnevem662@gmail.com>

	New commands CURSOR_TOP, CURSOR_MIDDLE and CURSOR_BOTTOM.
	Origin: https://github.com/tats/w3m/pull/148

	* main.c: Removed an unnecessary variable declaration.

	* scripts/w3mhelp.cgi.in: Added the other two commands to w3mhelp.

	* doc-de/README.func, doc/README.func, main.c, proto.h:
	* scripts/w3mhelp.cgi.in:
	New commands for moving to the top, middle and bottom of buffer.

2020-08-31  Tatsuya Kinoshita  <tats@debian.org>

	* w3m-doc/README.html, w3m-doc/community.html.in:
	* w3m-doc/configuration.html.in, w3m-doc/contain.wd:
	* w3m-doc/copyright.html.in, w3m-doc/define.wd, w3m-doc/detail.html.in:
	* w3m-doc/development.html.in, w3m-doc/faq.html.in:
	* w3m-doc/function.html.in, w3m-doc/index.html.in:
	* w3m-doc/install.html.in, w3m-doc/mkdocs, w3m-doc/operation.html.in:
	* w3m-doc/outline.html.in, w3m-doc/prologue.html.in:
	* w3m-doc/sample/README, w3m-doc/sample/define.wd:
	* w3m-doc/sample/html.wd, w3m-doc/sample/keymap.cgi:
	* w3m-doc/sample/s.wd, w3m-doc/sample/sample.html:
	* w3m-doc/sample/sample.wd, w3m-doc/sample/w3mdoc.pl:
	* w3m-doc/w3mdoc.pl: Drop obsolete w3m-doc.

2020-08-29  Ambrose Li  <ambrose.li@gmail.com>

	* file.c, html.c, html.h, tagtable.tab:
	Rudimentary support for the section tag
	Origin: https://github.com/tats/w3m/pull/147

	* file.c:
	Somehow the wrong quotes were used. This should fix the failing tests.
	Origin: https://github.com/tats/w3m/pull/139/commits/b9488ffe60963349bf622a7548e3b9dccc6e0728

	* po/zh_TW.po: Missed the spurious (_S).
	Origin: https://github.com/tats/w3m/pull/145/commits/5d7fb3719e1308d56e5505ab67160b6d8fae34b0

2020-08-24  Ambrose Li  <ambrose.li@gmail.com>

	* etc.c, file.c, fm.h, html.c, html.h, tests/a1.expected:
	* tests/a1.html, tests/a2.expected, tests/a2.html, tests/run_tests:
	Make w3m's handling of the a element HTML5 compatible (when the stream
	is HTML5).
	In HTML5 anchors should not be closed when encountering divs, for
	example, but should be closed when encountering buttons, for example.
	This also fixes a bug in the tokenizing FSM in etc.c that prevented
	the !doctype element from being recognized; the fix is necessary
	because HTML5 detection dependson checking the !doctype element.
	Origin: https://github.com/tats/w3m/pull/146
	Bug: https://sourceforge.net/p/w3m/patches/74/
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=290460
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=847875

	* po/zh_TW.po: Corrections to traditional Chinese translation,
	including corrections of a number of serious errors.
	Origin: https://github.com/tats/w3m/pull/145

2020-08-23  Ambrose Li  <ambrose.li@gmail.com>

	* entity.c, file.c, fm.h, tests/name_entity_1.expected:
	* tests/name_entity_1.html, tests/name_entity_1.opts:
	* tests/name_entity_2.expected, tests/name_entity_2.html:
	* tests/q1.expected, tests/q1.html, tests/q1.opts, tests/q2.expected:
	* tests/q2.html, tests/q3.expected, tests/q3.html, tests/q3.opts:
	* tests/q4.expected, tests/q4.html, tests/q4.opts, tests/q5.expected:
	* tests/q5.html, tests/q6.expected, tests/q6.html, tests/q6.opts:
	* tests/run_tests: Changes the behaviour of the q tag (when m17n and
	Unicode are configured) to use "smart" quotes if the display charset
	can handle them.  Falls back to old behaviour (ASCII quotes with
	left/right quote semantics for 6/0 and 2/6) if display charset is
	us-ascii.  Also changes the behaviour of conv_entity() to convert
	left/right quotes and some dashes because named entities are needed
	for the new code for the q tag.
	Origin: https://github.com/tats/w3m/pull/139

2020-08-23  Tatsuya Kinoshita  <tats@debian.org>

	* html.c: Add TFLG_END to "/sup", "/sub" and "/figure" for TagMAP.

2020-08-21  Ambrose Li  <ambrose.li@gmail.com>

	* file.c, html.c, html.h, tagtable.tab:
	Rudimentary support for figure, figcaption.
	Origin: https://github.com/tats/w3m/pull/136

2020-08-02  David Spickett  <david.spickett@linaro.org>

	* scripts/w3mhelp.cgi.in:
	Show keyboard shortcuts in a consistent order in help.
	Origin: https://github.com/tats/w3m/pull/134
	Bug-Debian: https://github.com/tats/w3m/issues/133

2020-07-11  Bjarni Ingi Gislason  <bjarniig@rhi.hi.is>

	* doc/w3m.1: Fix some source formatting in the manual.
	- Begin a sentence on a new line.
	- Split long lines (> 80).
	- Fix warnings from "mandoc -Tlint".
	- Remove space at end of lines.
	- Change a HYPHEN-MINUS (code 0x55, 2D) to a dash (minus) if it matches
	  " -[:alpha:]" or \[aq]-[:alpha:] (for options).
	- Use the macros .MT/.ME for e-mail addresses.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=963801#5

2020-05-07  Shun Sakai  <sorairolake@protonmail.ch>

	* Bonus/README, doc-jp/FAQ.html, doc-jp/HISTORY, doc-jp/MANUAL.html:
	* doc-jp/README, doc-jp/README.SSL, doc-jp/README.cookie:
	* doc-jp/README.cygwin, doc-jp/README.dict, doc-jp/README.func:
	* doc-jp/README.img, doc-jp/README.keymap, doc-jp/README.m17n:
	* doc-jp/README.mailcap, doc-jp/README.menu, doc-jp/README.migemo:
	* doc-jp/README.mouse, doc-jp/README.passwd, doc-jp/README.pre_form:
	* doc-jp/README.siteconf, doc-jp/README.tab, doc-jp/STORY.html:
	* doc-jp/menu.default, doc-jp/menu.submenu, doc-jp/w3m.1:
	* scripts/bm2menu/README, scripts/multipart/README:
	* scripts/w3mman/README, w3m-doc/README.html:
	* w3m-doc/community.html.in, w3m-doc/configuration.html.in:
	* w3m-doc/copyright.html.in, w3m-doc/detail.html.in:
	* w3m-doc/development.html.in, w3m-doc/faq.html.in:
	* w3m-doc/function.html.in, w3m-doc/index.html.in:
	* w3m-doc/install.html.in, w3m-doc/operation.html.in:
	* w3m-doc/outline.html.in, w3m-doc/prologue.html.in:
	* w3m-doc/sample/README, w3m-doc/sample/define.wd, w3m-doc/sample/s.wd:
	* w3m-doc/sample/sample.html, w3m-doc/sample/sample.wd:
	* w3mhelp-lynx_ja.html.in, w3mhelp-w3m_ja.html.in:
	* w3mimg/fb/readme.txt:
	Change the encoding of the Japanese docs to UTF-8.

	* COPYING: Add COPYING file.

2020-05-02  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS for 0.5.3+git20200502.

2020-03-27  Roland Illig  <rillig@NetBSD.org>

	* main.c: Fix -Wchar-subscripts.
	Origin: http://cvsweb.netbsd.org/bsdweb.cgi/~checkout~/pkgsrc/www/w3m/patches/patch-main.c?rev=1.1&content-type=text/plain
	Bug: https://sourceforge.net/p/w3m/patches/76/

2020-03-16  Tatsuya Kinoshita  <tats@debian.org>

	* doc-de/MANUAL.html, doc/MANUAL.html: Update documents for GOTO_HOME.

	* doc-de/README.func, doc-jp/README.func, doc-jp/keymap.default:
	* doc/README.func, doc/keymap.default, scripts/w3mhelp.cgi.in:
	Add GOTO_HOME to the help page.

2020-03-11  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/README.SSL, po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po:
	* po/zh_TW.po, rc.c: Update documents for ssl_forbid_method.

	* url.c: Extend ssl_forbid_method for TLSv1.2 and TLSv1.3.

2020-01-13  We're Yet  <58348703+butwerenotthereyet@users.noreply.github.com>

	* keybind.c, main.c, proto.h: Add command to go home.
	Origin: https://github.com/tats/w3m/pull/124

2019-11-10  Kyle J. McKay  <mackyle@gmail.com>

	* entity.tab, indep.c, indep.h: Support &apos; entity.
	Origin: https://github.com/tats/w3m/pull/122
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=927409

2019-07-02  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/README.siteconf: Update doc-jp for user_agent in siteconf.

2019-07-01  Azure  <azure@fox.blue>

	* doc/README.siteconf, fm.h, rc.c, url.c:
	Allow setting User Agent in Siteconf.
	Origin: https://github.com/tats/w3m/pull/119

2019-04-21  Laurent Arnoud  <laurent@spkdev.net>

	* fm.h, main.c, url.c: Allow to override User-Agent with -header.
	Origin: https://github.com/tats/w3m/pull/113

2019-01-05  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS.

	* config.guess, config.sub:
	Update config.* with autotools-dev 20180224.1.

2019-01-05  Akinori Hattori  <hattya@gentoo.org>

	* w3mimg/fb/fb_imlib2.c: Fix dependency for Imlib2.
	Bug-Gentoo: https://bugs.gentoo.org/605930
	Origin: https://gitweb.gentoo.org/repo/gentoo.git/commit/?id=97d6e3e6839898829e8cce211b97a7fa77f5d06e

2018-12-22  Tatsuya Kinoshita  <tats@debian.org>

	* scripts/w3mman/w3mman.1.in, scripts/w3mman/w3mman.in:
	Fix square brackets.

2018-12-21  Nemo Inis  <nemoinis@hotmail.com>

	* scripts/w3mman/w3mman.1.in, scripts/w3mman/w3mman.in:
	* scripts/w3mman/w3mman2html.cgi.in:
	w3mman support for section number during keyword search.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=916968#5

2018-10-26  Ben Wong  <bugs.debian.org@wongs.net>

	* buffer.c, display.c:
	Fix that the MarkAllPages option works as originally intended.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=911929

2018-10-26  Tatsuya Kinoshita  <tats@debian.org>

	* istream.c, url.c: Indentation fix.

2018-10-24  Mark Wright  <gienah@gentoo.org>

	* istream.c, url.c: Do not use deprecated features with openssl-1.1.
	Bug-Gentoo: https://bugs.gentoo.org/592510
	Bug-Debian: https://github.com/tats/w3m/pull/103

2018-05-20  Andrew Santosa  <santosa_1999@yahoo.com>

	* po/Makefile.in.in: Added check for : command not producing .gmo file.
	Bug-Debian: https://github.com/tats/w3m/pull/99

2018-03-24  Tatsuya Kinoshita  <tats@debian.org>

	* table.c: Respect simple_preserve_space for table cells.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=893902

2018-03-23  Mike  <barbeque@users.noreply.github.com>

	Update grammar & spelling in some English READMEs.
	Origin: https://github.com/tats/w3m/pull/97

	* doc/README.img: Update README.img.
	* doc/README.dict: Update README.dict.
	* doc/README.pre_form: Update README.pre_form.
	* doc/README.cookie: Update README.cookie.
	* doc/README.cookie: Update README.cookie.

2018-03-04  Jia Zhouyang  <jiazhouyang@nudt.edu.cn>

	Fix crashes when some external APIs fail.
	Origin: https://github.com/tats/w3m/pull/96

	* url.c: Add error handling code for fopen.
	Check the return code of fopen, and return when it fails.
	* file.c: Add error handling code for fopen.
	Check the return value of fopen, and add proper error handling code.
	* local.c: Add error handling for chdir.
	When chdir fails, print error message and exit.

2018-01-25  Tatsuya Kinoshita  <tats@debian.org>

	* ChangeLog, NEWS: Add CVE IDs.
	cf. https://security-tracker.debian.org/tracker/source-package/w3m

2018-01-21  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS.

	* scripts/Makefile.in: Do not remove w3mdict.cgi when "make distclean".

	* config.h.dist, config.h.in, configure, configure.ac, main.c, rc.c:
	Make temporary directory safely when ~/.w3m is unwritable.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=888097
	[CVE-2018-6198]

	* rc.c: Suppress error messages when ~/.w3m is unwritable.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=871425

2018-01-20  Tatsuya Kinoshita  <tats@debian.org>

	* config.guess, config.sub:
	Update config.* with autotools-dev 20171216.1.

	* table.c: Prevent negative indent value in feed_table_block_tag().
	Bug-Debian: https://github.com/tats/w3m/issues/88 [CVE-2018-6196]

2018-01-06  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/README.SSL: Doc fix for ssl_forbid_method.

	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po, rc.c:
	* url.c: Fix multi-character character constant for ssl_forbid_method.

2018-01-06  se  <se@example.com>

	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po, rc.c:
	* url.c: Extend ssl_forbid_method to disable TLSv1.1.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=874218#5

2018-01-06  Tatsuya Kinoshita  <tats@debian.org>

	* w3mimg/fb/fb_w3mimg.c: Accept TERM=fbterm.
	cf. https://bushowhige.blogspot.jp/2015/01/fbterm-w3m-img.html
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=590668

2017-12-27  Tatsuya Kinoshita  <tats@debian.org>

	* form.c: Prevent invalid columnPos() call in formUpdateBuffer().
	Bug-Debian: https://github.com/tats/w3m/issues/89 [CVE-2018-6197]

	* main.c: Typo fix in fusage().
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=878106

	* doc-jp/README.tab, doc/README.tab, main.c: English fix.
	cf. [emacs-w3m:12706], http://emacs-w3m.namazu.org/ml/msg12598.html

2017-12-24  HIGUCHI Daisuke (VDR dai)  <dai@debian.org>

	* scripts/w3mman/w3mman.1.in, terms.c: Fix spelling error.

2017-08-27  Kyle J. McKay  <mackyle@gmail.com>

	Correct <base ...> parsing and do not turn a form's GET into POST.
	Bug-Debian: https://github.com/tats/w3m/pull/93

	* form.c:
	form.c: do not gratuitously turn GET into POST.
	When encountering a <form ...> tag that contains these values:
	    method="get" enctype="multipart/form-data"
	Do not transform the method into POST to accomodate enctype.
	Instead behave in the compatible way that all other browsers
	behave in this instance and ignore the enctype parameter
	(treating it as the default application/x-www-form-urlencoded)
	and perform a "GET" just as the method parameter requests.
	This behavior produces far more compatible results than
	gratuitously changing the "get" into a "post" which can
	result in unexpected "405 Method Not Allowed" errors.
	Signed-off-by: Kyle J. McKay <mackyle@gmail.com>

	* file.c:
	file.c: compute correct base URL when not absolute.
	When a server makes use of the PATH_INFO feature in a CGI,
	the returned pages may often have a <base href="..." /> tag
	specifying the URL of the CGI itself as the base.
	However, to avoid hard-coding the scheme and host into such
	a base href, the href value will often omit the scheme, host
	and port.
	Make sure that when parsing any such base href value that
	any omitted components are taken from the current URL rather
	than taken as being from a bare, absolute file:/// URL.
	Signed-off-by: Kyle J. McKay <mackyle@gmail.com>

2017-01-02  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS.

2016-12-24  Tatsuya Kinoshita  <tats@debian.org>

	* libwc/wtf.c:
	Prevent overflow beyond the end of string in wtf_parse1().
	Bug-Debian: https://github.com/tats/w3m/issues/68

	* libwc/wtf.c:
	Revert "Prevent overflow beyond the end of string in wtf_parse1()".
	This reverts commit 998b6f91d4b02e8bf90b7744dfabc8cccdf9d4f9.
	Bug-Debian: https://github.com/tats/w3m/issues/86
	cf. http://emacs-w3m.namazu.org/ml/msg12505.html

	* form.c:
	Preserve one byte for end of string character in form_update_line().
	Bug-Debian: https://github.com/tats/w3m/issues/68#issuecomment-266214643

	* form.c:
	Prevent invalid form_update_line() call in formUpdateBuffer().
	Bug-Debian: https://github.com/tats/w3m/issues/82

2016-12-20  Tatsuya Kinoshita  <tats@debian.org>

	* form.c:
	Revert "Preserve one byte for end of string character in form_update_line()".
	This reverts commit a4152aaaea5cb51c9018880a1295e498c38889bf.

2016-12-18  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Prevent heap-use-after-free read in HTMLlineproc0().
	Bug-Debian: https://github.com/tats/w3m/issues/81

	* file.c: Prevent infinite loop in feed_textarea().
	Bug-Debian: https://github.com/tats/w3m/issues/85

	* form.c:
	Revert "Prevent overflow beyond the end of string in form_update_line()".
	This reverts commit 9ccaa1dd0dac6f9b35a649ae9901c225421500f6.

	* form.c:
	Revert "Prevent overflow beyond the end of string in form_update_line()".
	This reverts commit e0efc127ff20cbeb931847af1c9b353056340fbd.

2016-12-15  Tatsuya Kinoshita  <tats@debian.org>

	* libwc/wtf.c:
	Prevent overflow beyond the end of string for wtf to wcs macros.
	Bug-Debian: https://github.com/tats/w3m/issues/77

	* libwc/wtf.c:
	Revert "Prevent overflow beyond the end of string for wtf to wcs macros".
	This reverts commit b4d27ba5ccffaa38e968c2bf3a8eeb9cd43928ff.

	* file.c, libwc/wtf.c, libwc/wtf.h:
	Prevent overflow beyond the end of string in caller of get_mclen().
	Bug-Debian: https://github.com/tats/w3m/issues/59
	Bug-Debian: https://github.com/tats/w3m/issues/73
	Bug-Debian: https://github.com/tats/w3m/issues/74
	Bug-Debian: https://github.com/tats/w3m/issues/75
	Bug-Debian: https://github.com/tats/w3m/issues/76
	Bug-Debian: https://github.com/tats/w3m/issues/78
	Bug-Debian: https://github.com/tats/w3m/issues/79
	Bug-Debian: https://github.com/tats/w3m/issues/80
	Bug-Debian: https://github.com/tats/w3m/issues/83
	Bug-Debian: https://github.com/tats/w3m/issues/84

	* file.c:
	Revert "Prevent overflow beyond the end of string in proc_mchar()".
	This reverts commit 512ed467d12615f5ef40d0d28272e5662d8438ea.

	* table.c:
	Revert "Prevent overflow beyond the end of string in visible_length()".
	This reverts commit a932f78a6d8c105036ffeedf01215c1f6a0e0b71.

	* table.c:
	Revert "Prevent overflow beyond the end of string in skip_space()".
	This reverts commit e757b43bcf8c439c167f62b6d3317ee9518cabbf.

	* table.c:
	Revert "Prevent overflow beyond the end of string in visible_length_plain()".
	This reverts commit f763b8ebf5441cb44d2c0234565fadd5eb1c87a5.

	* form.c:
	Revert "Prevent overflow beyond the end of string in textfieldrep()".
	This reverts commit 77d8d8d6576d8afc0f6b2e09bb88c7ca9dba58bb.

	* file.c:
	Revert "Prevent overflow beyond the end of string in proc_mchar()".
	This reverts commit e79d0ec2a00369a6af24007a1f2bb5e876e2c847.

2016-12-13  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Prevent overflow beyond the end of string in proc_mchar().
	Bug-Debian: https://github.com/tats/w3m/issues/80
	cf. https://github.com/tats/w3m/issues/59

	* form.c: Prevent overflow beyond the end of string in textfieldrep().
	Bug-Debian: https://github.com/tats/w3m/issues/79

	* form.c:
	Preserve one byte for end of string character in form_update_line().
	Bug-Debian: https://github.com/tats/w3m/issues/82
	cf. https://github.com/tats/w3m/issues/68#issuecomment-266214643

2016-12-10  Tatsuya Kinoshita  <tats@debian.org>

	* libwc/wtf.c: Prevent overflow beyond the end of string in wtf_len().
	cf. https://github.com/tats/w3m/issues/57

	* etc.c: Prevent negative array index for realColumn in calcPosition().
	Bug-Debian: https://github.com/tats/w3m/issues/69

	* libwc/wtf.c:
	Prevent overflow beyond the end of string in wtf_parse1().
	Bug-Debian: https://github.com/tats/w3m/issues/68

	* Str.c: Prevent heap-buffer-overflow in Strnew_size().
	Bug-Debian: https://github.com/tats/w3m/issues/72

	* table.c:
	Prevent overflow beyond the end of string in visible_length_plain().
	Bug-Debian: https://github.com/tats/w3m/issues/76

	* libwc/wtf.c:
	Prevent overflow beyond the end of string for wtf to wcs macros.
	Bug-Debian: https://github.com/tats/w3m/issues/77

	* form.c:
	Prevent overflow beyond the end of string in form_update_line().
	Bug-Debian: https://github.com/tats/w3m/issues/78

2016-12-08  Tatsuya Kinoshita  <tats@debian.org>

	* form.c:
	Prevent overflow beyond the end of string in form_update_line().
	Bug-Debian: https://github.com/tats/w3m/issues/75

	* table.c: Prevent overflow beyond the end of string in skip_space().
	Bug-Debian: https://github.com/tats/w3m/issues/74

	* table.c:
	Prevent overflow beyond the end of string in visible_length().
	Bug-Debian: https://github.com/tats/w3m/issues/73

	* libwc/wtf.c:
	Prevent overflow beyond the end of string in wtf_strwidth().
	Bug-Debian: https://github.com/tats/w3m/issues/57

	* libwc/wtf.c:
	Revert "Prevent overflow beyond the end of string in wtf_strwidth()".
	This reverts commit d345c0950dfdef065b7377ecad0e4bc1d2601bf8.

2016-12-07  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Prevent heap-use-after-free in HTMLlineproc0().
	Bug-Debian: https://github.com/tats/w3m/issues/65

	* file.c: Prevent negative values for offset and pos in push_link().
	Bug-Debian: https://github.com/tats/w3m/issues/64

	* file.c: Prevent overflow beyond the end of string in proc_mchar().
	Bug-Debian: https://github.com/tats/w3m/issues/59

	* libwc/wtf.c:
	Prevent overflow beyond the end of string in wtf_strwidth().
	Bug-Debian: https://github.com/tats/w3m/issues/57

2016-12-05  Yixun Lan  <dlan@gentoo.org>

	* html.h: Explictily include <time.h> to avoid build err.
	While disable ssl, we will got a undefine time_t err.
	Bug-Gentoo: https://bugs.gentoo.org/show_bug.cgi?id=601498
	Origin: https://gitweb.gentoo.org/repo/gentoo.git/commit/?id=8ee43ba4e036db70fff258f3edb2f0335385e93f

2016-12-05  Tatsuya Kinoshita  <tats@debian.org>

	* table.c:
	Prevent array index out of bounds for tridvalue in feed_table_tag().
	Bug-Debian: https://github.com/tats/w3m/issues/71

	* table.c: Prevent negative array index in set_integered_width().
	Bug-Debian: https://github.com/tats/w3m/issues/70

	* table.c:
	Prevent array index out of bounds for tabattr in feed_table_tag().
	Bug-Debian: https://github.com/tats/w3m/issues/60

	* file.c: Prevent negative array index in process_textarea().
	Bug-Debian: https://github.com/tats/w3m/issues/58

	* file.c:
	Prevent negative array index for marks in HTMLlineproc2body().
	Bug-Debian: https://github.com/tats/w3m/issues/61

	* file.c:
	Prevent negative value of row for pushTable() in HTMLlineproc0().
	Bug-Debian: https://github.com/tats/w3m/issues/67

	* file.c: Prevent negative array index in getMetaRefreshParam().
	Bug-Debian: https://github.com/tats/w3m/issues/63

	* anchor.c:
	Prevent negative array index for marks in shiftAnchorPosition().
	Bug-Debian: https://github.com/tats/w3m/issues/62

2016-11-27  Kuang-che Wu  <kcwu@google.com>

	* file.c: Fix uninitialized variable in process_img(). fix #44.
	Bug-Debian: https://github.com/tats/w3m/issues/44
	Origin: https://github.com/tats/w3m/pull/50/commits/41a607b06e4475101de59e5c623b9e5f76594a21

	* menu.c: Fix menu buffer-overflow.
	Origin: https://github.com/tats/w3m/pull/49/commits/7e1c05dd90cf42a308e854881ea3813aed000d2e

2016-11-27  Tatsuya Kinoshita  <tats@debian.org>

	* ChangeLog, NEWS: Add CVE IDs.
	cf. https://security-tracker.debian.org/tracker/source-package/w3m
	    http://www.openwall.com/lists/oss-security/2016/11/24/1

2016-11-20  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS.

2016-11-19  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS.

2016-11-18  Tatsuya Kinoshita  <tats@debian.org>

	* ChangeLog, NEWS: Add CVE IDs.
	cf. https://security-tracker.debian.org/tracker/source-package/w3m
	    http://www.openwall.com/lists/oss-security/2016/11/18/3

	* libwc/ucs.map: Fix type mismatch for pcsw_ucs_map_size.
	cf. https://github.com/tats/w3m/issues/43

	* libwc/ucs.c, libwc/ucs.map:
	Prevent global-buffer-overflow in wc_any_to_ucs().
	Bug-Debian: https://github.com/tats/w3m/issues/43 [CVE-2016-9632]

2016-11-17  Tatsuya Kinoshita  <tats@debian.org>

	* url.c: Prevent global-buffer-overflow in parseURL().
	Bug-Debian: https://github.com/tats/w3m/issues/41 [CVE-2016-9630]

	* file.c: Prevent deref null pointer in HTMLlineproc0().
	Bug-Debian: https://github.com/tats/w3m/issues/42 [CVE-2016-9631]

2016-11-15  Tatsuya Kinoshita  <tats@debian.org>

	* table.c: Prevent deref null pointer in renderCoTable().
	Bug-Debian: https://github.com/tats/w3m/issues/20#issuecomment-260649537

	* file.c, proto.h, table.c:
	Prevent infinite recursion with nested table and textarea.
	Bug-Debian: https://github.com/tats/w3m/issues/20#issuecomment-260590257
	[CVE-2016-9439]

	* table.c:
	Revert "Prevent infinite recursion with nested table and textarea".
	This reverts commit f393faf55975a94217df479e1bd06ee4403c6958.

	* anchor.c: Prevent deref null pointer in shiftAnchorPosition().
	Bug-Debian: https://github.com/tats/w3m/issues/40 [CVE-2016-9629]

2016-11-14  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Prevent null pointer deref due to bad form id.
	Bug-Debian: https://github.com/tats/w3m/issues/39 [CVE-2016-9628]

	* display.c, file.c, fm.h, symbol.c:
	Prevent array index out of bounds for symbol.
	Bug-Debian: https://github.com/tats/w3m/issues/38 [CVE-2016-9627]

2016-11-13  Tatsuya Kinoshita  <tats@debian.org>

	* file.c:
	Prevent null pointer dereference in HTMLlineproc2body for textarea_int.
	Bug-Debian: https://github.com/tats/w3m/issues/32#issuecomment-260170163

2016-11-12  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS.

	* table.c: Prevent infinite recursion with nested table and textarea.
	Bug-Debian: https://github.com/tats/w3m/issues/20

2016-11-09  Tatsuya Kinoshita  <tats@debian.org>

	* table.c: Check indent_level to prevent infinite recursion.
	Bug-Debian: https://github.com/tats/w3m/issues/37 [CVE-2016-9626]

2016-11-07  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Prevent infinite recursion in HTMLlineproc0.
	Bug-Debian: https://github.com/tats/w3m/issues/36 [CVE-2016-9625]

	* NEWS, w3m-doc/install.html.in:
	Update documents for included w3mdict.cgi.

2016-11-07  ITOH Yasufumi  <itohy@NetBSD.org>

	* main.c: Fix suspend (^Z) behavior.
	Suspend the job w3m belongs to, not w3m only.
	Signed-off-by: Thomas Klausner <wiz@NetBSD.org>
	Bug-Debian: https://github.com/tats/w3m/pull/34
	Origin: http://cvsweb.netbsd.org/bsdweb.cgi/pkgsrc/www/w3m/patches/patch-ab?rev=1.4&content-type=text/x-cvsweb-markup

2016-11-07  Tatsuya Kinoshita  <tats@debian.org>

	* form.c: Prevent dereference near-null pointer in formUpdateBuffer.
	Bug-Debian: https://github.com/tats/w3m/issues/35 [CVE-2016-9624]

	* file.c: Prevent crash after allocate string of negative size.
	Bug-Debian: https://github.com/tats/w3m/issues/33 [CVE-2016-9623]

	* file.c: Prevent memory exhausted due to repeat appending "</table>".
	Bug-Debian: https://github.com/tats/w3m/issues/23 [CVE-2016-9633]

	* file.c: Prevent null pointer dereference in HTMLlineproc2body.
	Bug-Debian: https://github.com/tats/w3m/issues/32 [CVE-2016-9622]

2016-10-31  Tatsuya Kinoshita  <tats@debian.org>

	* table.c, table.h, textlist.h:
	Revert "Treat table height as int instead of short".
	This reverts commit 0c9aebb26a16ad3acc69b2e87ffd216d43879cb6.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=842623

2016-10-30  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS.

	* fm.h: Set use_dictcommand to 1 by default.

	* scripts/Makefile.in: Add w3mdict.cgi to LIB_TARGETS.

	* config.h.dist: Typo fix for USE_DICT.

2016-10-30  Boruch Baum  <boruch-baum@gmx.com>

	* scripts/w3mdict.cgi: Add w3mdict.cgi to use a dictd dictionary query.
	Bug-Debian: https://github.com/tats/w3m/issues/30

2016-10-09  Tatsuya Kinoshita  <tats@debian.org>

	* form.c:
	Fix incorrect dereference in formUpdateBuffer when MENU_SELECT.
	cf. https://github.com/tats/w3m/commit/ec9eb22e008a69ea9dc21fdca4b9b836679965ee
	    https://github.com/tats/w3m/issues/28

2016-10-08  Tatsuya Kinoshita  <tats@debian.org>

	* table.c, table.h, textlist.h:
	Treat table height as int instead of short.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=838952

	* form.c: Prevent global-buffer-overflow write in formUpdateBuffer.
	Bug-Debian: https://github.com/tats/w3m/issues/29 [CVE-2016-9429] [CVE-2016-9621]

	* form.c: Fix null pointer dereference in formUpdateBuffer.
	Bug-Debian: https://github.com/tats/w3m/issues/28 [CVE-2016-9443]

2016-08-30  Kuang-che Wu  <kcwu@google.com>

	* Str.c: Fix potential heap buffer corruption due to Strgrow.
	Origin: https://github.com/tats/w3m/pull/27 [CVE-2016-9442]

2016-08-29  Tatsuya Kinoshita  <tats@debian.org>

	* anchor.c:
	Prevent segfault due to buffer overflows in addMultirowsForm.
	Bug-Debian: https://github.com/tats/w3m/issues/21 [CVE-2016-9425]
	Bug-Debian: https://github.com/tats/w3m/issues/26 [CVE-2016-9428]

	* form.c: Prevent segfault for formUpdateBuffer.
	Bug-Debian: https://github.com/tats/w3m/issues/13#issuecomment-242981906

2016-08-24  Tatsuya Kinoshita  <tats@debian.org>

	* table.c: Prevent segfault with malformed table_alt.
	Bug-Debian: https://github.com/tats/w3m/issues/24 [CVE-2016-9441]

	* form.c: Prevent segfault for formUpdateBuffer.
	Bug-Debian: https://github.com/tats/w3m/issues/22 [CVE-2016-9440]

	* table.c: Truncate max_width for renderTable.
	Bug-Debian: https://github.com/tats/w3m/issues/25 [CVE-2016-9426]

2016-08-20  Tatsuya Kinoshita  <tats@debian.org>

	* file.c, parsetagx.c: Fix uninitialised values for <i> and <dd>.
	Bug-Debian: https://github.com/tats/w3m/issues/16
	[CVE-2016-9435] [CVE-2016-9436]

	* file.c, parsetagx.c:
	Revert "Fix uninitialised values for <i> and <dd>".
	This reverts commit 0fba2f1a6eb6861206ad120a02af2643938082cd.
	cf. https://github.com/tats/w3m/commit/0fba2f1a6eb6861206ad120a02af2643938082cd#commitcomment-18703355

2016-08-19  Tatsuya Kinoshita  <tats@debian.org>

	* file.c, parsetagx.c: Fix uninitialised values for <i> and <dd>.
	Bug-Debian: https://github.com/tats/w3m/issues/16

2016-08-18  Kuang-che Wu  <kcwu@google.com>

	* table.c: Fix table rowspan and colspan.
	Origin: https://github.com/tats/w3m/pull/19
	Bug-Debian: https://github.com/tats/w3m/issues/8 [CVE-2016-9422]

2016-08-18  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Prevent segfault with malformed input_alt.
	Bug-Debian: https://github.com/tats/w3m/issues/18 [CVE-2016-9438]

	* file.c: Prevent segfault with incorrect button type.
	Bug-Debian: https://github.com/tats/w3m/issues/17 [CVE-2016-9437]

2016-08-17  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Prevent segfault with incorrect form_int fid.
	Bug-Debian: https://github.com/tats/w3m/issues/15 [CVE-2016-9434]

	* libwc/iso2022.c: Prevent segfault when iso2022 parsing.
	Bug-Debian: https://github.com/tats/w3m/issues/14 [CVE-2016-9433]

	* form.c: Prevent segfault for formUpdateBuffer.
	Bug-Debian: https://github.com/tats/w3m/issues/13 [CVE-2016-9432]

	* file.c, form.c:
	Prevent negative array index for selectnumber and textareanumber.
	Bug-Debian: https://github.com/tats/w3m/issues/12 [CVE-2016-9424]

2016-08-16  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Truncate large values of table attributes.
	Bug-Debian: https://github.com/tats/w3m/issues/11

2016-08-15  Tatsuya Kinoshita  <tats@debian.org>

	* form.c: Prevent segfault for formUpdateBuffer.
	Bug-Debian: https://github.com/tats/w3m/issues/9 [CVE-2016-9423]
	Bug-Debian: https://github.com/tats/w3m/issues/10 [CVE-2016-9431]

2016-08-09  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Prevent segfault with malformed input type.
	Bug-Debian: https://github.com/tats/w3m/issues/7 [CVE-2016-9430]

2016-08-08  Tatsuya Kinoshita  <tats@debian.org>

	* Makefile.in, configure, configure.ac, scripts/w3mman/Makefile.in:
	Install German manpages.

2016-08-08  Markus Hiereth  <post@hiereth.de>

	* doc-de/MANUAL.html, doc/MANUAL.html:
	Update MANUAL.html in English and German.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=772341#90

	* doc-de/FAQ.html, doc/FAQ.html: Update FAQ.html in English and German.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=772341#85

	* scripts/w3mman/w3mman.1.in, scripts/w3mman/w3mman.de.1.in:
	Update manpage for w3mman in English and German.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=772341#80

	* doc-de/w3m.1, doc/w3m.1:
	Update manpage for w3m in English and German.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=772341#75

2016-07-18  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS.

	* fm.h: Set default_url to 1 by default.

2016-06-20  Tatsuya Kinoshita  <tats@debian.org>

	* doc-de/README.func, scripts/w3mhelp-funcdesc.de.pl.in:
	Trim trailing spaces.

2016-06-20  Markus Hiereth  <post@hiereth.de>

	* doc-de/README.func, scripts/w3mhelp-funcdesc.de.pl.in:
	Update German help messages.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=765682#47

2016-06-19  Tatsuya Kinoshita  <tats@debian.org>

	* doc-de/README.func, scripts/w3mhelp-funcdesc.de.pl.in:
	Convert German help messages to UTF-8.

	* main.c: Update description of SOURCE and VIEW.

2016-06-19  Markus Hiereth  <post@hiereth.de>

	* doc-de/README.func, doc/README.func:
	Update description of SOURCE and VIEW.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=765682#37

2016-06-19  Tatsuya Kinoshita  <tats@debian.org>

	* NEWS: Update NEWS.

	* doc-de/README.func: Update German messages.

	* doc/README.func, main.c, menu.c: Update English messages.

	* doc-jp/README.func, scripts/w3mhelp-funcdesc.ja.pl.in:
	Update Japanese help messages.

2016-06-19  Markus Hiereth  <post@hiereth.de>

	* doc-de/README.func, scripts/w3mhelp-funcdesc.de.pl.in:
	Update German help messages.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=765682

	* doc/README.func, scripts/w3mhelp-funcdesc.en.pl.in:
	* scripts/w3mhelp.cgi.in: Update English help messages.

2016-05-11  Tatsuya Kinoshita  <tats@debian.org>

	* config.guess, config.sub:
	Update config.* with autotools-dev 20160430.1.

2016-04-14  Tatsuya Kinoshita  <tats@debian.org>

	* doc-de/README.func, doc-jp/README.func, doc/README.func:
	* w3m-doc/sample/keymap.cgi: Cleanup obsolete INIT_MAILCAP.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=820902

	* NEWS: Update NEWS.

2016-04-08  Tatsuya Kinoshita  <tats@debian.org>

	* libwc/johab.c: Fix segfault on bogus text for wc_N_to_johab1.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=820373

2016-04-07  Tatsuya Kinoshita  <tats@debian.org>

	* libwc/map/big5_ucs.map, libwc/map/cns11643_ucs.map:
	* libwc/map/gb12345_ucs.map, libwc/map/gb2312_ucs.map:
	* libwc/map/gbk_ucs.map, libwc/map/hkscs_ucs.map:
	* libwc/map/jisx0208x0212x0213_ucs.map, libwc/map/ksx1001_ucs.map:
	* libwc/map/sjis_ext_ucs.map, libwc/map/uhc_ucs.map, libwc/ucs.c:
	* libwc/ucs.map: Fix segfault on bogus text for wc_any_to_ucs.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=820162

2016-04-03  Tatsuya Kinoshita  <tats@debian.org>

	* doc/FAQ.html, doc/MANUAL.html: Update English documents.

2016-04-03  Markus Hiereth  <markus.hiereth@freenet.de>

	* doc/FAQ.html, doc/MANUAL.html: Update English documents.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=772341#25

2016-04-02  Tatsuya Kinoshita  <tats@debian.org>

	* configure, configure.ac, doc-de/README.func, scripts/Makefile.in:
	* scripts/w3mhelp-funcdesc.de.pl.in, scripts/w3mhelp.cgi.in:
	Support German translated help messages (translation is in progress).
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=765682

	* doc-jp/w3m.1, doc/w3m.1: Update manpage footers to w3m 0.5.3.

	* doc-jp/MANUAL.html, doc-jp/w3m.1, w3m-doc/outline.html.in:
	* w3mhelp-lynx_ja.html.in, w3mhelp-w3m_ja.html.in:
	Update Japanese documents for extbrowser4..9.

2016-04-02  Justin B Rye  <justin.byam.rye@gmail.com>

	* doc/FAQ.html, doc/MANUAL.html, doc/README.func, doc/menu.submenu:
	* main.c, menu.c, scripts/w3mhelp-funcdesc.ja.pl.in:
	* scripts/w3mhelp.cgi.in, w3mhelp-lynx_en.html.in:
	* w3mhelp-w3m_en.html.in: English fixes.
	cf. https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=772341#15
	Origin: https://lists.debian.org/debian-l10n-english/2014/12/msg00002.html
	Origin: https://lists.debian.org/debian-l10n-english/2014/12/msg00030.html
	Origin: https://lists.debian.org/debian-l10n-english/2015/02/msg00011.html

2016-03-30  Leo Famulari  <leo@famulari.name>

	* url.c: Disable RC4.
	Origin: https://git.savannah.gnu.org/cgit/guix.git/commit/?id=62339e2d493bf87a3aabe12e45458581e9705d83

2016-03-29  Tatsuya Kinoshita  <tats@debian.org>

	* url.c: Fix variable is reassigned a value before the old one has
	been used.

	* regex.c: Fix printf format specifier mismatch when REGEX_DEBUG.

	* w3mimg/fb/fb.c: Fix invalid braces when not Linux or FreeBSD.

	* local.c: Fix uninitialized variable when not HAVE_PUTENV.

	* w3mimgdisplay.c: Fix realloc mistake for DrawImage.

	* file.c: Fix mistake of unescape spaces for _doFileCopy.
	cf. [w3m-dev-en 00751], [w3m-dev-en 00752] on 2002-06-09

	* url.c: Fix style of array index is used before limits check.
	Bug: https://sourceforge.net/p/w3m/feature-requests/25/

2016-03-22  Tatsuya Kinoshita  <tats@debian.org>

	* menu.c, proto.h: Fix build failure when not USE_MOUSE for sgrmouse.
	cf. https://twitter.com/naota344/status/711541592167854081

2016-03-20  Tatsuya Kinoshita  <tats@debian.org>

	* rc.c: Fix reverse ordered config parameters.

2016-03-19  Tatsuya Kinoshita  <tats@debian.org>

	* doc/FAQ.html: Update FAQ for extbrowser.

2016-03-14  Tatsuya Kinoshita  <tats@debian.org>

	* po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po, po/zh_TW.po, rc.c:
	Update PO strings for extbrowser2..9.

2016-03-13  Tatsuya Kinoshita  <tats@debian.org>

	* acinclude.m4, configure:
	Set firefox instead of mozilla to default browser.

	* po/Makefile.in.in, po/de.po, po/ja.po, po/w3m.pot, po/zh_CN.po:
	* po/zh_TW.po: Update PO strings for extbrowser4..9.

	* doc-jp/MANUAL.html, doc/MANUAL.html, fm.h, main.c, rc.c:
	Add extbrowser4, extbrowser5, ..., and extbrowser9.
	e.g.
	- extbrowser8 url=%s && printf %s "$url" | xsel && printf %s "$url" | xsel -b &
	- extbrowser9 mpv %s &
	cf. https://github.com/spcmd/w3m

2016-02-28  Tatsuya Kinoshita  <tats@debian.org>

	* menu.c: Fix SIGFPE for ACCESSKEY.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=779092

	* doc/README.func, main.c: Typo fix for ACCESSKEY.
	cf. https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=779092#5

2015-12-20  Franz Thoma  <franz.thoma@tngtech.com>

	* w3mimg/x11/x11_w3mimg.c: Fix semi-transparent artifacts in w3m-img
	when used with 32-bit color (e.g. urxvt).
	imlib_render_image_on_drawable_at_size() tended to leave nasty
	semi-transparent artifacts in 32-bit mode.  Apparently, resizing an
	image in 32-bit mode affects the alpha channel even if there is no
	transparency in the image.  With this patch, resizing is done in
	24-bit mode (or whatever depth the original image has) before
	converting the image to 32-bit and rendering it on the display.
	Origin: https://gist.github.com/fmthoma/f76a1b44e00d5ca972bb
	cf. https://github.com/hut/ranger/issues/86#issuecomment-166027119

2015-12-17  Tatsuya Kinoshita  <tats@debian.org>

	* w3mimg/x11/x11_w3mimg.c:
	Wrap render_pixbuf_to_pixmap_32() in USE_GTK2.

2015-12-17  Araki Ken  <arakiken@users.sf.net>

	* w3mimg/x11/x11_w3mimg.c:
	w3mimgdisplay supports 32 bit depth screen. (e.g. gnome-terminal)
	Origin: https://bitbucket.org/arakiken/w3m/commits/f9c22db8cfd1aaba9bb7301ef9ba51ed88d8bb40

2015-12-17  Tatsuya Kinoshita  <tats@debian.org>

	* w3mimg/x11/x11_w3mimg.c:
	Revert "Fix handling visuals and colormaps incorrectly".
	This reverts commit e24b4064daf3e022e370788a8c7267db40c37dda.

2015-11-19  Tatsuya Kinoshita  <tats@debian.org>

	* fm.h: Accept cookies by default.

	* fm.h: Set argv_is_url to 1 by default.
	Bug-Arch: https://bugs.archlinux.org/task/47102

2015-11-18  Tatsuya Kinoshita  <tats@debian.org>

	* config.guess, config.sub:
	Update config.* with autotools-dev 20150820.1.

2015-11-11  Mingye Wang (Arthur2e5)  <arthur200126@gmail.com>

	* po/LINGUAS, po/zh_CN.po, po/zh_TW.po:
	Add zh_CN and zh_TW translations.
	Please note that the zh_TW translation is machine-converted using
	OpenCC from zh_CN, and needs to be further polished by actual zh_TW
	speakers.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=804732#10

2015-10-24  BwackNinja  <BwackNinja@gmail.com>

	* w3mimg/x11/x11_w3mimg.c:
	Fix handling visuals and colormaps incorrectly.
	cf. https://github.com/hut/ranger/issues/86
	Origin: https://gist.github.com/BwackNinja/60a344730170f9ce2163
	Bug-Arch: https://bugs.archlinux.org/task/46836
	Bug: https://sourceforge.net/p/w3m/patches/72/

2015-10-10  Tatsuya Kinoshita  <tats@debian.org>

	* cookie.c: Remove incomplete special_domain tests.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=385702

2015-10-04  Gaetan Bisson  <bisson@archlinux.org>

	* scripts/w3mhelp.cgi.in: Do not use defined(%hash).
	Origin: https://projects.archlinux.org/svntogit/packages.git/commit/trunk?h=packages/w3m&id=d9e0a4f0b461c9e2177cd9e64a10581386650503
	Bug-Arch: https://bugs.archlinux.org/task/45608

2015-09-06  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Do not use C99-style comments.

2015-09-06  David Crosby  <dave@dafyddcrosby.com>

	* file.c: Mitigate issue #16 found by @kcwu.
	* table.c: Fix stack overflow found by @kcwu.
	Origin: https://github.com/dafyddcrosby/sw3m
	Bug-sw3m: https://github.com/dafyddcrosby/sw3m/issues/16

2015-08-21  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/keymap.lynx, doc/keymap.lynx: Fix unknown key.
	Bug-Ubuntu: https://bugs.launchpad.net/ubuntu/+source/w3m/+bug/265144
	Bug: https://sourceforge.net/p/w3m/bugs/48/

2015-08-11  David Crosby  <dave@dafyddcrosby.com>

	Fix resource leaks, dead assignments, divide-by-zero, and so on.
	Origin: https://github.com/dafyddcrosby/sw3m

	* buffer.c: Check for presence of prevl before using.

	* html.h: Adjust UFclose to remove false positive of CWE-481.

	* ftp.c: Move sockent for splint.

	* cookie.c: Use unsigned int for max_count.

	* libwc/iso2022.c: Add missing comparision that made if always true.

	* Str.c: Use fgetc in while loops, use int instead of char.

	* mailcap.c: Adjust len to size_t.

	* history.c: Check return value of rename.

	* main.c: Adjust while loop.

	* news.c: Check dup call for errors.

	* file.c: Remove unused value.

	* ftp.c: dup can give a negative value.

	* main.c: Use int for c.

	* table.c: Initialize new_tabwidth at declaration.

	* local.c: Remove overflow on readlink.

	* anchor.c, file.c, istream.c, main.c, menu.c, rc.c, table.c, terms.c:
	* url.c: Remove dead assignments flagged by Clang static analysis.

	* w3mbookmark.c:
	Move fclose to fix dereference after null check (Coverity).

	* file.c: Fix resource leak in AuthDigestCred.

	* buffer.c: Fix resource leak in readBufferCache.

	* cookie.c: Fix resource leak in load_cookies.

	* frame.c: Fix resource leak.

	* w3mhelperpanel.c: Fix resource leak.

	* w3mbookmark.c: Fix resource leak and a null return value dereference.

	* linein.c: Fix a divide-by-zero.

	* cookie.c: Change total_dot_number to unsigned int.

	* cookie.c: Free tmp.

	* local.c: Remove unreachable return.

2015-08-10  Alan Grow  <alangrow@gmail.com>

	* url.c (HTTPrequest):
	- Use Content-Type instead of Content-type.
	- Use Content-Length instead of Content-length.
	Origin: https://github.com/acg/w3m/commit/5946c2784d4eae46ec06e52390e43a874b3395fc

2015-08-09  Egmont Koblinger  <egmont@users.sourceforge.net>

	* terms.c: Support sgrmouse for skip_escseq.
	* menu.c: Adjust comments for keymaps.
	Origin: https://sourceforge.net/p/w3m/patches/65/#e2aa

2015-08-09  Tatsuya Kinoshita  <tats@debian.org>

	* keybind_lynx.c: Support sgrmouse for Lynx-like key binding.
	cf. https://sourceforge.net/p/w3m/patches/65/

2015-08-09  IWAMOTO Kouichi  <sue@iwmt.org>

	* menu.c: Support SGR style mouse handler for menu.
	cf. https://github.com/tats/w3m/issues/5
	Origin: https://gist.github.com/ttdoda/83fbcf676a21da28432b
	Bug: https://sourceforge.net/p/w3m/patches/65/

2015-08-06  Richard Quirk  <richard@quirk.es>

	Fix problems reported by cppcheck, clang --analyze and gcc warnings.
	Origin: https://github.com/tats/w3m/pull/6

	* Str.c, Str.h: Strnew_charp and co do not modify the char* input.

	* local.c: Close temp file if pipe open fails.

	* rc.c: Avoid passing null to strlen.

	* file.c: Initialise hidden_input to NULL.
	This prevents a possible use of garbage value on line 3017.

	* file.c: Use pclose for pipe.

2015-08-05  IWAMOTO Kouichi  <sue@iwmt.org>

	* main.c: Fix that SGR style mouse handler has off-by-one problem.
	cf. https://github.com/tats/w3m/issues/5
	Origin: https://gist.github.com/ttdoda/30c189a63d483beeb207
	Bug-Ubuntu: https://bugs.launchpad.net/ubuntu/+source/w3m/+bug/1390768
	Bug: https://sourceforge.net/p/w3m/patches/65/

2015-07-31  yshl  <yshl@takechiyo.net>

	* Bonus/goodict.cgi:
	- Use Encode.pm instead of NKF.
	- Update to the current URL.
	- Enable to select search mode.
	Origin: https://github.com/tats/w3m/pull/4

2015-07-20  Tatsuya Kinoshita  <tats@debian.org>

	* README: Add short description.

	* doc-jp/FAQ.html, doc/FAQ.html: Mention GOPHER_PROXY and FTP_PROXY.

2015-07-05  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/FAQ.html, doc/FAQ.html: Mention HTTPS_PROXY.
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=791425

2015-06-27  yshl  <yshl@takechiyo.net>

	* doc-jp/README.SSL: Modify certdata2pem.rb to assume the encoding
	of the certdata.txt to be UTF-8.
	Origin: https://github.com/tats/w3m/pull/3

2015-06-23  Daniel Schepler  <dschepler@gmail.com>

	* terms.c: Wrap the functions used by image.c in USE_IMAGE.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=789539

2015-05-09  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/README.siteconf, doc/README.siteconf:
	Update examples of siteconf for twitter.com.

2015-05-03  Tatsuya Kinoshita  <tats@debian.org>

	* main.c: Correct GC version confirmation.

2015-05-02  yshl  <yshl@takechiyo.net>

	* main.c: Correct GC version confirmation.
	Origin: https://github.com/tats/w3m/pull/2

2015-04-29  Markus Hiereth  <post@hiereth.de>

	* po/de.po: Update German translation.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=783383

2015-04-29  Tatsuya Kinoshita  <tats@debian.org>

	* po/Makevars, po/de.po, po/ja.po, po/w3m.pot, rc.c:
	Update PO strings for display_borders.

2015-04-26  yshl  <yshl@takechiyo.net>

	* main.c: Use GC_oom_fn instead of GC_set_oom_fn for gc-7.1.
	Origin: https://github.com/tats/w3m/pull/1

2015-02-03  Tatsuya Kinoshita  <tats@debian.org>

	* po/de.po, po/ja.po, po/w3m.pot: Update PO.

2015-02-02  Tatsuya Kinoshita  <tats@debian.org>

	* file.c, fm.h, rc.c:
	New option "display_borders" to display 0 pixel table borders.
	cf. http://d.hatena.ne.jp/rubikitch/20101120

2015-01-24  Tatsuya Kinoshita  <tats@debian.org>

	* acinclude.m4, configure, version.c.in:
	Update to 0.5.3+gitYYYYMMDD (generate from ChangeLog).

2015-01-15  Tatsuya Kinoshita  <tats@debian.org>

	* alloc.h, main.c: Drop C99 features.

2015-01-15  Scarlett  <scarlett@xavin.net>

	Add overflow detection.
	Origin: http://marc.info/?l=openbsd-ports&m=142090828929750&w=2
	* main.c: Call exit(1) when out of memory to avoid dereferencing null
	pointers when gc's malloc fails.
	* alloc.h: Replacements for w3m's allocation macros which add
	overflow detection and concentrate the macros in one file.
	* indep.h, libwc/charset.c, libwc/status.c, matrix.c: Use the
	overflow-detecting allocation macros from alloc.h.

2015-01-15  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c, cookie.c, map.c:
	Do not use C99 printf format specifiers and asprintf.

2015-01-15  Scarlett  <scarlett@xavin.net>

	Correct printf arguments and use asprintf.
	Origin: http://marc.info/?l=openbsd-ports&m=142090828929750&w=2
	* Str.c: Use asprintf() instead of rolling our own printf string
	length detection.
	* cookie.c: Pass the char pointer in the string struct to printf %s
	instead of the string struct itself.
	Print time_t using %lld instead of %ld to allow for 64-bit time_t.
	* main.c: Print a long int using the correct format specifier.
	* map.c: Print size_t using the correct format specifier.

2014-12-06  Araki Ken  <arakiken@users.sf.net>

	Support OSC 5379 remote imaging and sixel graphics.
	Origin: https://bitbucket.org/arakiken/w3m/branch/remoteimg (2014-11-16)

	* doc/README.sixel, terms.c: Add README.sixel.  W3M_IMG2SIXEL
	environmental variable enables to specify options of img2sixel.

	* image.c, terms.c:
	Add n_terminal_image argument to put_image_{sixel|osc5379}().
	Use struct winsize to calculate ppc and ppl.

	* terms.c: If SCREEN_VARIANT=sixel on GNU screen, exec img2sixel
	without -P option.

	* terms.c: ttymode_set() -> ttymode_reset().

	* terms.c: Fix.

	* terms.c: Support GNU screen.

	* terms.c: Show GIF (except animation GIF) correctly.

	* main.c, terms.c: img2sixel exits by Ctrl+C. Enable GIF Animation if
	'I' is pressed to show it.

	* image.c: Add declaration of get_pixel_per_cell().

	* terms.c: Show the first frame of animation gif files.

	* terms.c: system() -> fork()&execvp()

	* display.c: Draw underline on anchor which contains cboth text and
	images.

	* etc.c: Remove close_tty() from setup_child() because close_tty()
	sometimes interrupts loadGeneralFile() in loadImage() and corrupt
	image data can be cached in ~/.w3m.

	* image.c: Minor fix.

	* image.c: Cache image files if at all possible and convert them to
	sixel when -sixel option is specified.

	* image.c: Init pixel_per_{char|line}_i if get_pixel_per_cell() fails.

	* display.c, file.c, fm.h, image.c, main.c, terms.c:
	Add -sixel option which supports image processing by img2sixel.

	* image.c: Don't download image files whose size is specified in
	<img> tag.

	* image.c: Minor fixes of parseImageHeader().

	* image.c: Determine the format of an image file by its header data
	not by its file name suffix.

	* image.c: Read width and height from jpeg, png and gif files directly
	instead of executing w3mimgdisplay -size.

	* display.c: display.c: Draw underline on anchor text which is not
	overlapped with any image.

	* terms.c: Clear fd_set by FD_ZERO() before select().

	* file.c: nw and ni are rounded up instead of rounded off to show
	every corner of images.

	* terms.c: Change time to wait for the response of "\x1b[14t\x1b[18t"
	from 0.1 sec to 0.5 sec.

	* image.c:
	- clearImage() works.
	- Use cached image files created by w3m in getImage().

	* file.c: Hack for alignment.

	* fm.h, image.c, terms.c:
	- Adjust the image size to the terminal cell size.
	- If the image size is specified in html source, skip to load the image.

	* display.c, fm.h, image.c, main.c, terms.c, w3mimg/x11/x11_w3mimg.c:
	Support remote image by OSC 5379 show_picture sequence.

2014-12-06  Olaf Hering  <olh@suse.de>

	* parsetagx.c: Fix crash in parse_tag() during every start.
	Origin: https://build.opensuse.org/package/view_file/openSUSE:Factory/w3m/w3m-parsetagx-crash.patch?expand=1

	* fm.h: Change the default to alt_entity=0.
	Change the default for the option "Use ASCII equivalents to
	display entities" from YES to NO.
	Origin: https://build.opensuse.org/package/view_file/openSUSE:Factory/w3m/w3m-0.5.1-no-ASCII-equivalents-by-default.patch?expand=1
	Bug-Novell: https://bugzilla.novell.com/show_bug.cgi?id=247397

	* anchor.c, libwc/gb18030.c, libwc/ucs.c, regex.c:
	Fix a few harmless uninitialized variables.
	Origin: https://build.opensuse.org/package/view_file/openSUSE:Factory/w3m/w3m-uninitialized.patch?expand=1

2014-12-06  Peter Poeml  <poeml@suse.de>

	* terms.c: Prevent segfault when editing a textarea field with vi.
	Add fix for segfault that can occur when editing a textarea field
	with vi, and returning to w3m (it seems to happen if the terminal
	is not writable, as when using w3m after 'su - some_user')
	Origin: https://build.opensuse.org/package/view_file/openSUSE:Factory/w3m/w3m-0.4.1-textarea-segfault.dif?expand=1

2014-12-04  Tatsuya Kinoshita  <tats@debian.org>

	* acinclude.m4: Follow updated configure.

2014-12-03  Yusuke Baba  <babayaga1@y8.dion.ne.jp>

	* configure, w3mimg/fb/fb.c, w3mimg/fb/fb.h, w3mimg/fb/fb_w3mimg.c:
	Support FreeBSD framebuffer.
	Origin: http://www.ac.auone-net.jp/~baba/w3m-img/index.html
	Bug-FreeBSD: https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=122673

2014-12-02  Naohiro Aota  <naota@gentoo.org>

	* acinclude.m4, configure, w3mimg/fb/fb_gdkpixbuf.c:
	* w3mimg/x11/x11_w3mimg.c:
	Depend on gdk-pixbuf instead of gtk when gtk2.
	Origin: http://sources.gentoo.org/cgi-bin/viewvc.cgi/gentoo-x86/www-client/w3m/files/w3m-0.5.3-gdk-pixbuf.patch?revision=1.1

2014-12-02  Jeroen Roovers  <jer@gentoo.org>

	* acinclude.m4, configure: Add tinfo to with_termlib.
	Fix building against sys-libs/ncurses[tinfo].
	Origin: https://504588.bugs.gentoo.org/attachment.cgi?id=372650
	Bug-Gentoo: https://bugs.gentoo.org/show_bug.cgi?id=504588

2014-12-01  OBATA Akio  <obache@netbsd.org>

	* acinclude.m4, configure:
	Assume defined PKG_CONFIG points right location when gtk2.
	Origin: http://cvsweb.netbsd.org/bsdweb.cgi/pkgsrc/www/w3m/patches/patch-aa?rev=1.13&content-type=text/x-cvsweb-markup
	Origin: http://cvsweb.netbsd.org/bsdweb.cgi/pkgsrc/www/w3m/patches/patch-ak?rev=1.1&content-type=text/x-cvsweb-markup

2014-12-01  Vsevolod Stakhov  <vsevolod@FreeBSD.org>

	* config.h.in: Disable USE_EGD for LibreSSL.
	Disable use of RAND_egd as it is absent in FreeBSD.
	This also fixes build error with LibreSSL.
	Origin: https://bz-attachments.freebsd.org/attachment.cgi?id=144635
	Bug-FreeBSD: https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=191852
	Bug-FreeBSD: https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=191956

2014-12-01  zimous  <zimous@matfyz.cz>

	* po/ja.po: Set Language tag properly for Japanese translation.
	Origin: https://512722.bugs.gentoo.org/attachment.cgi?id=378452
	Bug-Gentoo: https://bugs.gentoo.org/show_bug.cgi?id=512722

2014-11-30  Tatsuya Kinoshita  <tats@debian.org>

	* doc/w3m.1: Typo fix.

2014-11-30  Markus Hiereth  <post@hiereth.de>

	* doc/w3m.1: Miscellaneous changes to improve English manpage.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=766550#30

2014-11-29  Markus Hiereth  <post@hiereth.de>

	* doc/w3m.1: Improve FILES.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=766550#30
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=403634

	* doc/w3m.1: Improve EXAMPLES.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=766550#30
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=380560

	* doc/w3m.1: Improve explanation about option -N.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=766550#30
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=345084
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=530468

	* doc/w3m.1: Note that -cols only affects when HTML is rendered.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=766550#30
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=285251

	* doc/w3m.1: Add more info on configuration.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=766550#30
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=268211

2014-11-29  Justin B Rye  <justin.byam.rye@gmail.com>

	* scripts/w3mman/w3mman.1.in: Tweak for W3MMAN_W3M.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=771003#5

	* scripts/w3mman/w3mman.1.in: English fixes.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=766550#25

2014-11-29  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-19+.

2014-11-29  Justin B Rye  <justin.byam.rye@gmail.com>

	* scripts/w3mman/w3mman2html.cgi.in: Fix Perl warnings.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=771004

2014-10-21  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-19

	* po/LINGUAS: Correct LINGUAS to a whitespace separated list

2014-10-21  Markus Hiereth  <markus.hiereth@freenet.de>

	* po/LINGUAS, po/de.po: Add German translation
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=763964

2014-10-15  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-18

	* doc-jp/README.SSL: Update README.SSL to follow default values

	* config.sub: Update config.sub with autotools-dev 20140911.1

	* fm.h: Disable SSLv3 by default [CVE-2014-3566]
	cf. https://blog.mozilla.org/security/2014/10/14/the-poodle-attack-and-the-end-of-ssl-3-0/

2014-10-15  Ludwig Nussel  <ludwig.nussel@suse.de>

	* fm.h: Force ssl_verify_server on and disable SSLv2 support
	Origin: http://www.openwall.com/lists/oss-security/2010/06/14/4

2014-10-13  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-17+

2014-10-04  Tatsuya Kinoshita  <tats@debian.org>

	* libwc/ambwidth_map.awk, libwc/map/ucs_ambwidth.map:
	Fix incorrect generation of ucs_ambwidth_map

2014-08-22  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-17

	* config.guess:
	Update config.guess to 2014-03-23 with autotools-dev 20140510.1

	* config.sub:
	Update config.sub to 2014-05-01 with autotools-dev 20140510.1

2014-08-22  Micah Cowan  <micah@addictivecode.org>

	* main.c: Support Boehm GC 7.2.
	Replace Gentoo's patch to prevent segfaults due to infinite recursion.
	Origin: https://bugs.debian.org/cgi-bin/bugreport.cgi?msg=5;filename=080_gc72.patch;att=1;bug=758831
	Bug-Debian: https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=758831
	Bug-Fedora: https://bugzilla.redhat.com/show_bug.cgi?id=555467
	Bug: http://sourceforge.net/p/w3m/patches/63/
	Bug: http://sourceforge.net/p/w3m/patches/59/

2014-08-22  Tatsuya Kinoshita  <tats@debian.org>

	* main.c:
	Revert "Support Boehm GC 7.2" (w3m-0.5.2-gc72.patch from Gentoo)
	This reverts commit 4331db3e3e673ac4dbfe8e9f2b42a8e0478dc98a.

2014-06-23  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-16

	* url.c: Disable ciphers that use keys smaller than 128 bits
	Bug-Ubuntu: https://bugs.launchpad.net/ubuntu/+source/w3m/+bug/1325674

2014-01-04  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-15

2014-01-03  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-14

	* acinclude.m4, configure: Use pkg-config to build with imlib2 1.4.6

	* doc/HISTORY, doc/README.cookie, doc/README.m17n:
	Prefer US-ASCII rathar than Japanese encodings in English documents

2013-12-27  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/MANUAL.html, doc/MANUAL.html:
	Cleanup unusable links in MANUAL.html
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=517315

	* version.c.in: Update to 0.5.3+debian-13+

2013-12-17  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-13

2013-12-14  Tatsuya Kinoshita  <tats@debian.org>

	* config.guess:
	Update config.guess to 2013-06-10 with autotools-dev 20130810.1

	* config.sub:
	Update config.sub to 2013-08-10 with autotools-dev 20130810.1

2013-12-07  Reinhard Max  <max@suse.de>

	* local.c: Fix a directory descriptor leak in loadLocalDir.
	Patch from openSUSE on 2009-09-07.
	Origin: https://build.opensuse.org/package/view_file/openSUSE:Factory/w3m/w3m-closedir.patch
	Bug-Novell: https://bugzilla.novell.com/show_bug.cgi?id=531675

2013-12-07  AIDA Shinra  <shinra@j10n.org>

	* main.c: Fix crash after SEARCH_NEXT.
	Patch from <http://www.j10n.org/files/w3m-cvs-1.1055-search-next.patch>,
	[w3m-dev:04473] on 2013-12-07.

2013-11-11  Paul Boekholt  <p.boekholt@gmail.com>

	* file.c: Add support for single quoted meta refresh URL
	Bug: https://sourceforge.net/p/w3m/patches/53/
	Bug-NetBSD: http://gnats.netbsd.org/42400

2013-11-07  Cristian Rodriguez  <crrodriguez@opensuse.org>

	* url.c: Use SSL_OP_NO_COMPRESSION if available.
	Due to the "CRIME attack" (CVE-2012-4929) HTTPS clients that
	negotiate TLS-level compression can be abused for MITM attacks.
	* url.c: Use SSL_MODE_RELEASE_BUFFERS if available.
	Patch from openSUSE on 2012-11-12:
	https://build.opensuse.org/request/show/141054

2013-10-15  Tatsuya Kinoshita  <tats@debian.org>

	* Makefile.in:
	Depend on funcname.tab to fix parallel make issue of scripts
	Bug: https://sourceforge.net/p/w3m/patches/64/
	Bug-Gentoo: https://bugs.gentoo.org/show_bug.cgi?id=362249

	* w3mimg/Makefile.in:
	Avoid prerequisite $(IMGOBJS) to fix parallel make issue of w3mimg
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=726188

	* acinclude.m4, configure:
	Explicitly add -lX11 to IMGX11LDFLAGS only when gtk2
	Bug: https://sourceforge.net/p/w3m/patches/57/

	* w3mimg/Makefile.in: Revert "Fix parallel make issue"
	This reverts commit aa6f871c6dcc108118142bcc786e4a6ac3d46867.

	* Makefile.in:
	Revert "Explicitly link w3mimgdisplay with -lX11 to build with gcc 4.5"
	This reverts commit 7410954066d68ac2ad6aea638801714447321fec.

2013-10-14  AIDA Shinra  <shinra@j10n.org>

	* url.c: Define schemeNumToName() to fix scheme bug.
	Patch from <http://www.j10n.org/files/w3m-cvs-1.1055-schemebug.patch>,
	[w3m-dev:04470] on 2013-10-14.
	Bug: https://sourceforge.net/p/w3m/patches/60/

	* config.h.in, file.c, fm.h, html.h, image.c, indep.c, indep.h:
	* istream.c, istream.h, local.c, main.c, mimehead.c, proto.h:
	Workaround of GC crash on Cygwin64.
	Patch from <http://www.j10n.org/files/w3m-cvs-1.1055-win64gc.patch>,
	[w3m-dev:04469] on 2013-10-14.

2013-10-14  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-12+

2013-10-14  Jarek Czekalski  <jarekczek@poczta.onet.pl>

	* terms.c: Fix paren in check_cygwin_console()
	Bug: https://sourceforge.net/p/w3m/patches/66/

2013-10-13  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-12

	* doc-jp/MANUAL.html, doc-jp/w3m.1, doc/MANUAL.html, doc/w3m.1:
	Update document for the -s option change
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=75527

	* terms.c: Do not fail when LANG is not set.
	Check whether the value of LC_ALL, LC_CTYPE or LANG is not NULL in
	check_cygwin_console().
	Bug: https://sourceforge.net/p/w3m/patches/66/

2013-10-12  Tatsuya Kinoshita  <tats@debian.org>

	* table.h: Bump MAXCOL to 256
	Bug: https://sourceforge.net/p/w3m/feature-requests/24/

2013-10-12  Laurence Richert  <laurencerichert@yahoo.de>

	* main.c, proto.h: vim/-perator like handling
	- half page scrolling
	- jumping to elements numbered by getLinkNumberStr() from Karsten
	  Schoelzel
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=724028

2013-10-12  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/README, doc/README:
	Mention project page rather than unavailable mailing lists

2013-10-09  Rafael Laboissiere  <rafael@laboissiere.net>

	* doc/README.img: Fix typo
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=725892

2013-08-12  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-11+

	* ChangeLog: Update ChangeLog to use contributor's name

2013-08-08  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-11

2013-08-04  Tatsuya Kinoshita  <tats@debian.org>

	* Str.c: Check length for Strchop()

	* main.c: Fix potentially segfault of execdict()

	* version.c.in: Update to 0.5.3+debian-10+

	* file.c: Fix segfault of loadGeneralFile()
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=718612

2013-08-02  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-10

2013-08-02  Piotr P. Karwasz <piotr.p@karwasz.org>

	* scripts/w3mman/w3mman2html.cgi.in:
	Correct underline processing and more UTF-8 support for w3mman2html.cgi.
	Patch from <https://bugs.launchpad.net/ubuntu/+source/w3m/+bug/680202>
	on 2010-11-23.

2013-08-01  Hilko Bengen  <bengen@debian.org>

	* entity.c: Ignore SOFT HYPHEN to prevent drawing hyphens everywhere.
	Patch from <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=441934>
	on 2011-03-01.

2013-08-01  Tatsuya Kinoshita  <tats@debian.org>

	* doc-jp/README, doc/README: Update contact list in README
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=696209

2013-07-30  Tatsuya Kinoshita  <tats@debian.org>

	* config.guess, config.sub:
	Update config.guess and config.sub to supprot aarch64.
	Updated with Debian autotools-dev version 20130515.1.

2013-07-30  Conrad J.C. Hughes  <debbugs@xrad.org>

	* main.c: Sort anchors by sequence number in -dump.
	Patch from <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=657666>
	on 2012-01-27.

2013-07-30  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update to 0.5.3+debian-9+

2013-07-29  Tatsuya Kinoshita  <tats@debian.org>

	* version.c.in: Update version to w3m/0.5.3+debian-9

	* version.c.in: Set CURRENT_VERSION to debian version

2013-07-28  Tatsuya Kinoshita  <tats@debian.org>

	* file.c: Fix segfault of process_button()

2013-04-08  AIDA Shinra  <shinra@j10n.org>

	* file.c: One more patch for siteconf from [w3m-dev 04464]

	* anchor.c, config.h.in, display.c, doc-jp/README.siteconf:
	* doc/README.siteconf, file.c, fm.h, form.c, frame.c, func.c:
	* history.c, indep.c, indep.h, linein.c, main.c, map.c, menu.c:
	* po/ja.po, proto.h, rc.c, url.c: Support the siteconf feature.
	Patch to support the siteconf feature, from [w3m-dev 04463]
	on 2012-06-27.

2013-04-08  Hayaki Saito  <user@zuse.jp>

	* keybind.c, main.c, proto.h, terms.c:
	Support SGR 1006 mouse reporting.
	cf. [w3m-dev 04466] on 2012-07-15
	Origin: https://gist.github.com/3114255
	Bug: https://sourceforge.net/p/w3m/patches/65/

2012-05-19  Hilko Bengen  <bengen@debian.org>

	* form.c: Assume "text" if an input type is unknown.
	Patch from <http://bugs.debian.org/615843> on 2011-03-01.

2012-05-19  Simon Ruderich  <simon@ruderich.org>

	* Makefile.in: Use $(CPPFLAGS) with $(CPP).
	Patch from <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=665491>
	on 2012-03-24.

2012-05-03  Miroslav Šulc  <fordfrog@gentoo.org>

	* w3mimg/Makefile.in: Fix parallel make issue.
	Patch from Gentoo
	<http://sources.gentoo.org/cgi-bin/viewvc.cgi/gentoo-x86/www-client/w3m/files/w3m-0.5.3-parallel-make.patch?revision=1.1&view=markup>
	<https://bugs.gentoo.org/show_bug.cgi?id=353390> on 2011-02-01.

2012-05-03  MATSUU Takuto  <matsuu@gentoo.org>

	* main.c: Support Boehm GC 7.2.
	Patch from Gentoo
	<http://sources.gentoo.org/cgi-bin/viewvc.cgi/gentoo-x86/www-client/w3m/files/w3m-0.5.2-gc72.patch?revision=1.1&view=markup>
	on 2009-12-13.

2012-05-02  Reinhard Tartler  <siretart@tauware.de>

	* istream.c, istream.h:
	Fix that struct file_handle conflicts with glibc 2.14.
	Patch from <https://bugs.launchpad.net/ubuntu/+source/w3m/+bug/935540>
	on 2012-02-19.

2011-10-30  Colin Watson  <cjwatson@ubuntu.com>

	* acinclude.m4, configure, w3mbookmark.c:
	Appease gcc -Werror=format-security.
	Patch from 0.5.3-3ubuntu1 on 2011-10-23.
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=646321

2011-06-19  Martin Pitt  <martin.pitt@ubuntu.com>

	* Makefile.in:
	Explicitly link w3mimgdisplay with -lX11 to build with gcc 4.5.
	Patch from 0.5.2-10ubuntu1 on 2010-12-03.
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=605761

2011-06-19  Fumitoshi UKAI  <ukai@debian.or.jp>

	* main.c: Change the -s option to "squeeze multiple blank lines".
	Change the -s option from "display charset Shift_JIS" to "squeeze
	multiple blank lines" to work as /usr/bin/pager.  In addition, the
	options -j and -e are disabled.  To specify the display charset,
	use -O{s|j|e} instead.
	Patch from [w3m-dev 01275] on 2000-10-26.
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=75527

2011-06-19  Hiroyuki Ito  <ZXB01226@nifty.com>

	* file.c, fm.h, html.c, html.h, proto.h, table.c, tagtable.tab:
	Support the button element as defined in HTML 4.01.
	Patch from upstream, [w3m-dev 04411] on 2010-09-17, to support the
	button element.  It is discussed upstream and incomplete, but enough
	to login Launchpad.
	Bug-Debian: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=136810
	Bug-Ubuntu: https://bugs.launchpad.net/ubuntu/+source/w3m/+bug/628755

See ChangeLog.1 for earlier changes.

;; Local Variables:
;; coding: utf-8
;; End:
