# =============================================================================
#  XLEngineEditor 冒烟测试（smoke test）
#  目的：自动化「构建 - 启动 - 稳定观测 - 注入关键输入 - 监测闪退/断言错误」，
#        让每次代码改动在进入人工功能评测前先跑一轮回归检查，避免回归问题带入。
#  配套：
#     .\Scripts\LocalDumps-XLEngine.ps1    先固化崩溃转储（建议管理员执行一次）
#  用法：
#     .\Scripts\smoke_test.ps1               增量构建 + ECS 单测 + 运行 + 输入注入
#     .\Scripts\smoke_test.ps1 -SkipBuild    仅运行 + 输入注入（不重新构建/不重跑单测）
#     .\Scripts\smoke_test.ps1 -Play         以运行时(Play)模式启动并注入输入
#  输出：
#     控制台 绿/黄/红 汇总；详细日志写 build\bin\logs\smoke_test_*.log；
#     崩溃时提示检查 LocalDumps 目录 与 运行日志目录（build\bin\logs）。
# =============================================================================

[CmdletBinding()]
param(
    [string]$BuildDir = '',
    [string]$ExeRelPath = 'bin\XLEngineEditor.exe',
    # 启动后稳定运行的宽限时长（秒）
    [int]$StableSeconds = 3,
    # 持续注入输入的总时长（秒）
    [int]$InputSeconds = 8,
    [switch]$SkipBuild,
    # 运行时(Play)模式：启动即向进程传 --play，首个可渲染帧自动进入 Play，
    # 从而覆盖运行时玩法更新/渲染路径的回归（默认为编辑态启动）。
    [switch]$Play
)

$ErrorActionPreference = 'Stop'
# 用 $PSCommandPath 稳定推导仓库根目录，避免参数默认值里 $PSScriptRoot 为空的问题
$scriptDir = Split-Path -Parent $PSCommandPath
$root      = Split-Path -Parent $scriptDir
Set-Location $root
if (-not $BuildDir) { $BuildDir = Join-Path $root 'build' }

# 让新会话也具备 MSYS2 工具链：g++/cc1plus 及其 DLL 需要 ucrt64\bin。
# 缺 PATH 时 g++ 会静默启动失败（exit=1 且无任何诊断），此处显式补齐。
foreach ($msysBin in @('C:\msys64\ucrt64\bin', 'C:\msys64\usr\bin')) {
    if ((Test-Path $msysBin) -and ($env:PATH -notlike "*$msysBin*")) {
        $env:PATH = $msysBin + ';' + $env:PATH
    }
}

$Exe    = Join-Path $BuildDir $ExeRelPath
$LogDir = Join-Path $BuildDir 'bin\logs'
$LogPath = Join-Path $LogDir ('smoke_test_' + (Get-Date -Format 'yyyyMMdd_HHmmss') + '.log')
$DumpDir = Join-Path $env:LOCALAPPDATA 'CrashDumps\XLEngine'

function Write-Step([string]$m) { Write-Host "==> $m" -ForegroundColor Cyan; Add-Content $LogPath "[STEP] $m" }
function Write-Pass([string]$m) { Write-Host "[PASS] $m" -ForegroundColor Green; Add-Content $LogPath "[PASS] $m" }
function Write-Warn([string]$m) { Write-Host "[WARN] $m" -ForegroundColor Yellow; Add-Content $LogPath "[WARN] $m" }
function Write-Fail([string]$m) { Write-Host "[FAIL] $m" -ForegroundColor Red; Add-Content $LogPath "[FAIL] $m" }

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
Add-Content $LogPath "==== smoke_test 开始 $(Get-Date) ===="

# ----------------------------------------------------------------------------
# P/Invoke 辅助：把引擎窗口置为前台焦点 + 通过 SendInput 注入虚拟键
# （扁平结构体，避免嵌套类型解析问题）
# ----------------------------------------------------------------------------
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;

public static class KeyInj
{
    [DllImport("user32.dll", SetLastError = true)]
    public static extern bool SetForegroundWindow(IntPtr hWnd);

    [DllImport("user32.dll")]
    public static extern bool AttachThreadInput(uint idAttach, uint idAttachTo, bool fAttach);

    [DllImport("user32.dll")]
    public static extern uint GetWindowThreadProcessId(IntPtr h, IntPtr pid);

