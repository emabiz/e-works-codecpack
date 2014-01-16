set PATH="C:\Program Files (x86)\Inno Setup 5"

rem binary setup generation...
cd open
compil32.exe /cc opencodecpack.template.iss
cd ..

pause