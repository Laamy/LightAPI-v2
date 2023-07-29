# Lua functions

```lua
readfile(string path) -> 1
```
```lua
-- returns the content of test.txt
print(readfile('test.txt'))
```
Read a files content and returns as result

<br/>

```lua
listfiles(string path) -> 1
```
```lua
-- returns 1: file.txt, ect
for i,v in pairs(listfiles("")) do
  print(i .. ": " .. v)
end
```
Get a list of files as a table

<br/>

```lua
writefile(string path, string content) -> 0
```
```lua
-- write file with the content of "hello world!"
writefile('test.txt', 'hello world!')

-- print hello world!
print(readfile('test.txt'))
```
Write content to a file path

<br/>

```lua
makefolder(string path) -> 0
```
```lua
-- create test folder
makefolder('test')
```
Create a folder

<br/>

```lua
appendfile(string path, string content) -> 0
```
```lua
-- write then append content to end of file
writefile('test.txt', "Hello, ")
appendfile('test.txt', "world!")

-- returns Hello, world!
print(readfile('test.txt'))
```
Append content to a file

<br/>

```lua
isfile(string path) -> 1
```
```lua
-- create test file
writefile('test.txt', "")

-- create test folder
makefolder('test')

-- print false
print(isfile('test'))

-- print true
print(isfile('test.txt'))
```
Check if a path is a file or not

<br/>

```lua
isfolder(string path) -> 1
```
```lua
-- create test file
writefile('test.txt', "")

-- create test folder
makefolder('test')

-- print true
print(isfolder('test'))

-- print false
print(isfolder('test.txt'))
```
Check if a path is a folder or not

<br/>

```lua
delfile(string path) -> 0
```
```lua
-- create test file
writefile('test.txt', "")

-- delete test file
delfile('test.txt')
```
Delete a file from workspace

<br/>

```lua
delfolder(string path) -> 0
```
```lua
-- create test folder
makefolder('test')

-- delete test folder
delfolder('test')
```
Delete a folder from workspace

<br/>

```lua
dofile(string path) -> 0
```
```lua
-- create test script in file
writefile('test.txt', "error('test')")

-- queue file content as new script content & file name as script chunkname
-- print [string "test.txt"):1: test
dofile('test.txt')
```
Queue a file as a script

<br/>