    [DllImport("user32.dll")]
    public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);

    [DllImport("kernel32.dll")]
    public static extern uint GetCurrentThreadId();

    [DllImport("user32.dll")]
    public static extern uint GetForegroundWindow();

    [StructLayout(LayoutKind.Sequential)]
    public struct INPUT
    {
        public uint type;
        public KEYBDINPUT ki;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct KEYBDINPUT
    {
        public ushort wVk;
        public ushort wScan;
        public uint dwFlags;
        public uint time;
        public IntPtr dwExtraInfo;
    }

    [DllImport("user32.dll", SetLastError = true)]
    static extern uint SendInput(uint nInputs, INPUT[] pInputs, int cbSize);

    // 把进程主窗口强制设为前台
    public static void Focus(IntPtr hwnd)
    {
        SetForegroundWindow(hwnd);
        uint curThread = GetCurrentThreadId();
        uint targetThread = GetWindowThreadProcessId(hwnd, IntPtr.Zero);
        AttachThreadInput(curThread, targetThread, true);
        SetForegroundWindow(hwnd);
        AttachThreadInput(curThread, targetThread, false);
    }

    public delegate bool EnumProc(IntPtr h, IntPtr l);

    [DllImport("user32.dll")]
    public static extern bool EnumWindows(EnumProc cb, IntPtr l);

    [DllImport("user32.dll", CharSet = CharSet.Auto)]
    public static extern int GetClassName(IntPtr h, System.Text.StringBuilder sb, int c);

    [DllImport("user32.dll")]
    public static extern bool IsWindowVisible(IntPtr h);

    // 发送单个虚拟键的一次按下+抬起（可带修饰标志，见 user32 KEYEVENTF_*）
    public static void Press(ushort vk, uint flags)
    {
        INPUT[] seq = new INPUT[2];
        seq[0].type = 1; seq[0].ki.wVk = vk; seq[0].ki.dwFlags = flags;
        seq[1].type = 1; seq[1].ki.wVk = vk; seq[1].ki.dwFlags = flags | 0x0002; // KEYEVENTF_KEYUP
        SendInput(2, seq, Marshal.SizeOf(typeof(INPUT)));
    }

    // 定位进程的实际渲染窗口（GLFW30 类名、可见）。注意：进程的 MainWindowHandle
    // 在 console 子系统下返回的是控制台窗口，键盘事件不会到达 GLFW，必须用 GLFW 窗口。
    public static IntPtr FindGLFW(uint pid)
    {
        IntPtr found = IntPtr.Zero;
        EnumWindows(delegate(IntPtr h, IntPtr l) {
            if (found != IntPtr.Zero) return true;
            uint p; GetWindowThreadProcessId(h, out p);
            if (p != pid) return true;
            if (!IsWindowVisible(h)) return true;
            var sb = new System.Text.StringBuilder(64);
            GetClassName(h, sb, sb.Capacity);
            if (sb.ToString() == "GLFW30") { found = h; return false; }
            return true;
        }, IntPtr.Zero);
        return found;
    }
}
'@ -ReferencedAssemblies 'System.Runtime.InteropServices'

# 虚拟键（对应 GLFW 常用映射；此处多数动作由引擎逻辑动作响应）
$VK_W = 0x57; $VK_A = 0x41; $VK_S = 0x53; $VK_D = 0x44
$VK_E = 0x45
$VK_SHIFT = 0x10
$VK_SPACE = 0x20   # 无 switch case 的按键：压测 EditorLayer::OnKeyPressed 的 default 分支不崩溃

# =============================================================================
# 1. 构建
# =============================================================================
if (-not $SkipBuild) {
    Write-Step "cmake --build $BuildDir"
    $buildOut = & cmake --build $BuildDir 2>&1
    Add-Content $LogPath ($buildOut -join "`n")
    if ($LASTEXITCODE -ne 0) {
        Write-Fail "构建失败（exit=$LASTEXITCODE）"
        exit 1
    }
    Write-Pass '构建成功'
} else {
    Write-Step '跳过构建（-SkipBuild）'
}

# ----------------------------------------------------------------------------
# 1.5 ECS core unit tests (headless: build + run, no window/GL needed)
# ----------------------------------------------------------------------------
Write-Step "run ECS core unit tests via ctest (EcsCoreTests)"
$ctestOut = & ctest --test-dir $BuildDir -R EcsCoreTests --output-on-failure 2>&1
Add-Content $LogPath ($ctestOut -join "`n")
if ($LASTEXITCODE -ne 0) {
    Write-Fail "ECS core unit tests FAILED: exit=$LASTEXITCODE"
    exit 8
}
Write-Pass 'ECS core unit tests PASS'

if (-not (Test-Path $Exe)) {
    Write-Fail "未找到可执行文件：$Exe"
    exit 2
}

