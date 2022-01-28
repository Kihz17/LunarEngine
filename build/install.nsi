# define the name of our installer
OutFile "Installer.exe"

RequestExecutionLevel admin

# define the directory to install to.
InstallDir $DESKTOP\PhysProj1\

# start our default section
Section

# define the output path for this file
SetOutPath $INSTDIR

Call CheckExists

# define what to install and put in the output path
File Project1.exe
File assimp-vc142-mt.dll
File fmod.dll
File /r "assets"

WriteUninstaller "$INSTDIR\Uninstaller.exe"

MessageBox MB_OK "Installed Successfully!"

# end our default section
SectionEnd

Section "Uninstall"

Delete "$INSTDIR\Project1.exe"
Delete "$INSTDIR\assimp-vc142-mt.dll"
Delete "$INSTDIR\fmod.dll"
Delete "$INSTDIR\Uninstaller.exe"
Delete "$INSTDIR\imgui.ini"
RMDir /r "$INSTDIR\assets"

# delete the directory
RMDir $INSTDIR

MessageBox MB_OK "Uninstalled Successfully!"

# end the uninstaller section
SectionEnd

Function CheckExists
IfFileExists "$INSTDIR\Project1.exe" Exists Doesnt
	

Exists:
	MessageBox MB_YESNO "Installation already exists, would you like to overwrite?" IDYES Overwrite IDNO Cancel
	Cancel:
		Quit
	Overwrite:
Doesnt:
FunctionEnd