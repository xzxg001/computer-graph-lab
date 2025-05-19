
======================================================================

Date                           : August 1st 2003
Filename                       : jajsnow1_tga.zip
Author                         : Jose "Jaj" Arcediano
Web site                       : www.planetquake.com/jaj 
                                 
Email Address                  : jaj@planetquake.com 
Description                    : A skybox for Quake3 Arena and similar
				 games.
				  			
======================================================================

* Construction *

Base                           : From scratch.
Software used                  : Terragen, Photoshop.
Compile machine                : An arcaic PII 233 (128M) :)
			       
======================================================================

* Instructions *

1. Unzip the files in the Baseq3 folder. All files will be placed in
   their respective directories automatically.	
 
2. Go to the "baseq3\scripts" folder and open a file called 
   "Shaderlist.txt".

3. Add this line at the bottom of the text: "jajsnow1" (without the
   quotes). 
   Save and close it.

4. Once in the Q3radiant editor, go to the texture menu and select 
   "jajsnow1".
   When applying the skybox texture select the one called 
   "jajsnow1_skybox".

5. Note that you will only see one out of the six textures of the
   skybox, the purpose of this texture is to indicate what is the 
   skybox that will be assigned to the map.	 	
   As a guide, the right side of the skybox corresponds with the right
   side of the top view in the editor, and the back side corresponds 
   with the upper side of the top view.

======================================================================

* Comments / Suggestions *

The "jajsnow1.shader" file, provides a default colour, intensity,
etc... for the light from the skybox. 
You can change these values through the key named "q3_sun <r> <g> <b>
<intensity> <degrees> <altitude>" wich is in the shader file.

<r><g><b> are the colour values, red, green and blue respectively.
Their values are normalized to a 0.0 to 1.0 range.

<intensity> is the brightness of the light.

<degrees> is the sun angle relative to the directions of the map.

<elevation> are the degrees formed by the horizon and the sun.

======================================================================

* Copyright / Permissions *

These textures can only be distributed by electronic formats and free
of charge, with this text file included, unless you have my permission
to do otherwise.

If you use this skybox in your work, please don't forget to give me
credits in the readme.txt file.

Thanks and enjoy :) .

Jaj

