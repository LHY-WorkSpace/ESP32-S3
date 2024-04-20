@echo off
@chcp 65001
rem  cls:清屏
cls
@title ESP32-S3升级服务.exe
@echo 当前路径:  %cd%


@echo off
for /f "tokens=2 delims=:" %%G in ('ipconfig ^| findstr /I "IPv4"') do (
    set IP=%%G
    goto out
)
:out
echo 本服务器地址: %IP%

if exist .\UpgraFile.bin (
    echo 找到升级文件
    echo 启动升级服务
    @python -m http.server 8070
    @REM del UpgraFile.bin
) else (
    echo 未找到升级文件
)

echo 按任意键关闭............
pause>nul
exit


