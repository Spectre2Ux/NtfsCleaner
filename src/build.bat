@echo off
cl /EHsc /MT /Fe:NtfsCleaner_v0.6.exe NtfsCleaner_v0.6.c user32.lib kernel32.lib advapi32.lib gdi32.lib
cl /EHsc /LD /MT /Fe:SelfDestructDll.dll SelfDestructDll.cpp user32.lib kernel32.lib
cl /EHsc /Fe:TestInjectTarget.exe test.cpp user32.lib kernel32.lib
