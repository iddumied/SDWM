Install a font
==============

- Edit font with gbdfed
- run bdftopcf yourfont.bdf > yourfont.pcf (convert bdf font to pcf)
- create an font.dir like:

  1  
  yourfont.pcf fontname

- add the dir with the fonts and font.dir to /etc/X11/xorg.conf like:

  Section "Files"  
     FontPath    "/home/user/.fonts"  
  EndSection

- run fc-cache -vf  (to update font cache)
- to be shure your font is installed run: xlsfonts | grep fontname
