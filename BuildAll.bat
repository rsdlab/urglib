@echo このスクリプトを実行するには、cmakeおよびmsbuildの双方がPATHに入っている必要があります。cmakeのパスは、C:\Program Files (x86)\CMake 2.8\bin。msbuildのパスは、C:\Windows\Microsoft.NET\Framework\v4.0.30319として、処理をしますが、環境の違いからビルドできない場合は、手動でスクリプトのパスを変更してください

@timeout 5

@call :BUILDW urglib


@timeout 10
@GOTO :EOF

:BUILDW
@if NOT EXIST build-win32 mkdir build-win32
@cd build-win32
@cmake ../
@msbuild %1.sln /p:Configuration=Release /p:Platform=WIN32
@cd ..
@exit /b