@title MongoDB
@ECHO off

del mongodb\db.log

mongodb\mongod.exe --config mongodb\mongod.cfg
color c1
pause