# Lua functions

```lua
print(string arg) -> returns 0
```
```lua
-- print "Hello world!"
print("Hello %s", "World!")

-- print 1
print(1)

-- print nil
print(nil)
```
Allow printing to the UWP console (If your game is installed using a third party launcher only!)

<br/>

```lua
warn(string arg) -> returns 0
```
```lua
-- warn "Hello world!"
warn("Hello %s", "World!")

-- warn 1
warn(1)

-- warn nil
warn(nil)
```
same as print but for warning about legacy features or possible violation

<br/>

```lua
error(string arg) -> returns 0
```
```lua
-- error "[string "chunkname"]:line: Hello world!"
error("Hello %s", "World!")

-- error "[string "chunkname"]:line: 1"
error(1)

-- error "[string "chunkname"]:line: nil"
error(nil)
```
same as print and warn except it pauses the scripts execution and prints in a red font (if console is allocated)

<br/>

```lua
identify() -> returns 1
```
```lua
-- returns "LightAPI" this is so you can tell what executor the script is being run on
print(identify())
```
Returns the current executor name

<br/>

```lua
createscript(string script, string chunkname = "LightScript") -> returns 0
```
```lua
-- creates then queues a new script instance under the chunkname (if defined else "LightScript")
-- queues under same identity as the thread its called by (if exists)
createscript("print('hello world!'", "newChunk")
```
Queue a new script for execution that has its own environmnet different from the current script

<br/>

```lua
time() -> returns 1
```
```lua
-- prints the time in seconds (with the floating point as ms)
print(time())
```
Get the current time useful for calculating how long its been

<br/>

```lua
printidentity(string val = "Current identity is %s") -> returns 0
```
```lua
-- Current identity is 1
printidentity()
```
Used as a debugging tool for the developers of the executor api (cuz i gotta test if identities pass thread to thread properly)

<br/>

```lua
loadstring() -> returns 1
```
```lua
-- load the string as a lua function then store it
local func = loadstring("print('test')")

-- prints test
func()
```
Used to load a string as luau bytecode which you can call

<br/>

```lua
version() -> returns 1
```
```lua
-- prints the api version (x*.*.*)
print(version())
```
used to tell you the api version so scripts can do stuff cuz api changes idk

<br/>

```lua
wait(double seconds = 0.01667) -> returns 0
```
```lua
-- print test
print('test')

-- wait 2 seconds
wait(2)

-- print test 2 two seconds later
print('test 2')
```
Stops executing of lua state and queues it in script context with a new TimeoutInstance (Smallest wait is 1 frame cuz it checks if it should stop yielding per game frame)

<br/>

```lua
spawn(LuaFunc func) -> returns 0
```
```lua
-- print test
-- this script will print test two times then test 2 two times

spawn(function()
  print'test'
  wait()
  print'test 2'
end)

spawn(function()
  print'test'
  wait()
  print'test 2'
end)
```
Spawns a new luau thread using the current thread identity

<br/>
