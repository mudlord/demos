@echo off
Set SourceFile=output.exe
Set Ovl=data.txt
Set Target=output_overlay.exe
copy /B %SourceFile% + %Ovl% %Target% /B