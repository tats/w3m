Source: http://homepage3.nifty.com/slokar/fb/
original readme.txt

■提供するもの
  ・w3mimgdisplayfb w3mimgdisplay (ほぼ)互換の framebuffer 用画像ビューア
  ・w3mimgsizefb    w3mimgsize 互換の画像サイズ報告プログラム

■必要なもの
  ・GdkPixbuf or Imlib2
  ・TRUE-COLOR の framebuffer を利用できる環境

■コンパイル
  ・Makefile の CFLAGS, LDFLAGS を Imlib2, GdkPixbuf のどちらか利用する
    方を有効にしてから make してださい。

■利用法
  ・w3mimgdisplay, w3mimgsize と同様

■制限等
  ・framebuffer は 15,16,24,32bpp PACKED-PIXELS TRUE-COLOR
    にしか対応していません。
  ・現在 w3mimgdisplayfb は -bg オプションを使用しない場合の背景色は黒
    (#000000)と仮定しています。

■開発環境
  ・ w3m version w3m/0.3+cvs-1.353-m17n-20020316
  ・ linux 2.4.18 (Vine Linux 2.5)
  ・ gcc 2.95.3
  ・ GdkPixbuf 0.16.0
  ・ Imlib2 1.0.6
  ・ $ dmesg |grep vesafb
     vesafb: framebuffer at 0xe2000000, mapped to 0xc880d000, size 8192k
     vesafb: mode is 1024x768x16, linelength=2048, pages=4
     vesafb: protected mode interface info at c000:4785
     vesafb: scrolling: redraw
     vesafb: directcolor: size=0:5:6:5, shift=0:11:5:0
  ・ ビデオカード
    VGA compatible controller: ATI Technologies Inc 3D Rage Pro AGP 1X/2X (rev 92).
      Master Capable.  Latency=64.  Min Gnt=8.
      Non-prefetchable 32 bit memory at 0xe2000000 [0xe2ffffff].
      I/O at 0xd800 [0xd8ff].
      Non-prefetchable 32 bit memory at 0xe1800000 [0xe1800fff].

■その他
  ・w3mimgsizefb, w3mimgdisplayfb は坂本浩則さんの w3mimgsize,
    w3mimgdisplay をもとにしています(というかほとんどそのままです)。
  ・framebuffer 描画関係のコードは、やまさきしんじさんのサンプルプログ
    ラムをもとにしています(というかほとんどそのままです)。
  ・まだ開発途上であり、動作確認も不十分です。使用される際はご自身の責任
    でお願いします。
  ・この配布物に含まれるコードは変更済み BSD ライセンスに従うものとしま
    す。詳細は license.txt を参照してください。

■関連 URI
   ・ W3M Homepage  http://w3m.sourceforge.net/
   ・ w3m-img http://www2u.biglobe.ne.jp/~hsaka/w3m/index-ja.html
   ・ Linux Kernel Hack Japan http://www.sainet.or.jp/~yamasaki/
   ・ Imlib2 http://www.enlightenment.org/pages/main.html
   ・ GdkPixbuf http://developer.gnome.org/arch/imaging/gdkpixbuf.html

■履歴
  ・2002/07/05 開発開始
  ・2002/07/07 ImageMagick 版動作確認
  ・2002/07/10 GdkPixbuf 版動作確認
  ・2002/07/11 Imlib2 版動作確認
  ・2002/07/15 Version 0.1
               公開
  ・2002/07/22 Version 0.2
               描画の高速化

■連絡先
  ZXB01226@nifty.com
  http://homepage3.nifty.com/slokar/
