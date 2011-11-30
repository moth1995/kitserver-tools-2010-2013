GDB Manager                                               November 30, 2011
===========================================================================
Version 12.0.0


1. INTRODUCTION
---------------

GDB Manager 12 is a GUI front-end for KitServer 12 GDB (Game DataBase). 
It simplifies the task of defining kit attributes, such as 3D-model of a 
shirt, collar type, name location, name shape, and some others.
GDB Manager will modify (and create, if necessary) the corresponding 
"config.txt" files.


2. USAGE
--------

Run GDBManager.exe. When run first time, it will ask you to select the
location of your "GDB" folder. After that it should show a directory-like
tree on the left side which enumerates all the kits in the GDB. 
(IMPORTANT: Only kits that are mapped in your map.txt file will be shown)
You can select a kit there and set/modify attributes in the right panel.

2.1. Picking colors of the kit image: color picker
--------------------------------------------------
Right click on the kit image and the color will be picked from the
shirt and set as the kit main (radar) color. If you do a SHIFT-RightClick,
then the picked color will be set as shorts color.



3. CREDITS
----------

Programming: juce
Third party tools and libraries: wxPython by Robin Dunn and Co.

===========================================================================


