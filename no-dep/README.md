# No Dependencies

During restoration/check, it's possible to use no-dependencies utilities:

if unireedsolomon/colorama is not available, use a Windows no-dependencies python:
  - open a windows terminal: .\backup\no-dep\WPy-3710\WinPython Command Prompt.exe
  - go to root directory: `cd ..\..\..\..`
  - execute `python .\backup\ecc.py create -i .\path\to\ecc`

if duplicity is not available, use a Windows no-dependencies gpg: 
    `.\backup\no-dep\gpg4win-x.x.x\bin\gpg.exe --homedir X:\path\to\gnupg\directory -d .\path\to\file\to\decrypt.tar.gpg > X:\path\to\output\decoded\file.tar`

# Sources

Sources of gpg/colorama/unireedsolomon are also available.
