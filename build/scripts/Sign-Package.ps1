$currentDirectory = split-path $MyInvocation.MyCommand.Definition

# See if we have the ClientSecret available
if([string]::IsNullOrEmpty($Env:SignClientSecret)){
	Write-Host "Client Secret not found, not signing packages"
	return;
}

dotnet tool install signclient --tool-path $currentDirectory

# Setup Variables we need to pass into the sign client tool
$appSettings = "$currentDirectory\..\config\SignClient.json"
$fileList = "$currentDirectory\..\config\filelist.txt"

$appxs = gci $Env:ArtifactDirectory\*.*bundle -recurse | Select -ExpandProperty FullName

foreach ($appx in $appxs){
	Write-Host "Submitting $appx for signing"

	& $currentDirectory\SignClient 'sign' -c $appSettings -i $appx -f $fileList -r $Env:SignClientUser -s $Env:SignClientSecret -n 'Microsoft Terminal' -d 'Microsoft Terminal' -u 'https://github.com/onovotny/Terminal' 

	Write-Host "Finished signing $appx"
}

$insts = gci $Env:ArtifactDirectory\*.appinstaller -recurse | Select -ExpandProperty FullName

foreach ($inst in $insts){
	Write-Host "Submitting $inst for signing"

	& $currentDirectory\SignClient 'sign' -c $appSettings -i $inst -r $Env:SignClientUser -s $Env:SignClientSecret -n 'Microsoft Terminal' -d 'Microsoft Terminal' -u 'https://github.com/onovotny/Terminal' 

	Write-Host "Finished signing $inst"
}

Write-Host "Sign-package complete"
