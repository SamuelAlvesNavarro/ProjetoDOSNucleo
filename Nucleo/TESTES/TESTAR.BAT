@echo off
TNUCLEO t
if errorlevel 1 goto falha
TNUCLEO f
if errorlevel 1 goto falha
TNUCLEO m
if errorlevel 1 goto falha
TNUCLEO d
if errorlevel 1 goto falha
TNUCLEO z
if errorlevel 1 goto falha
TNUCLEO r
if errorlevel 1 goto falha
TMENSAG e
if errorlevel 1 goto falha
TMENSAG r
if errorlevel 1 goto falha
TMENSAG f
if errorlevel 1 goto falha
TMENSAG s
if errorlevel 1 goto falha
TMENSAG x
if errorlevel 1 goto falha
TMENSAG d
if errorlevel 1 goto falha
TMENSAG l
if errorlevel 1 goto falha
TMENSAG p
if errorlevel 1 goto falha
..\NUCLEO v
if errorlevel 1 goto falha
echo BATERIA OK > BATERIA.TXT
goto fim
:falha
echo BATERIA FALHOU > BATERIA.TXT
:fim
type TESTES.TXT
type RESULT.TXT
type MENSAG.TXT
