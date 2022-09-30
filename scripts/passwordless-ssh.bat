@echo off
set IPorFQDN=%1

echo Checking ssh keys
if exist %USERPROFILE%\.ssh\id_rsa.pub (
    echo Public key found
) else (
	echo Can't locate public key. Please run ssh-keygen
    exit /b %errorlevel%
)

echo Connecting %IPorFQDN% ...
echo type %USERPROFILE%\.ssh\id_rsa.pub | ssh %IPorFQDN% "cat >> .ssh/authorized_keys" || exit /b %errorlevel%

echo Done!
