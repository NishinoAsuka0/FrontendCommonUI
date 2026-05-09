@echo off
cd /d "%~dp0"
python xlsx_to_json.py
if %errorlevel% neq 0 (
    echo Failed. Make sure Python and openpyxl are installed.
    echo Run: pip install openpyxl
)
pause
