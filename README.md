# AnimatedDex
For Animated Pokemon Face

Steps to change this watch face to your liking:

1. Fork this repository and add to CloudPebble or Pebble SDK

2. Change the data resources (azumarill.png, converted.png, lanturn.png, and luxray.png) to your APNGs

3. Fix naming conventions as you see fit

4. In main.c change lines 22 and 24 to your preferences.

5. Recompile and build PBW

6. Enjoy- get creative, maybe have one pokemon with all of its different animations

7. Note: Be sure not to use more than 4 APNG's due to space requirements



Steps to create an Pebble Ready Gen 6 Sprite:

1. Go to http://www.pkparaiso.com/xy/sprites_pokemon.php and save a gif (choose 120x120 or less)

2. Optional: Edit gif to fit pebble's color palette

    • Use GIMP to load gif. Change color mode to index. Rename layers to (replace) instead of (combined)(!). Export as gif with animate option checked. Loop infinitely
    
3. Use gif2apng (http://gif2apng.sourceforge.net/) to convert the gif to apng

4. Add as a data/raw binary resource
