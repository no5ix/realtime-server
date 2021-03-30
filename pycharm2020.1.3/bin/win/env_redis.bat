@title REDIS
@ECHO off

del dump.rdb
redis\redis-server.exe redis\redis.windows.conf

color c1
pause
