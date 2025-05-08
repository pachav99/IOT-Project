$envFilePath = "C:\IOT\SAD\sad\.env"

if (Test-Path $envFilePath) {
    Get-Content $envFilePath | ForEach-Object {
        if ($_ -match "^\s*([^#].*?)=(.*)$") {
            $key = $matches[1].Trim()
            $value = $matches[2].Trim()
            [System.Environment]::SetEnvironmentVariable($key, $value, "Process")
        }
    }
    Write-Host "✅ Environment variables loaded from $envFilePath"
} else {
    Write-Host "❌ .env file not found at: $envFilePath"
}
