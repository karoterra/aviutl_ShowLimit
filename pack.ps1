$gitTag = (git tag --points-at)
$archiveName = "aviutl_ShowLimit"
if (![string]::IsNullOrEmpty($gitTag))
{
    $archiveName = "${archiveName}_${gitTag}"
}

New-Item publish -ItemType Directory -Force

7z a "publish\${archiveName}.zip" `
    .\README.md `
    .\CHANGELOG.md `
    .\LICENSE `
    .\CREDITS.md `
    ".\build\Release\ShowLimit.auf"
