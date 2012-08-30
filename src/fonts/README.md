Install a font
==============

- Edit font with gbdfed
- create an font.dir like:

  1  
  yourfont.bdf fontname

- add the dir with the fonts and font.dir to /etc/X11/xorg.conf like:

  Section "Files"  
     FontPath    "/home/user/.fonts"  
  EndSection

- run fc-cache -vf  (to update font cache)
- restart xserver
- to be shure your font is installed run: xlsfonts | grep fontname

-------------
if you just want other symbols insted of an new font:
- edit lokaltog-symbols.bdf with gbdfed
- run makepkg -f
- after that run makepkg -i
