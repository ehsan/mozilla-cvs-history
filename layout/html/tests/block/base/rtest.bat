@echo off
if "%1"=="baseline" goto baseline

:verify
if not exist verify mkdir verify
s:\mozilla\dist\win32_d.obj\bin\viewer -d 1 -o s:\mozilla\layout\html\tests\block\base\verify\ -rd s:\mozilla\layout\html\tests\block\base -f s:\mozilla\layout\html\tests\block\base\file_list.txt
goto done

:baseline
s:\mozilla\dist\win32_d.obj\bin\viewer -d 1 -o s:\mozilla\layout\html\tests\block\base\ -f s:\mozilla\layout\html\tests\block\base\file_list.txt
goto done

:error
echo syntax: rtest (baseline verify) 

:done
