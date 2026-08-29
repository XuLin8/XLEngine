# 固化 XLEngineEditor 的 WER LocalDumps 崩溃转储配置（写 HKLM，需管理员；非管理员会自动弹 UAC 提权）
# 用法：  .\Scripts\LocalDumps-XLEngine.ps1
# 效果：  XLEngineEditor 崩溃时自动把 minidump 写入 "DumpFolder"，保留最近 "DumpCount" 份

[CmdletBinding()]
param(
    [string]$DumpFolder = "$env:LOCALAPPDATA\CrashDumps\XLEngine"
)

$keyPath = 'HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\XLEngineEditor.exe'

# 检查管理员权限
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host '未以管理员运行，请求 UAC 提权…' -ForegroundColor Yellow
    Start-Process powershell -Verb RunAs -ArgumentList "-ExecutionPolicy Bypass -File `"$PSCommandPath`" -DumpFolder `"$DumpFolder`"" -Wait
    exit
}

# 建键（NextLevel：即使已存在也强制创建以便写值）
New-Item -Path $keyPath -Force | Out-Null
New-ItemProperty -Path $keyPath -Name 'DumpFolder' -PropertyType ExpandString -Value $DumpFolder -Force | Out-Null
New-ItemProperty -Path $keyPath -Name 'DumpType'   -PropertyType DWord -Value 2 -Force | Out-Null  # 2 = 完整内存转储（含完整调用栈）
New-ItemProperty -Path $keyPath -Name 'DumpCount'  -PropertyType DWord -Value 5 -Force | Out-Null  # 仅保留最近 5 个转储

Write-Host ("✔ LocalDumps 已固化：崩溃转储写入 {0}（完整转储，保留最近 5 份）" -f $DumpFolder) -ForegroundColor Green