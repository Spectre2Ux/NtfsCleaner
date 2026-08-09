@echo off
cl /EHsc /MT /Fe:NtfsCleaner_v0.1.exe NtfsCleaner_v0.1.c user32.lib kernel32.lib advapi32.lib gdi32.lib
