# ASCIIGalaxiesSim
Particle simulation in terminal for win

First:

  setup a desired resolution (height and width) in .cpp file. Remember, that Y is ~doubled in terminal
  
  set ur terminal (font size and window size) to height and width u wrote in .cpp
  
  something too small probably won't work very well because text need some space, 
  
  That is what u get with 250/100 res with 8pt font:
  ![image](https://user-images.githubusercontent.com/88677966/133024951-983d1c22-d93d-45fe-87b3-09b8845a53f9.png)
  
  (u can see that not all the text is placed correctly)

  so I mostly go 
  
    int width = 600;
    int height = 220; 
    
  in .cpp file and same setting in the terminal with 2pt font size (Courier New for me)    
  
  =============== TO MAKE FONT SIZE SMALLER THAN five pt ==============
  
  =============== u can use CTRL + mouse wheel in terminal=============
  
  Minimal font size is 2pt, but remember, that calculations rapidly increased with resolution growth, so maxing it to the top is probably not the bes idea
  
Second:

  if u figured out first step now probably circles might be elliptical on some resolutions/fonts, change xMod in const section
  
  u can use WASD + QE keys to move 
  
  RF, TG, 12, ZX, H and C keys to change coeffs
  
  u can change srand(coeff) coeff in main to change seed and gCount in the top of .cpp to change galaxies amount. I'll ad a big head with settings later and probably a settings menu.
  
  
If u done all the steps correctly u'll have something like this, ENJOY!:

![image](https://user-images.githubusercontent.com/88677966/132976930-9bf8973b-80b2-48c8-a772-55ca0d7fac54.png)


