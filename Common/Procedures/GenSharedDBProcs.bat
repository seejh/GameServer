:: Import Path = 프로토 파일 위치
:: --cpp_out / --java_out 해당 언어로 변환한 파일이 생성될 위치
:: 마지막 프로토 파일 명칭

:: %~dp0 = drive path, 배치파일 디렉토리의 전체 경로(정적)

pushd %~dp0

GenProcs.exe --path=SharedDB.xml --output=GenSharedDBProcedures.h

IF ERRORLEVEL 1 PAUSE

XCOPY /Y GenSharedDBProcedures.h "../../ServerPF"

DEL /Q /F *.h

PAUSE