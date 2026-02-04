# 定义可用的平台选项
$platforms = @(
    @{Name = "Windows x64"; Value = "win-x64"},
    @{Name = "Windows x86"; Value = "win-x86"},
    @{Name = "Windows ARM64"; Value = "win-arm64"},
    @{Name = "Linux x64"; Value = "linux-x64"},
    @{Name = "Linux ARM64"; Value = "linux-arm64"},
    @{Name = "macOS x64"; Value = "osx-x64"},
    @{Name = "macOS ARM64"; Value = "osx-arm64"}
)

# 显示平台选择菜单
Write-Host "`n请选择目标平台：" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
for ($i = 0; $i -lt $platforms.Count; $i++) {
    Write-Host "$($i + 1). $($platforms[$i].Name)" -ForegroundColor Yellow
}
Write-Host "================================" -ForegroundColor Cyan

# 获取用户选择
do {
    $choice = Read-Host "请输入选项编号 (1-$($platforms.Count))"
    $choiceIndex = [int]$choice - 1
} while ($choiceIndex -lt 0 -or $choiceIndex -ge $platforms.Count)

# 获取选中的平台
$selectedPlatform = $platforms[$choiceIndex].Value
Write-Host "`n已选择平台: $($platforms[$choiceIndex].Name) ($selectedPlatform)" -ForegroundColor Green
Write-Host "开始编译..." -ForegroundColor Green

# 执行编译
dotnet publish -c Release -r $selectedPlatform --self-contained true