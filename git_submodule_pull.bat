@echo off

set CurrentPath=%~dp0

echo update thirdparty/cjson
cd %CurrentPath%/thirdparty/cjson
git checkout master
git pull

echo update thirdparty/exosip
cd %CurrentPath%/thirdparty/exosip
git checkout master
git pull

echo update thirdparty/hiredis
cd %CurrentPath%/thirdparty/hiredis
git checkout master
git pull

echo update thirdparty/libcstl
cd %CurrentPath%thirdparty/libcstl
git checkout master
git pull

echo update thirdparty/libffmpeg
cd %CurrentPath%/thirdparty/libffmpeg
git checkout master
git pull

echo update thirdparty/libmariadb
cd %CurrentPath%/thirdparty/libmariadb
git checkout master
git pull

echo update thirdparty/librabbitmq
cd %CurrentPath%/thirdparty/librabbitmq
git checkout master
git pull

echo update thirdparty/libuv
cd %CurrentPath%/thirdparty/libuv
git checkout master
git pull

echo update thirdparty/lua
cd %CurrentPath%/thirdparty/lua
git checkout master
git pull

echo update thirdparty/ocilib
cd %CurrentPath%/thirdparty/ocilib
git checkout master
git pull

echo update thirdparty/pugixml
cd %CurrentPath%/thirdparty/pugixml
git checkout master
git pull

echo update thirdparty/zlib
cd %CurrentPath%/thirdparty/zlib
git checkout master
git pull

echo update common/crypto
cd %CurrentPath%/common/crypto
git checkout master
git pull

echo update common/ludb
cd %CurrentPath%/common/ludb
git checkout master
git pull

echo update common/pm
cd %CurrentPath%/common/pm
git checkout master
git pull

echo update common/util
cd %CurrentPath%/common/util
git checkout master
git pull

echo update common/utilc
cd %CurrentPath%/common/utilc
git checkout master
git pull

echo update uvmodules/uvipc
cd %CurrentPath%/uvmodules/uvipc
git checkout master
git pull

echo update uvmodules/uvlogplus
cd %CurrentPath%/uvmodules/uvlogplus
git checkout master
git pull

echo update uvmodules/uvnetplus
cd %CurrentPath%/uvmodules/uvnetplus
git checkout master
git pull

pause
