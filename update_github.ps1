$body = Get-Content github_update.json -Raw
$token = Read-Host "Enter GitHub personal access token"
$headers = @{
    Authorization = "token $token"
    Accept = 'application/vnd.github.v3+json'
    'Content-Type' = 'application/json'
}
Invoke-RestMethod -Uri 'https://api.github.com/repos/revitalyr/Secure_Remote_Agent' -Method Patch -Headers $headers -Body $body
