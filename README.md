# Quick Links
This project is one that I'm most proud of. I created this program with c++ to improve efficiency at work centers that I've worked in. I've made 4 different versions of the same program with different graphics for each work center. I don't have the exact initial creation date, but I have an old image on my hard drive with a creation date of 2017-12-15. Although it isn't really necessary to have a program that simply links people to a shared drive or share point location, etc my work center found it useful. I put it on the common shared drive and encouraged people to make a shortcut on their desktop to the shared drive location. The Quick Links program was designed to be light-weight/low-resources. So when it runs from the shared drive it runs fine. I designed it to have a configuration file in the same directory. If the configuration file wasn't there, Quick Links would create it and populate it. The executable, when run from the shared drive location would load the defaults of the config file that was stored on the shared drive. This way whoever was running the shortcut version would all share the same links. This made it easy to teach people how to do the actual job because of the common location of things.  

Over time I improved the Quick Links program. I changed from simple Microsoft Windows API buttons to custom buttons which would change colors when you hover over them and yet another color when you click the button.  

I included a change to the Quick Links that allowed users to change the button caption or link within the program. Previously I or users that I taught how would have to go into the config file and manually change the links. Now you can type a special code (like a videogame cheat) to simply right-click the button and change the caption or the link. The code is "lb4l"  

I added a tetris easter egg to the Quick Links program. While this didn't increase productivity, it was fun for me to do AND it's fun to play tetris. The code to access tetris is "mandatoryfun" then press the escape key to enter tetris. If you need to exit tetris you can simply hit the escape key again. I cleverly coded tetris to hide if the Quick Links program ever lost focus. I programmed high scores which saved locally and could update. There is also a pause functionality by pressing the 'p' key. View the high scores by pressing the "h" key. Place the block instantly by pressing the space key. A block can be saved by pressing left control. I left some debug features that I needed for random tests in the program as well: cycle through debug settings with the 'd' key.  

# Youtube videos and descriptions
# Early Video 1
2018-05-06  
This was actually the first programming related video that I uploaded to YouTube. It was a short 3 second demonstration of the button class that I was working on. Once I finally figured out how to track mouse-over events I wanted to show off to my friends.  
https://www.youtube.com/watch?v=9lFZBBrhHps  

# Early Video 2
2018-05-11  
I completed my button class and incorporated it into the Quick Links program. You can see that the buttons change color when hovered and they turn a bright green when clicked. While a bright green color isn't great UI for normal users, the color green has a specific meaning to the workplace that I made the program for.  
https://www.youtube.com/watch?v=3qc3lBUieCY

# Playlist Video 1
2019-02-06  
The following videos just detail the progress of the easter egg that I programmed into the app. I have assembled all of these videos into the playlist below for ease of watching.  
https://www.youtube.com/watch?v=86e8Lgi8RUU&list=PLTBZ3qil-RVUqxEZOCee0xQGXxt2bSSRp  
I had just recently learned how to use Directx9 from the book "Beginning Game Programming" by Jonathan S Harbour and I wanted to improve an app that I made by including an easter egg for a tetris game.  
Video 1: https://youtu.be/86e8Lgi8RUU  

# Playlist Video 2
2019-02-07  
I finally got a grid system working with a single falling block. The colors changed every time that the block hit the end because it was coded very similarly to what the easter egg looked like in Playlist Video 1. At this point the tetris code was a complete mess, but it looked good. I used a 2x2 array to store block locations.  
Video 2: https://youtu.be/xZeifMKZ4OI  

# Playlist Video 3
2019-02-08  
I finally programmed in the simplest tetris piece. I was moving it 1 or 2 pixels down at a time which I later regretted. I had some simple collision set up, but as I demonstrate in the video there's a lot to be desired. Also the colors are still changing. I don't remember why that is.  
Video 3: https://youtu.be/V5a5CAe5A84  

# Playlist Video 4
2019-02-20  
I included an on-screen indicator of which blocks are which as well as an indicator which shows which key was the last key pressed. I show off a "bump-up" feature that I later removed which allowed you to turn blocks even when there wasn't room. Hit detection was improved massively. You can see that the falling tetronimo is not considered part of the same grid. I believe I massively over-engineered this program. One Lone Coder on youtube has a very elegant method for making a tetris game.  
Video 4: https://youtu.be/FzeTXnnY3u8  

# Playlist Video 5
2019-12-23  
There's a large gap between the last video and this one. I got fed-up with the way that I had designed it and I was struggling to improve it. A few months later I decided to take another stab at tetris, but this time I would re-code the way that it worked to be simpler, but still not as simple as One Lone Coder would have done it! This time I decided to use the same paint method that I used to draw my buttons. I added different colors for the different block types and even a speed-up if you push the 'down' key. At this point I still hadn't added line clearing.  
Video 5: https://youtu.be/qEwehMlH9xc  

# Playlist Video 6
2019-12-24  
I finally added a clear line function. I added a pause function and a game over function. I also added a display to show the player what the next block would be. There was a "Saved Block" area, but I hadn't added that functionality yet.  
Video 6: https://youtu.be/uo7FGTo9F9w  

# Playlist Video 7
2019-12-25  
This was when I actually added the "Change Caption" and "Change Link" features. Up until this point I had been manually changing a config file to update what the button said and where the link would take me. Regarding tetris, I added the "Saved Block" functionality. I improved the UI a little bit as well.  
Video 7: https://youtu.be/p-qe_nTHW04

# Playlist Video 8
2019-12-30  
I added an indicator which would show where the piece would land if the user waited for it to land. I also added the ability to instantly place the piece with the spacebar. I added high scores to the game. I showed off that if there is no high-score that the program will create a default high score file. At this point a user could not enter their initials into the high scores.  
Video 8: https://youtu.be/833WRBzQrb0

# Playlist Video 9
2019-12-31  
I added the ability for players to input their initals on the high scores. The program actually writes/reads the high scores in a pseudocryptographic manner. This way someone couldn't just edit the text of the "hs" (highscores) file in plain text. Someone once downloaded the program and took it home just to use cheat engine to get a good high score. I did the best I could to prevent cheating on the high scores, but I can't think of everything.  
Video 9: https://youtu.be/ISwoztjhy1I