# =============================================================================
# 2. 启动 + 稳定观测 + 输入注入 + 监测
# =============================================================================
$engineProc = $null
try {
    $playLabel = ''
    if ($Play) { $playLabel = ' --play(运行时模式)' }
    Write-Step ("启动：" + $Exe + $playLabel)
    $startArgs = @()
    if ($Play) { $startArgs += '--play' }
    $engineProc = Start-Process -FilePath $Exe -ArgumentList $startArgs -PassThru
    Start-Sleep -Seconds $StableSeconds

    if ($engineProc.HasExited) {
        Write-Fail ("引擎启动后立即退出（exit={0}）。崩溃转储：{1}" -f $engineProc.ExitCode, $DumpDir)
        exit 3
    }
    Write-Pass "引擎已稳定运行 {$StableSeconds}s，PID=$($engineProc.Id)"

    # 让引擎窗口获得前台焦点。console 子系统下 MainWindowHandle 是控制台窗口，
    # 键盘事件不会到达 GLFW，必须用 GLFW30 渲染窗口（否则注入永远落空，冒烟成假通过）。
    $engineProc.Refresh()
    $hwnd = [KeyInj]::FindGLFW([uint32]$engineProc.Id)
    if ($hwnd -eq [IntPtr]::Zero) {
        $hwnd = $engineProc.MainWindowHandle   # 兜底：实在找不到 GLFW 窗口再退回主窗口
        Write-Warn '未定位到 GLFW30 渲染窗口，退回 process.MainWindowHandle（注入可能落空）'
    } else {
        Write-Pass "已定位引擎 GLFW 渲染窗口（hwnd=$hwnd）"
    }
    if ($hwnd -ne [IntPtr]::Zero) {
        [KeyInj]::Focus($hwnd)
        Write-Pass "已请求前台焦点（hwnd=$hwnd）"
    } else {
        Write-Warn '未取到引擎窗口句柄，输入注入可能落空'
    }

    Write-Step "注入关键输入 {$InputSeconds}s（WASD 移动 / E 点亮灯台 / Shift 压力）"
    $deadline = (Get-Date).AddSeconds($InputSeconds)
    $failedLog = $null
    while ((Get-Date) -lt $deadline) {
        if ($engineProc.HasExited) {
            Write-Fail ("引擎在输入注入期间退出（exit={0}）。崩溃转储：{1}" -f $engineProc.ExitCode, $DumpDir)
            exit 4
        }

        # ---- 扫描最近一份运行日志中的错误/断言标记 ----
        $logs = Get-ChildItem (Join-Path $LogDir 'XLEngine*.log') -ErrorAction SilentlyContinue |
                Sort-Object LastWriteTime -Descending
        if ($logs) {
            $tail = Get-Content $logs[0].FullName -Tail 80 -ErrorAction SilentlyContinue
            # spdlog 文件日志 pattern 无级别前缀（只有 [时间] name: msg），
            # 这里按断言语义匹配消息正文；真正的崩溃仍由进程存活检测兜底。
            if ($tail -cmatch 'Assertion|assertion failed|terminate called|fatal error|SetTitleIcon: failed|Could not open file') {
                $failedLog = $logs[0].FullName
                break
            }
        }

        # ---- 周期性注入一组按键（按下-抬起各一次）----
        [KeyInj]::Press($VK_W, 0); Start-Sleep -Milliseconds 60
        [KeyInj]::Press($VK_A, 0); Start-Sleep -Milliseconds 60
        [KeyInj]::Press($VK_S, 0); Start-Sleep -Milliseconds 60
        [KeyInj]::Press($VK_D, 0); Start-Sleep -Milliseconds 60
        [KeyInj]::Press($VK_E, 0); Start-Sleep -Milliseconds 60
        [KeyInj]::Press($VK_SHIFT, 0); Start-Sleep -Milliseconds 60
        [KeyInj]::Press($VK_SPACE, 0); Start-Sleep -Milliseconds 60
        Start-Sleep -Milliseconds 200
    }

    if ($failedLog) {
        Write-Fail "检测到运行日志错误/断言标记，最近日志：$failedLog"
        exit 5
    }
    Write-Pass '输入注入完成，引擎仍存活（无闪退、无断言错误）'

    if ($engineProc.HasExited) {
        Write-Fail ("引擎最终退出（exit={0}）。崩溃转储：{1}" -f $engineProc.ExitCode, $DumpDir)
        exit 6
    }
    Write-Pass '引擎持续存活，观测期结束，冒烟通过'
}
finally {
    if ($engineProc -and -not $engineProc.HasExited) {
        Write-Step '观测结束，关闭引擎进程'
        Stop-Process -Id $engineProc.Id -Force -ErrorAction SilentlyContinue
    }
}

Write-Pass "冒烟测试完成，详细日志：$LogPath（运行日志保留最近 5 次启动）"
exit 0