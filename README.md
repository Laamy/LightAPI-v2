# Why cant i build?

You need to build roblox luau (https://github.com/Roblox/luau)
then put the built .libs in C:\Luau\x64\libs
you then put the remaining linker headers in C:\Luau\x64\includes
if you dont know how to do this then dont bother me

# How does this work?

Basically we import the same lua version that roblox uses (luau) this allows us to compile load & call luau bytecode with custom functions & others defined
we first setup a console (if your mc install allows it, required soon)
next we check if the tmp file we use as a pipe is setup in the roaming minecraft state
we initialize our game hooks like a normal mcbe client would we then setup the global luau envrionment we use to execute scripts under
we then setup a while true loop to constantly queue the scripts sent

<br\><br\>

Now to where we execute and how we execute. we execute 1 script every game frame (to avoid script spam that can cause major lag) while this doesnt completely stop lag from luau it stops queuing multiple scripts crashing the game next on the game frame we also check the luau scripts that are yielding (wait/pause) if its past the time it was made + the time it needs to wait for it'll resume its execution (meaning the smallest wait is 1 frame but deal with it cuz this is the best method i can come up with)
